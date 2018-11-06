#pragma once

#include "YKMapManager.h"

template <typename T>
class YKWeakPtr
{
public:
	typedef typename YKSmartPtr<T>					value_type;
	typedef typename value_type::pointer			pointer;
	typedef typename value_type::const_pointer		const_pointer;
	typedef typename value_type::reference			reference;
	typedef typename value_type::const_reference	const_reference;
public:
	YKWeakPtr() : m_nId(0), m_pObj(0) {}

	YKWeakPtr(const YKWeakPtr& rhs)
		: m_nId(rhs.m_nId), m_pObj(rhs.m_pObj)
	{}

	template <typename T1>
	YKWeakPtr(const YKWeakPtr<T1>& rhs)
		: m_nId(rhs.m_nId), m_pObj(rhs.m_pObj)
	{}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class SP1>
	YKWeakPtr(YKSmartPtr<T1, OP1, KP1, SP1>& rhs)
	{
		m_nId = rhs.ValidObj() ? rhs->GetId() : 0;
		m_pObj = 0;
	}

	template <typename T1>
	YKWeakPtr& operator= (const YKWeakPtr<T1>& rhs)
	{
		m_nId = rhs.m_nId;
		m_pObj = rhs.m_pObj;
		return *this;
	}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class SP1>
	YKWeakPtr& operator= (const YKSmartPtr<T1, OP1, KP1, SP1>& rhs)
	{
		m_nId = rhs.ValidObj() ? rhs->GetId() : 0;
		m_pObj = 0;
		return *this;
	}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class SP1>
	YKWeakPtr& operator= (YKSmartPtr<T1, OP1, KP1, SP1>& rhs)
	{
		m_nId = rhs.ValidObj() ? rhs->GetId() : 0;
		m_pObj = 0;
		return *this;
	}

	bool ValidObj() 
	{
		value_type obj = g_pMapManager->GetBasePtr(T::Obj_Type, m_nId);
		if (!obj.ValidObj())
		{
			m_pObj = 0;
			return false;
		}

		m_pObj = &(*obj);
		return true;
	}

	pointer operator-> () const
	{
		assert(m_pObj);
		return m_pObj;
	}

	template <typename T1, template <class> class CT1> friend class YKContainPtr;

private:
	YK_ULONG m_nId;
	T* m_pObj;
};

template <typename T>
struct YKVectorT
{
protected:
	typedef typename std::vector<T>			value_type;
	typedef typename value_type::iterator	iter_type;

	inline iter_type	RFind(value_type& val, const T& id)
	{
		return std::find(val.begin(), val.end(), id);
	}

	inline void RInsert(value_type& val, const T& id)
	{
		val.push_back(id);
	}

	inline void RRemove(value_type& val, const T& id)
	{
		val.erase(std::remove(val.begin(), val.end(), id), val.end());
	}
};

template <typename T>
struct YKListT
{
protected:
	typedef typename std::list<T>			value_type;
	typedef typename value_type::iterator	iter_type;

	inline iter_type RFind(value_type& val, const T& id)
	{
		return std::find(val.begin(), val.end(), id);
	}

	inline void RInsert(value_type& val, const T& id)
	{
		val.push_back(id);
	}

	inline void RRemove(value_type& val, const T& id)
	{
		val.erase(std::remove(val.begin(), val.end(), id), val.end());
	}
};

template <typename T>
struct YKSetT
{
protected:
	typedef typename std::set<T>			value_type;
	typedef typename value_type::iterator	iter_type;

	inline iter_type RFind(value_type& val, const T& id)
	{
		return val.find(id);
	}

	inline void RInsert(value_type& val, const T& id)
	{
		val.insert(id);
	}

	inline void RRemove(value_type& val, const T& id)
	{
		val.erase(id);
	}
};

