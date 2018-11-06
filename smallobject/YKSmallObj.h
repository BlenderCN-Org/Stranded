#pragma once

#include "YKThreadPolicy.h"
#include <vector>

#ifndef DEFAULT_CHUNK_SIZE
#define DEFAULT_CHUNK_SIZE 4096
#endif

#ifndef MAX_SMALL_OBJECT_SIZE
#define MAX_SMALL_OBJECT_SIZE 64
#endif

class FixedAllocator
{
private:
	struct Chunk
	{
		void Init(std::size_t blockSize, unsigned char blocks);
		void* Allocate(std::size_t blockSize);
		void Deallocate(void* p, std::size_t blockSize);
		void Reset(std::size_t blockSize, unsigned char blocks);
		void Release();
		unsigned char* pData_;
		unsigned char
			firstAvailableBlock_,
			blocksAvailable_;
	};

	// Internal functions        
	void DoDeallocate(void* p);
	Chunk* VicinityFind(void* p);

	// Data 
	std::size_t blockSize_;
	unsigned char numBlocks_;
	typedef std::vector<Chunk> Chunks;
	Chunks chunks_;
	Chunk* allocChunk_;
	Chunk* deallocChunk_;
	// For ensuring proper copy semantics
	mutable const FixedAllocator* prev_;
	mutable const FixedAllocator* next_;

public:
	// Create a FixedAllocator able to manage blocks of 'blockSize' size
	explicit FixedAllocator(std::size_t blockSize = 0);
	FixedAllocator(const FixedAllocator&);
	FixedAllocator& operator=(const FixedAllocator&);
	~FixedAllocator();

	void Swap(FixedAllocator& rhs);

	// Allocate a memory block
	void* Allocate();
	// Deallocate a memory block previously allocated with Allocate()
	// (if that's not the case, the behavior is undefined)
	void Deallocate(void* p);
	// Returns the block size with which the FixedAllocator was initialized
	std::size_t BlockSize() const
	{ return blockSize_; }
	bool operator<(std::size_t rhs) const
	{ return BlockSize() < rhs; }
};

class SmallObjAllocator
{
public:
	SmallObjAllocator(
		std::size_t chunkSize, 
		std::size_t maxObjectSize);

	void* Allocate(std::size_t numBytes);
	void Deallocate(void* p, std::size_t size);

private:
	SmallObjAllocator(const SmallObjAllocator&);
	SmallObjAllocator& operator=(const SmallObjAllocator&);

	typedef std::vector<FixedAllocator> Pool;
	Pool pool_;
	FixedAllocator* pLastAlloc_;
	FixedAllocator* pLastDealloc_;
	std::size_t chunkSize_;
	std::size_t maxObjectSize_;
};

template <template <class> class ThreadPolicy = POLICY_THREAD::SingleThread,
	std::size_t chunkSize = DEFAULT_CHUNK_SIZE, 
	std::size_t maxSmallObjectSize = MAX_SMALL_OBJECT_SIZE>
class YKSmallObject : public ThreadPolicy<YKSmallObject<ThreadPolicy, chunkSize, maxSmallObjectSize> >
{
	typedef ThreadPolicy< YKSmallObject<ThreadPolicy, chunkSize, maxSmallObjectSize> > YKThreadPolicy;

	struct YKSmallObjAllocator : public SmallObjAllocator
	{
		YKSmallObjAllocator() 
			: SmallObjAllocator(chunkSize, maxSmallObjectSize)
		{}

		static YKSmallObjAllocator& Instance()
		{
			return m_Obj;
		}

		static YKSmallObjAllocator m_Obj;
	};
public:
	static void* operator new(std::size_t size)
	{
#if (MAX_SMALL_OBJECT_SIZE != 0) && (DEFAULT_CHUNK_SIZE != 0)
		typename YKThreadPolicy::Lock lock;
		(void)lock;

		return YKSmallObjAllocator::Instance().Allocate(size);
#else
		return ::operator new(size);
#endif
	}

	static void operator delete(void* p, std::size_t size)
	{
#if (MAX_SMALL_OBJECT_SIZE != 0) && (DEFAULT_CHUNK_SIZE != 0)
		typename YKThreadPolicy::Lock lock;
		(void)lock;

		YKSmallObjAllocator::Instance().Deallocate(p, size);
#else
		::operator delete(p);
#endif
	}
	virtual ~YKSmallObject(void);
};

template <template <class> class ThreadPolicy,
	std::size_t chunkSize, 
	std::size_t maxSmallObjectSize>
	typename YKSmallObject<ThreadPolicy, chunkSize, maxSmallObjectSize>::YKSmallObjAllocator
	YKSmallObject<ThreadPolicy, chunkSize, maxSmallObjectSize>::YKSmallObjAllocator::m_Obj;