
#ifndef _SMARTPTR_H__
#define _SMARTPTR_H__

#include "MemoryPolicy.h"
#include "SmallObj.h"

template <class Target, class Source>
inline Target Safe_DownCast(Source* x)
{
	assert(dynamic_cast<Target>(x) == x);
	return static_cast<Target>(x);
}

template <bool flag, typename T, typename U>
struct Select
{
	template <bool flag>
	struct In
	{
		typedef T	Result;
	};

	template<>
	struct In<false>
	{
		typedef U	Result;
	};
public:
	typedef	typename In<flag>::Result	Result;
};

template <class P>
class RefCounted
{
public:
	RefCounted() : pCount(0) {}

	RefCounted(const RefCounted& rhs) : pCount(rhs.pCount)
	{}

	template <typename P1>
	RefCounted(const RefCounted<P1>& rhs) : pCount(reinterpret_cast<const RefCounted&>(rhs).pCount)
	{}

	void NewCount()
	{
 		pCount = static_cast<unsigned int*>(
 			SmallObject<>::operator new(sizeof(unsigned int)));
		//pCount = new unsigned int;
		assert(pCount);
		*pCount = 1;
	}

	P Clone(const P& val)
	{
		if (pCount)
			++*pCount;
		return val;
	}

	bool Release(const P&)
	{
		if (pCount && (!--*pCount))
		{
			SmallObject<>::operator delete(pCount, sizeof(unsigned int));
			//delete pCount;
			pCount = 0;
			return true;
		}

		return false;
	}

	void Swap(RefCounted& rhs)
	{
		std::swap(pCount, rhs.pCount);
	}

	enum {destructiveCopy = false};

private:
	unsigned int* pCount;
};

template <template <class> class ThreadingPolicy>
struct RefCountedMTAdj
{
	template <class P>
	class RefCountedMT : public ThreadingPolicy< RefCountedMT<P> >
	{
		typedef ThreadingPolicy< RefCountedMT<P> > base_type;
		typedef typename base_type::IntType       CountType;
		typedef volatile CountType               *CountPtrType;

	public:
		RefCountedMT() : pCount(0) {}

		void NewCount()
		{
			pCount = static_cast<CountPtrType>(
				SmallObject<ThreadingPolicy>::operator new(sizeof(*pCount)));
			//pCount = new unsigned int;
			assert(pCount);
			*pCount = 1;
		}

		RefCountedMT(const RefCountedMT& rhs) 
			: pCount(rhs.pCount)
		{}

		//MWCW lacks template friends, hence the following kludge
		template <typename P1>
		RefCountedMT(const RefCountedMT<P1>& rhs) 
			: pCount(reinterpret_cast<const RefCountedMT<P>&>(rhs).pCount)
		{}

		P Clone(const P& val)
		{
			ThreadingPolicy<RefCountedMT>::AtomicIncrement(*pCount);
			return val;
		}

		bool Release(const P&)
		{
			if (pCount && !ThreadingPolicy<RefCountedMT>::AtomicDecrement(*pCount))
			{
				SmallObject<ThreadingPolicy>::operator delete(
					const_cast<CountType *>(pCount), 
					sizeof(*pCount));
				return true;
			}
			return false;
		}

		void Swap(RefCountedMT& rhs)
		{ std::swap(pCount, rhs.pCount); }

	private:
		// Data
		CountPtrType pCount;
	};
};

template <class P>
struct NoCheck
{
	NoCheck() {}

	template <class P1>
	NoCheck(const NoCheck<P1>&) {}
	static void OnDefault(const P&) {}
	static void OnInit(const P&) {}
	static void OnDereference(const P&) {}
	static void Swap(NoCheck&) {}
};

template <class P>
struct AssertCheck
{
	AssertCheck() {}
	
	template <class P1>
	AssertCheck(const AssertCheck<P1>&) {}
	template <class P1>
	AssertCheck(const NoCheck<P1>&) {}

	static void OnDefault(const P&) {}
	static void OnInit(const P&) {}
	static void OnDereference(P val)
	{
		assert(val);
		(void) val;
	}

	static void Swap(AssertCheck&) {}
};

struct NullPointerException : public std::runtime_error
{
	NullPointerException() : std::runtime_error("") {}
	const char* what() const throw()
	{return "Null Pointer Exception";}
};

template <class P>
struct NullCheck
{
	NullCheck() {}

	template <class P1>
	NullCheck(const NullCheck<P1>&) {}

	template <class P1>
	NullCheck(const NoCheck<P1>&) {}

	template <class P1>
	NullCheck(const AssertCheck<P1>&) {}

	static void OnDereference(P val)
	{if (!val) throw NullPointerException();}
	
	static void Swap(NullCheck&) {}
};

template <typename T, 
		template <class> class OwnershipPolicy = RefCounted,
		template <class> class CheckingPolicy = NoCheck,
		template <class> class MemoryPolicy = MemPolicyCached>