template <typename T,
template <class> class ContainerType = YKVectorT>
class YKContainPtr : public ContainerType<YK_ULONG>
{
	typedef YKSmartPtr<T>							object_type;
	typedef ContainerType<YK_ULONG>					container_type;
	typedef typename container_type::value_type		ID_TYPE;
	typedef typename container_type::iter_type		ID_ITER;

public:
	YKContainPtr() {}
	YKContainPtr(YKContainPtr& rhs)
	{
		m_ids = rhs.m_ids;
	}

	template <typename T1,
		template <class> class CT1>
	YKContainPtr(YKContainPtr<T1, CT1>& rhs)
	{
		Clear();
		//m_ids.resize(rhs.Count());
		std::copy(rhs.m_ids.begin(), rhs.m_ids.end(), std::back_inserter<ID_TYPE >(m_ids));
	}

	YKContainPtr& operator= (const YKContainPtr& rhs)
	{
		m_ids = rhs.m_ids;
		return *this;
	}

// 	YKContainPtr& operator= (YKContainPtr& rhs)
// 	{
// 		m_ids = rhs.m_ids;
// 		return *this;
// 	}

	template <typename T1,
		template <class> class CT1>
	YKContainPtr& operator= (const YKContainPtr<T1, CT1>& rhs)
	{
		Clear();
		//m_ids.resize(rhs.Count());
		std::copy(rhs.m_ids.begin(), rhs.m_ids.end(), std::back_inserter<ID_TYPE >(m_ids));
		return *this;
	}

	void Begin() {m_iter = m_ids.begin();}
	bool NotEnd() {return !(m_iter == m_ids.end());}
	void Step() {if (m_iter != m_ids.end()) ++m_iter;}
	YK_ULONG GetId() {if (m_iter != m_ids.end()) return *m_iter; return 0;} 
	object_type Get()
	{
		return g_pMapManager->GetBasePtr(T::Obj_Type, *m_iter);
	}

	object_type Front()
	{
		ID_ITER iter = m_ids.begin();
		if (iter != m_ids.end())
			return g_pMapManager->GetBasePtr(T::Obj_Type, *iter);

		return NULL;
	}

	object_type Back()
	{
		ID_TYPE::reverse_iterator rIter = m_ids.rbegin();
		if (iterator != m_ids.rend())
			return g_pMapManager->GetBasePtr(T::Obj_Type, *rIter);

		return NULL;
	}

	object_type Get(YK_ULONG id)
	{
		if (RFind(m_ids, id) != m_ids.end())
			return g_pMapManager->GetBasePtr(T::Obj_Type, m_nId);

		return NULL;
	}

	void Insert(YK_ULONG id)
	{
		if (id <= 0)
			return;

		RInsert(m_ids, id);
	}

	template <typename T1>
	void Insert(YKWeakPtr<T1>& rhs)
	{
		Insert(rhs.m_nId);
	}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class SP1>
	void Insert(YKSmartPtr<T1, OP1, KP1, SP1>& rhs)
	{
		YKWeakPtr<T1> obj = rhs;
		Insert(obj);
	}

	void Remove(YK_ULONG id)
	{
		RRemove(m_ids, id);
	}

	// 为考虑通用性，此处方式效率不高，请注意使用
	void ClearInvalidId()
	{
		ID_TYPE idTemp;
		for (Begin(); NotEnd(); Step())
		{
			if (Get().ValidObj())
			{
				RInsert(idTemp, GetId());
			}
		}

		m_ids = idTemp;
	}

	bool Exist(YK_ULONG id)
	{
		return RFind(m_ids, id) != m_ids.end();
	}

	void Clear() {m_ids.clear();}
	bool Empty() {return m_ids.empty();}
	std::size_t Count() {return m_ids.size();}

	ID_TYPE	GetIdList()
	{return m_ids;}

	template <typename T1,
		template <class> class CT1>
	friend class YKContainPtr;

private:
	ID_TYPE	m_ids;
	ID_ITER m_iter;
};