#pragma once

#include <assert.h>
#include <string>

template<typename T>
class CPoolNode : public T
{
	public:
		CPoolNode()
		{
			m_pNext = NULL;
			m_pPrev = NULL;
		}

		virtual ~CPoolNode()
		{
		}

	public:
		CPoolNode<T> * m_pNext;
		CPoolNode<T> * m_pPrev;
};

template<typename T>
class CDynamicPool
{
	public:
		typedef CPoolNode<T> TNode;

	public:
		CDynamicPool()
		{
			Initialize();
		}

		virtual ~CDynamicPool()
		{
			assert(m_pFreeList==NULL && "CDynamicPool::~CDynamicPool() - NOT Clear");
			assert(m_pUsedList==NULL && "CDynamicPool::~CDynamicPool() - NOT Clear");
			Clear();
		}

		void Initialize()
		{
			m_nodes = NULL;
			m_nodeCount = 0;

			m_pFreeList = NULL;
			m_pUsedList = NULL;
		}

		void SetName(const char* c_szName)
		{
			m_stName = c_szName;
		}

		DWORD GetCapacity()
		{
			return m_nodeCount;
		}

		T* Alloc()
		{
			TNode* pnewNode;

			if (m_pFreeList)
			{
				pnewNode = m_pFreeList;
				m_pFreeList = m_pFreeList->m_pNext;
			}
			else
			{
				pnewNode = AllocNode();
			}

			if (!pnewNode)
				return NULL;

			if (!m_pUsedList)
			{
				m_pUsedList = pnewNode;
				m_pUsedList->m_pPrev = m_pUsedList->m_pNext = NULL;
			}
			else
			{
				m_pUsedList->m_pPrev = pnewNode;
				pnewNode->m_pNext = m_pUsedList;
				pnewNode->m_pPrev = NULL;
				m_pUsedList = pnewNode;
			}
			//Tracef("%s Pool Alloc %p\n", m_stName.c_str(), pnewNode);
			return (T*) pnewNode;
		}

		void Free(T * pdata)
		{
			TNode* pfreeNode = (TNode*) pdata;

			if (pfreeNode == m_pUsedList)
			{
				if (NULL != (m_pUsedList = m_pUsedList->m_pNext))
					m_pUsedList->m_pPrev = NULL;
			}
			else
			{
				if (pfreeNode->m_pNext)
					pfreeNode->m_pNext->m_pPrev = pfreeNode->m_pPrev;

				if (pfreeNode->m_pPrev)
					pfreeNode->m_pPrev->m_pNext = pfreeNode->m_pNext;
			}

			pfreeNode->m_pPrev = NULL;
			pfreeNode->m_pNext = m_pFreeList;
			m_pFreeList = pfreeNode;
			//Tracef("%s Pool Free\n", m_stName.c_str());
		}

		void FreeAll()
		{
			TNode * pcurNode;
			TNode * pnextNode;

			pcurNode = m_pUsedList;

			while (pcurNode)
			{
				pnextNode = pcurNode->m_pNext;
				Free(pcurNode);
				pcurNode = pnextNode;
			}
		}

		void Clear()
		{
			TNode* pcurNode;
			TNode* pnextNode;

			DWORD count = 0;

			pcurNode = m_pFreeList;
			while (pcurNode)
			{
				pnextNode = pcurNode->m_pNext;
				delete pcurNode;
				pcurNode = pnextNode;
				++count;
			}
			m_pFreeList = NULL;

			pcurNode = m_pUsedList;
			while (pcurNode)
			{
				pnextNode = pcurNode->m_pNext;
				delete pcurNode;
				pcurNode = pnextNode;
				++count;
			}

			m_pUsedList = NULL;

			assert(count==m_nodeCount && "CDynamicPool::Clear()");

			m_nodeCount=0;
		}

	protected:
		TNode* AllocNode()
		{
			++m_nodeCount;
			return new TNode;
		}

	protected:
		TNode *		m_nodes;
		TNode *		m_pFreeList;
		TNode *		m_pUsedList;

		DWORD		m_nodeCount;
		std::string	m_stName;
};

template<typename T>
struct PoolNode {
	T* block;
	PoolNode* next;
};

template<typename T>
struct PoolAllocation {
	T* chunk;
	size_t num_blocks;
	PoolNode<T>* nodes;
};

template<typename T>
struct PoolDetail {
	typedef T* PointerType;
	typedef T* ArithmeticPointerType;

	static PointerType Alloc(size_t num) {
		return new T[num];
	}
	static void Free(PointerType p) {
		delete[] p;
	}
};

// Explicit specialization of PoolDetail for raw non-initialized memory blocks.
template<>
struct PoolDetail<void> {
	typedef void* PointerType;
	typedef char* ArithmeticPointerType;

	static PointerType Alloc(size_t num) {
		return ::malloc(num);
	}
	static void Free(PointerType p) {
		::free(p);
	}
};

// Generic grow-only pool of arrays.
// Non-void template parameter type T should provide no-arg default constructor.
template<typename T>
class ArrayPool {
public:
	ArrayPool(size_t array_size, size_t initial_capacity = 0)
			: free_(NULL),
			  array_size_(array_size),
			  capacity_(0),
			  alloc_count_(0),
			  alloc_index_of_last_release_(0) {
		assert(array_size_ != 0);
		if (initial_capacity != 0) {
			Reserve(initial_capacity);
		}
	}
	~ArrayPool() {
		CleanUp();
	}