class SmartPtr : public MemoryPolicy<T>
	, public OwnershipPolicy<typename MemoryPolicy<T>::pointer>
	, public CheckingPolicy<typename MemoryPolicy<T>::stored_type>
{
	typedef MemoryPolicy<T>		MP;
	typedef OwnershipPolicy<typename MemoryPolicy<T>::pointer> OP;
	typedef CheckingPolicy<typename MemoryPolicy<T>::stored_type> KP;
public:
	typedef typename MP::reference			reference;
	typedef typename MP::const_reference	const_reference;
	typedef typename MP::pointer			pointer;
	typedef typename MP::const_pointer		const_pointer;
	typedef typename MP::value_type			value_type;

	enum {YK_destructiveCopy = OP::destructiveCopy};

	typedef typename Select<YK_destructiveCopy, SmartPtr, const SmartPtr>::Result		copySelect;	
public:
	SmartPtr(bool bNew = false)
	{
		if (bNew)
		{
			Create();
			NewCount();
		}
	}

	SmartPtr(int rhs)
	{
		assert(rhs == 0);
		SmartPtr temp;
		temp.Swap(*this);
	}

	SmartPtr(SmartPtr& rhs)
		: MP(rhs), OP(rhs), KP(rhs)
	{
		GetImplRef(*this) = OP::Clone(GetImplRef(rhs));
	}

	/*SmartPtr(pointer pObj)
	{
		Create();
		Copy(pObj);
		NewCount();
	}*/

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class MP1>
	SmartPtr(const SmartPtr<T1, OP1, KP1, MP1>& rhs)
		: MP(rhs), OP(rhs), KP(rhs)
	{
		GetImplRef(*this) = OP::Clone(Safe_DownCast<pointer>(GetImplRef(rhs)));
	}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class MP1>
	SmartPtr(SmartPtr<T1, OP1, KP1, MP1>& rhs)
		: MP(rhs), OP(rhs), KP(rhs)
	{
		GetImplRef(*this) = OP::Clone(Safe_DownCast<pointer>(GetImplRef(rhs)));
	}

	SmartPtr& operator= (copySelect& rhs)
	{
		SmartPtr temp(rhs);
		temp.Swap(*this);
		return *this;
	}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class MP1>
	SmartPtr& operator= (const SmartPtr<T1, OP1, KP1, MP1>& rhs)
	{
		SmartPtr temp(rhs);
		temp.Swap(*this);
		return *this;
	}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class MP1>
	SmartPtr& operator= (SmartPtr<T1, OP1, KP1, MP1>& rhs)
	{
		SmartPtr temp(rhs);
		temp.Swap(*this);
		return *this;
	}

	void Swap(SmartPtr& rhs)
	{
		OP::Swap(rhs);
		KP::Swap(rhs);
		MP::Swap(rhs);
	}

	~SmartPtr() 
	{
		if (OP::Release(GetImpl(*static_cast<MP*>(this))))
		{
			Destroy();
		}
	}

	pointer operator-> ()
	{
		KP::OnDereference(GetImplRef(*this));
		return MP::operator ->();
	}

	pointer operator-> () const
	{
		KP::OnDereference(GetImplRef(*this));
		return MP::operator ->();
	}

	reference operator* ()
	{
		KP::OnDereference(GetImplRef(*this));
		return MP::operator *();
	}

	reference operator* () const
	{
		KP::OnDereference(GetImplRef(*this));
		return MP::operator *();
	}

	inline friend bool operator== (const SmartPtr& lhs, const_pointer rhs)
	{return GetImpl(lhs) == rhs;}
	inline friend bool operator== (const_pointer lhs, const SmartPtr& rhs)
	{return rhs == lhs;}
	inline friend bool operator!= (const SmartPtr& lhs, const_pointer rhs)
	{return !(lhs == rhs);}
	inline friend bool operator!= (const_pointer lhs, const SmartPtr& rhs)
	{return rhs != lhs;}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class MP1>
	bool operator== (const SmartPtr<T1, OP1, KP1, MP1>& rhs) const
	{return *this == GetImpl(rhs);}

	template <typename T1, 
		template <class> class OP1,
		template <class> class KP1,
		template <class> class MP1>
	bool operator!= (const SmartPtr<T1, OP1, KP1, MP1>& rhs) const
	{return !(*this == rhs);}

// 	template <typename T1, 
// 		template <class> class OP1,
// 		template <class> class KP1,
// 		template <class> class MP1>
// 	bool operator< (const SmartPtr<T1, OP1, KP1, MP1>& rhs) const
// 	{return *this < GetImpl(rhs);}

	bool ValidObj()
	{
		return GetImpl(*this) != 0;
	}

	bool ValidObj() const
	{
		return GetImpl(*this) != 0;
	}

protected:
	pointer GetPointer()
	{
		return GetImpl(*this);
	}

	template <typename T1> friend class YKObjPtr;
	template <typename _Key, typename _Value> friend class YKMap;
};

#endif	// _SMARTPTR_H__