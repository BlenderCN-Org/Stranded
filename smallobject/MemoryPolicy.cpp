#pragma once

#include <memory>
#include <new>
#include <assert.h>
#include <vector>

struct Chunk
{
public:
	void Init(std::size_t blockSize, unsigned char blocks)
	{
		assert(blockSize > 0 && blocks > 0);
		// Overflow check
		assert((blockSize * blocks) / blockSize == (unsigned)blocks);

		pData_ = new unsigned char[blockSize * blocks];
		Reset(blockSize, blocks);
	}
	void* Allocate(std::size_t blockSize)
	{
		if (!blocksAvailable_) return 0;
		assert((firstAvailableBlock_ * blockSize) / blockSize == 
			(unsigned)firstAvailableBlock_);

		unsigned char* pResult = pData_ + (firstAvailableBlock_ * blockSize);
		firstAvailableBlock_ = *pResult;
		--blocksAvailable_;
		memset(pResult, 0, blockSize);
		return pResult;
	}
	void Deallocate(void* p, std::size_t blockSize)
	{
		assert(p >= pData_);

		unsigned char* toRelease = static_cast<unsigned char*>(p);
		assert((toRelease - pData_) % blockSize == 0);

		*toRelease = firstAvailableBlock_;
		firstAvailableBlock_ = static_cast<unsigned char>(
			(toRelease - pData_) / blockSize);
		assert((unsigned)firstAvailableBlock_ == (toRelease - pData_) / blockSize);

		++blocksAvailable_;
	}
	void Reset(std::size_t blockSize, unsigned char blocks)
	{
		firstAvailableBlock_ = 0;
		blocksAvailable_ = blocks;
		unsigned char* p = pData_;
		for (unsigned char i = 0; i != blocks; p += blockSize)
		{
			*p = ++i;
		}
	}
	void Release()
	{
		delete[] pData_;
	}
	unsigned char* pData_;
	unsigned char firstAvailableBlock_, blocksAvailable_;
};

template <typename T>
class MemPolicyCached
{
public:
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T*			stored_type;
	typedef T			value_type;
public:
	MemPolicyCached() : m_pT(0) {}
	MemPolicyCached(const MemPolicyCached&) {}
	template <class U>
	MemPolicyCached(const MemPolicyCached<U>&) {}
	MemPolicyCached(const stored_type& p) : m_pT(p) {}
	~MemPolicyCached() {}

	pointer operator-> () const {return m_pT;}
	reference operator* () const {return *m_pT;}
	void Swap(MemPolicyCached& rhs)
	{std::swap(m_pT, rhs.m_pT);}

	friend inline pointer GetImpl(const MemPolicyCached& sp)
	{return sp.m_pT;}

	friend inline const stored_type& GetImplRef(const MemPolicyCached& sp)
	{return sp.m_pT;}

	friend inline stored_type& GetImplRef(MemPolicyCached& sp)
	{return sp.m_pT;}

	//void Copy(const stored_type& p){*m_pT = *p;}
	//ÄÚ´æÐÂ½¨
	void Create()
	{
		if (0 == m_pAllocChunk || 0 == m_pAllocChunk->blocksAvailable_)
		{
			for (std::vector<chunk_type >::iterator iter = m_vecChunks.begin(); ; ++iter)
			{
				if (iter == m_vecChunks.end())
				{
					chunk_type newChunk;
					newChunk.Init(sizeof(T), m_nCount);
					m_vecChunks.push_back(newChunk);
					m_pAllocChunk = &m_vecChunks.back();
					m_pDeallocChunk = &m_vecChunks.front();
					break;
				}
				if (iter->blocksAvailable_ > 0)
				{
					m_pAllocChunk = &*iter;
					break;
				}
			}
		}
		assert(m_pAllocChunk != 0 && m_pAllocChunk->blocksAvailable_ > 0);
		void* p = m_pAllocChunk->Allocate(sizeof(T));
		new (p)T();
		m_pT = static_cast<stored_type>(p);
	}