	// Acquires an available array from the pool.
	T* Acquire() {
		if (free_ == NULL) {
			if (Stretch(capacity_) == false) {
				return NULL;
			}
		}
		assert(free_ != NULL);
		PointerType p = free_->block;
		free_ = free_->next;
		return p;
	}
	// Releases the specified array and returns it to the pool.
	void Release(T* p) {
		if (p == NULL) {
			return;
		}
		size_t index = alloc_index_of_last_release_;
		for (size_t i = 0; i < alloc_count_; ++i, ++index) {
			if (index >= alloc_count_) {
				index = 0;
			}
			AllocationType& alloc = allocated_[index];
			ArithmeticPointerType ptr = static_cast<ArithmeticPointerType>(p);
			ArithmeticPointerType begin = static_cast<ArithmeticPointerType>(
				alloc.chunk);
			ArithmeticPointerType end = begin + (array_size_ * alloc.num_blocks);
			if (begin <= ptr && ptr < end) {
				size_t node_index = (ptr - begin) / array_size_;
				NodeType* node = alloc.nodes + node_index;
				assert(node->block == p);
				if (node->block != p) {
					break;
				}
				node->next = free_;
				free_ = node;
				alloc_index_of_last_release_ = index;
				break;
			}
		}
	}

	// Requests that the capacity of the pool be enough to hold at least n arrays.
	void Reserve(size_t n) {
		if (n <= capacity_) {
			return;
		}
		Stretch(n - capacity_);
	}
	// Frees all the memory blocks allocated by the pool.
	void CleanUp() {
		if (alloc_count_ == 0) {
			return;
		}
		while (alloc_count_ != 0) {
			AllocationType& alloc = allocated_[--alloc_count_];
			DetailType::Free(alloc.chunk);
			delete[] alloc.nodes;
		}
		capacity_ = 0;
		free_ = NULL;
	}

	// Gets the size of an array in the pool.
	size_t array_size() const { return array_size_; }
	// Gets the current total capacity, in number of arrays, of the pool.
	size_t capacity() const { return capacity_; }

private:
	typedef PoolNode<T> NodeType;
	typedef PoolAllocation<T> AllocationType;
	typedef PoolDetail<T> DetailType;
	typedef typename DetailType::PointerType PointerType;
	typedef typename DetailType::ArithmeticPointerType ArithmeticPointerType;

	static const size_t kMaxAllocCount = sizeof(size_t) * CHAR_BIT;

	bool Stretch(size_t increment) {
		if (increment == 0) {
			++increment; // minimum increment 1
		}
		if (alloc_count_ >= kMaxAllocCount) {
			return false;
		}

		ArithmeticPointerType p = static_cast<ArithmeticPointerType>(
			DetailType::Alloc(array_size_ * increment));
		assert(p != NULL);
		if (p == NULL) {
			return false;
		}
		NodeType* node = new NodeType[increment];
		assert(node != NULL);
		if (node == NULL) {
			DetailType::Free(p);
			return false;
		}

		AllocationType& alloc = allocated_[alloc_count_++];
		alloc.chunk = p;
		alloc.num_blocks = increment;
		alloc.nodes = node;

		NodeType* tail = free_;
		NodeType** link = &free_;
		for (size_t i = 0; i < increment ; ++i, ++node, p += array_size_) {
			node->block = p;
			*link = node;
			link = &(node->next);
		}
		*link = tail;

		capacity_ += increment;
		return true;
	}

	NodeType* free_;

	size_t array_size_;
	size_t capacity_;

	AllocationType allocated_[kMaxAllocCount];
	size_t alloc_count_;
	size_t alloc_index_of_last_release_;

	// No copy
	ArrayPool(const ArrayPool&);
	void operator=(const ArrayPool&);
};

// Special alias for the pool of raw(non-typed) non-initialized memory blocks.
typedef ArrayPool<void> Pool;

// Variable-length memory pool backed by multiple fixed-length pools.
class MemoryPool {
public:
	MemoryPool() {}
	~MemoryPool() {
		PoolMapType::iterator it = pools_.begin(), end = pools_.end();
		for ( ; it != end; ++it) {
			delete (it->second);
		}
	}
	// Acquires a memory block of specified size from a pool.
	void* Acquire(size_t size) {
		Pool* pool;
		PoolMapType::iterator it = pools_.find(size);
		if (it != pools_.end()) {
			pool = it->second;
		} else {
			pool = new Pool(size);
			pools_.insert(PoolMapType::value_type(size, pool));
		}
		return pool->Acquire();
	}
	// Releases the specified memory block and returns it to a pool.
	void Release(void* p, size_t size) {
		PoolMapType::iterator it = pools_.find(size);
		if (it == pools_.end()) {
			return;
		}
		Pool* pool = it->second;
		pool->Release(p);
	}
private:
	typedef std::unordered_map<size_t, Pool*> PoolMapType;
	PoolMapType pools_;
};

// Grow-only simple object pool, relying on ctor/dtor.
template<class T> // T should provide no-arg default constructor
class ObjectPool {
public:
	ObjectPool(size_t initial_capacity = 0)
		: pool_(sizeof(T), initial_capacity) {}
	~ObjectPool() {}

	// Constructs a new object from the pool.
	T* Construct() {
		void* p = pool_.Acquire();
		if (p == NULL) {
			return NULL;
		}
		return new (p) T();
	}
	// Destroys the specified object and returns it to the pool.
	void Destroy(T* p) {
		if (p == NULL) {
			return;
		}
		p->~T();
		pool_.Release(p);
	}

	// Requests that the pool capacity be enough to hold at least n objects.
	void Reserve(size_t n) {
		pool_.Reserve(n);
	}

private:
	Pool pool_;

	// No copy
	ObjectPool(const ObjectPool&);
	void operator=(const ObjectPool&);
};
