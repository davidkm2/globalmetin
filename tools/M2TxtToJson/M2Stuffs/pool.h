#pragma once

#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>

template <typename T>
class CPoolNode : public T
{
	public:
		CPoolNode()
		{
			m_pNext = nullptr;
			m_pPrev = nullptr;
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
			assert(m_pFreeList==nullptr && "CDynamicPool::~CDynamicPool() - NOT Clear");
//			assert(m_pUsedList==nullptr && "CDynamicPool::~CDynamicPool() - NOT Clear");
			Clear();
		}

		void Initialize()
		{
			m_nodes = nullptr;
			m_nodeCount = 0;

			m_pFreeList = nullptr;
			m_pUsedList = nullptr;
		}

		void SetName(const char* c_szName)
		{
			m_stName = c_szName;
		}

		uint32_t GetCapacity()
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
				return nullptr;

			if (!m_pUsedList)
			{
				m_pUsedList = pnewNode;
				m_pUsedList->m_pPrev = m_pUsedList->m_pNext = nullptr;
			}
			else
			{
				m_pUsedList->m_pPrev = pnewNode;
				pnewNode->m_pNext = m_pUsedList;
				pnewNode->m_pPrev = nullptr;
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
				if (nullptr != (m_pUsedList = m_pUsedList->m_pNext))
					m_pUsedList->m_pPrev = nullptr;
			}
			else
			{
				if (pfreeNode->m_pNext)
					pfreeNode->m_pNext->m_pPrev = pfreeNode->m_pPrev;

				if (pfreeNode->m_pPrev)
					pfreeNode->m_pPrev->m_pNext = pfreeNode->m_pNext;
			}

			pfreeNode->m_pPrev = nullptr;
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

			uint32_t count = 0;

			pcurNode = m_pFreeList;
			while (pcurNode)
			{
				pnextNode = pcurNode->m_pNext;
				delete pcurNode;
				pcurNode = pnextNode;
				++count;
			}
			m_pFreeList = nullptr;

			pcurNode = m_pUsedList;
			while (pcurNode)
			{
				pnextNode = pcurNode->m_pNext;
				delete pcurNode;
				pcurNode = pnextNode;
				++count;
			}

			m_pUsedList = nullptr;

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

		uint32_t		m_nodeCount;
		std::string	m_stName;
};
