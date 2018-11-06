#include "StdAfx.h"
#include "YKSmallObj.h"

#include <assert.h>
#include <algorithm>
#include <functional>
void FixedAllocator::Chunk::Init(std::size_t blockSize, unsigned char blocks)
{
	assert(blockSize > 0);
	assert(blocks > 0);
	// Overflow check
	assert((blockSize * blocks) / blockSize == blocks);

	pData_ = new unsigned char[blockSize * blocks];
	Reset(blockSize, blocks);
}

void FixedAllocator::Chunk::Reset(std::size_t blockSize, unsigned char blocks)
{
	assert(blockSize > 0);
	assert(blocks > 0);
	// Overflow check
	assert((blockSize * blocks) / blockSize == blocks);

	firstAvailableBlock_ = 0;
	blocksAvailable_ = blocks;

	unsigned char i = 0;
	unsigned char* p = pData_;
	for (; i != blocks; p += blockSize)
	{
		*p = ++i;
	}
}

void FixedAllocator::Chunk::Release()
{
	delete[] pData_;
}

void* FixedAllocator::Chunk::Allocate(std::size_t blockSize)
{
	if (!blocksAvailable_) return 0;

	assert((firstAvailableBlock_ * blockSize) / blockSize == 
		firstAvailableBlock_);

	unsigned char* pResult =
		pData_ + (firstAvailableBlock_ * blockSize);
	firstAvailableBlock_ = *pResult;
	--blocksAvailable_;

	return pResult;
}

void FixedAllocator::Chunk::Deallocate(void* p, std::size_t blockSize)
{
	assert(p >= pData_);

	unsigned char* toRelease = static_cast<unsigned char*>(p);
	assert((toRelease - pData_) % blockSize == 0);

	*toRelease = firstAvailableBlock_;
	firstAvailableBlock_ = static_cast<unsigned char>(
		(toRelease - pData_) / blockSize);
	assert(firstAvailableBlock_ == (toRelease - pData_) / blockSize);

	++blocksAvailable_;
}

FixedAllocator::FixedAllocator(std::size_t blockSize)
: blockSize_(blockSize)
, allocChunk_(0)
, deallocChunk_(0)
{
	assert(blockSize_ > 0);

	prev_ = next_ = this;

	std::size_t numBlocks = DEFAULT_CHUNK_SIZE / blockSize;
	if (numBlocks > UCHAR_MAX) numBlocks = UCHAR_MAX;
	else if (numBlocks == 0) numBlocks = 8 * blockSize;

	numBlocks_ = static_cast<unsigned char>(numBlocks);
	assert(numBlocks_ == numBlocks);
}

FixedAllocator::FixedAllocator(const FixedAllocator& rhs)
: blockSize_(rhs.blockSize_)
, numBlocks_(rhs.numBlocks_)
, chunks_(rhs.chunks_)
{
	prev_ = &rhs;
	next_ = rhs.next_;
	rhs.next_->prev_ = this;
	rhs.next_ = this;

	allocChunk_ = rhs.allocChunk_
		? &chunks_.front() + (rhs.allocChunk_ - &rhs.chunks_.front())
		: 0;

	deallocChunk_ = rhs.deallocChunk_
		? &chunks_.front() + (rhs.deallocChunk_ - &rhs.chunks_.front())
		: 0;
}

FixedAllocator& FixedAllocator::operator=(const FixedAllocator& rhs)
{
	FixedAllocator copy(rhs);
	copy.Swap(*this);
	return *this;
}

FixedAllocator::~FixedAllocator()
{
	if (prev_ != this)
	{
		prev_->next_ = next_;
		next_->prev_ = prev_;
		return;
	}

	assert(prev_ == next_);
	Chunks::iterator i = chunks_.begin();
	for (; i != chunks_.end(); ++i)
	{
		assert(i->blocksAvailable_ == numBlocks_);
		i->Release();
	}
}

void FixedAllocator::Swap(FixedAllocator& rhs)
{
	using namespace std;

	swap(blockSize_, rhs.blockSize_);
	swap(numBlocks_, rhs.numBlocks_);
	chunks_.swap(rhs.chunks_);
	swap(allocChunk_, rhs.allocChunk_);
	swap(deallocChunk_, rhs.deallocChunk_);
}

void* FixedAllocator::Allocate()
{
	if (allocChunk_ == 0 || allocChunk_->blocksAvailable_ == 0)
	{
		Chunks::iterator i = chunks_.begin();
		for (;; ++i)
		{
			if (i == chunks_.end())
			{
				// Initialize
				chunks_.reserve(chunks_.size() + 1);
				Chunk newChunk;
				newChunk.Init(blockSize_, numBlocks_);
				chunks_.push_back(newChunk);
				allocChunk_ = &chunks_.back();
				deallocChunk_ = &chunks_.front();
				break;
			}
			if (i->blocksAvailable_ > 0)
			{
				allocChunk_ = &*i;
				break;
			}
		}
	}
	assert(allocChunk_ != 0);
	assert(allocChunk_->blocksAvailable_ > 0);

	return allocChunk_->Allocate(blockSize_);
}