	void  Destroy()
	{
		assert(!m_vecChunks.empty());
		assert(&m_vecChunks.front() <= m_pDeallocChunk);
		assert(&m_vecChunks.back() >= m_pDeallocChunk);

		const std::size_t nChunkLen = sizeof(T) * m_nCount;
		chunk_type* lo = m_pDeallocChunk;
		chunk_type* hi = m_pDeallocChunk + 1;
		chunk_type* loBount = &m_vecChunks.front();
		chunk_type* hiBount = &m_vecChunks.back() + 1;
		if (hi == hiBount) hi = 0;
		void* pTemp = m_pT;

		for (;;)
		{
			if (lo)
			{
				if (pTemp >= lo->pData_ && pTemp < lo->pData_ + nChunkLen)
				{
					m_pDeallocChunk = lo;
					break;
				}

				if (lo == loBount)
					lo = 0;
				else
					--lo;
			}

			if (hi)
			{
				if (pTemp >= hi->pData_ && pTemp < hi->pData_ + nChunkLen)
				{
					m_pDeallocChunk = hi;
					break;
				}

				if (++hi == hiBount) hi = 0;
			}

			if (0 == lo && 0 == hi)
			{
				assert(false);
				break;
			}
		}

		m_pT->~T();
		assert(m_pDeallocChunk);
		m_pDeallocChunk->Deallocate(pTemp, sizeof(T));

		if (m_pDeallocChunk->blocksAvailable_ == m_nCount)
		{
			chunk_type& lastChunk = m_vecChunks.back();
			if (&lastChunk == m_pDeallocChunk)
			{
				if (m_vecChunks.size() > 1 && m_pDeallocChunk[-1].blocksAvailable_ == m_nCount)
				{
					lastChunk.Release();
					m_vecChunks.pop_back();
					m_pAllocChunk = m_pDeallocChunk = &m_vecChunks.front();
				}
			}
			else if (lastChunk.blocksAvailable_ == m_nCount)
			{
				lastChunk.Release();
				m_vecChunks.pop_back();
				m_pAllocChunk = m_pDeallocChunk;
			}
			else
			{
				std::swap(*m_pDeallocChunk, lastChunk);
				m_pAllocChunk = &m_vecChunks.back();
			}
		}
	}

public:
	typedef typename Chunk		chunk_type;
private:
	static std::vector<chunk_type> m_vecChunks;
	static chunk_type*		m_pAllocChunk;
	static chunk_type*		m_pDeallocChunk;
	static std::size_t		m_nCount;
	stored_type				m_pT;
};

template <class T>	std::vector<typename MemPolicyCached<T>::chunk_type> MemPolicyCached<T>::m_vecChunks;
template <class T>	typename MemPolicyCached<T>::chunk_type*	MemPolicyCached<T>::m_pAllocChunk = 0;
template <class T>	typename MemPolicyCached<T>::chunk_type*	MemPolicyCached<T>::m_pDeallocChunk = 0;
template <class T>	std::size_t	MemPolicyCached<T>::m_nCount = 255;


template <typename T>
class MemPolicyNew
{
public:
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T*			stored_type;
	typedef T			value_type;
public:
	MemPolicyNew() {}
	~MemPolicyNew() {}

	void Create()
	{
		m_pT = new T();
	}

	void Destroy()
	{
		if (m_pT) delete m_pT;
		m_pT = 0;
	}

private:
	stored_type	m_pT;
};

//#include <limits>
//
//template <class T>
//class MyAllocator
//{
//public:
//	typedef size_t		size_type;
//	typedef ptrdiff_t	difference_type;
//	typedef T*			pointer;
//	typedef const T*	const_pointer;
//	typedef T&			reference;
//	typedef const T&	const_reference;
//	typedef T			value_type;
//
//	template <class U>
//	struct rebind
//	{
//		typedef MyAllocator<U> other;
//	};
//
//	pointer address(reference value) const {return &value;}
//	const_pointer address(const_reference value) const {return &value;}
//
//	MyAllocator() throw() {}
//	MyAllocator(const MyAllocator&) throw() {}
//	template <class U> MyAllocator(const MyAllocator<U>&) throw() {}
//	~MyAllocator() throw() {}
//
//	size_type max_size() const throw() {return std::numeric_limits<size_t>::max() / sizeof(T);}
//
//	pointer allocate(size_type num, MyAllocator<void>::const_pointer hint = 0)
//	{
//		return (pointer)(::operator new(num*sizeof(T)));
//	}
//	void construct(pointer p, const T& value) 
//	{
//		new((void*)p)T(value);
//	}
//	void destroy(pointer p) 
//	{
//		p->~T();
//	}
//	void deallocate(pointer p, size_type num)
//	{
//		::operator delete((void*)p);
//	}
//
//	template <class T1>
//	bool operator== (const MyAllocator<T1>&) throw() {return true;}
//	template <class T1>
//	bool operator!= (const MyAllocator<T1>&) throw() {return false;}
//};
//
//template <>
//class MyAllocator<void> 
//{
//public:
//	typedef void*		pointer;
//	typedef const void* const_pointer;
//	typedef void		value_type;
//	template <class U>
//	struct rebind
//	{
//		typedef MyAllocator<U> other;
//	};
//};