void FixedAllocator::Deallocate(void* p)
{
	assert(!chunks_.empty());
	assert(&chunks_.front() <= deallocChunk_);
	assert(&chunks_.back() >= deallocChunk_);

	deallocChunk_  = VicinityFind(p);
	assert(deallocChunk_);

	DoDeallocate(p);
}

FixedAllocator::Chunk* FixedAllocator::VicinityFind(void* p)
{
	assert(!chunks_.empty());
	assert(deallocChunk_);

	const std::size_t chunkLength = numBlocks_ * blockSize_;

	Chunk* lo = deallocChunk_;
	Chunk* hi = deallocChunk_ + 1;
	Chunk* loBound = &chunks_.front();
	Chunk* hiBound = &chunks_.back() + 1;

	// Special case: deallocChunk_ is the last in the array
	if (hi == hiBound) hi = 0;

	for (;;)
	{
		if (lo)
		{
			if (p >= lo->pData_ && p < lo->pData_ + chunkLength)
			{
				return lo;
			}
			if (lo == loBound) lo = 0;
			else --lo;
		}

		if (hi)
		{
			if (p >= hi->pData_ && p < hi->pData_ + chunkLength)
			{
				return hi;
			}
			if (++hi == hiBound) hi = 0;
		}
	}
	assert(false);
	return 0;
}

void FixedAllocator::DoDeallocate(void* p)
{
	assert(deallocChunk_->pData_ <= p);
	assert(deallocChunk_->pData_ + numBlocks_ * blockSize_ > p);

	// call into the chunk, will adjust the inner list but won't release memory
	deallocChunk_->Deallocate(p, blockSize_);

	if (deallocChunk_->blocksAvailable_ == numBlocks_)
	{
		// deallocChunk_ is completely free, should we release it? 

		Chunk& lastChunk = chunks_.back();

		if (&lastChunk == deallocChunk_)
		{
			// check if we have two last chunks empty

			if (chunks_.size() > 1 && 
				deallocChunk_[-1].blocksAvailable_ == numBlocks_)
			{
				// Two free chunks, discard the last one
				lastChunk.Release();
				chunks_.pop_back();
				allocChunk_ = deallocChunk_ = &chunks_.front();
			}
			return;
		}

		if (lastChunk.blocksAvailable_ == numBlocks_)
		{
			// Two free blocks, discard one
			lastChunk.Release();
			chunks_.pop_back();
			allocChunk_ = deallocChunk_;
		}
		else
		{
			// move the empty chunk to the end
			std::swap(*deallocChunk_, lastChunk);
			allocChunk_ = &chunks_.back();
		}
	}
}

SmallObjAllocator::SmallObjAllocator(
									 std::size_t chunkSize, 
									 std::size_t maxObjectSize)
									 : pLastAlloc_(0), pLastDealloc_(0)
									 , chunkSize_(chunkSize), maxObjectSize_(maxObjectSize) 
{   
}



struct CompareFixedAllocatorSize
	: std::binary_function<const FixedAllocator &, const FixedAllocator &, bool>
{
	bool operator()(const FixedAllocator& lhs, const FixedAllocator& rhs) const
	{
		return lhs.BlockSize() < rhs.BlockSize();
	}
};

void* SmallObjAllocator::Allocate(std::size_t numBytes)
{
	if (numBytes > maxObjectSize_) return operator new(numBytes);

	if (pLastAlloc_ && pLastAlloc_->BlockSize() == numBytes)
	{
		return pLastAlloc_->Allocate();
	}

	FixedAllocator fixAlloc(numBytes);
	Pool::iterator i = std::lower_bound(pool_.begin(), pool_.end(), fixAlloc, 
		CompareFixedAllocatorSize());
	if (i == pool_.end() || i->BlockSize() != numBytes)
	{
		i = pool_.insert(i, fixAlloc);
		pLastDealloc_ = &*pool_.begin();
	}
	pLastAlloc_ = &*i;
	return pLastAlloc_->Allocate();
}

void SmallObjAllocator::Deallocate(void* p, std::size_t numBytes)
{
	if (numBytes > maxObjectSize_) return operator delete(p);

	if (pLastDealloc_ && pLastDealloc_->BlockSize() == numBytes)
	{
		pLastDealloc_->Deallocate(p);
		return;
	}
	Pool::iterator i = std::lower_bound(pool_.begin(), pool_.end(), FixedAllocator(numBytes), 
		CompareFixedAllocatorSize());
	assert(i != pool_.end());
	assert(i->BlockSize() == numBytes);
	pLastDealloc_ = &*i;
	pLastDealloc_->Deallocate(p);
}