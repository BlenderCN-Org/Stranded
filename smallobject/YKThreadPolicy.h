#pragma once

#include <Windows.h>
namespace POLICY_THREAD
{
	template <class T>
	class SingleThread
	{
	public:
		struct Lock
		{
			Lock() {}
			explicit Lock(const SingleThread&) {}
		};

		typedef T	VolatileType;
		typedef int IntType;

		static IntType AtomicAdd(volatile IntType& lval, IntType val)
		{ return lval += val; }

		static IntType AtomicSubtract(volatile IntType& lval, IntType val)
		{ return lval -= val; }

		static IntType AtomicMultiply(volatile IntType& lval, IntType val)
		{ return lval *= val; }

		static IntType AtomicDivide(volatile IntType& lval, IntType val)
		{ return lval /= val; }

		static IntType AtomicIncrement(volatile IntType& lval)
		{ return ++lval; }

		static IntType AtomicDecrement(volatile IntType& lval)
		{ return --lval; }

		static void AtomicAssign(volatile IntType & lval, IntType val)
		{ lval = val; }

		static void AtomicAssign(IntType & lval, volatile IntType & val)
		{ lval = val; }
	};

	template <class T>
	class ObjectLevelThread
	{
		mutable CRITICAL_SECTION mtx_;

	public:
		ObjectLevelThread()
		{
			::InitializeCriticalSection(&mtx_);
		}

		~ObjectLevelThread()
		{
			::DeleteCriticalSection(&mtx_);
		}

		class Lock;
		friend class Lock;

		class Lock
		{
			ObjectLevelThread const& host_;

			Lock(const Lock&);
			Lock& operator=(const Lock&);
		public:

			explicit Lock(const ObjectLevelThread& host) : host_(host)
			{
				::EnterCriticalSection(&host_.mtx_);
			}

			~Lock()
			{
				::LeaveCriticalSection(&host_.mtx_);
			}
		};

		typedef volatile T VolatileType;
		typedef LONG IntType;

		static IntType AtomicIncrement(volatile IntType& lval)
		{
#ifdef YK_X64
			return InterlockedIncrement64(&const_cast<IntType&>(lval)); 
#else
			return InterlockedIncrement(&const_cast<IntType&>(lval)); 
#endif
		}

		static IntType AtomicDecrement(volatile IntType& lval)
		{
#ifdef YK_X64
			return InterlockedDecrement64(&const_cast<IntType&>(lval)); 
#else
			return InterlockedDecrement(&const_cast<IntType&>(lval)); 
#endif
		}

		static void AtomicAssign(volatile IntType& lval, IntType val)
		{
#ifdef YK_X64
			InterlockedExchange64(&const_cast<IntType&>(lval), val); 
#else
			InterlockedExchange(&const_cast<IntType&>(lval), val); 
#endif
		}

		static void AtomicAssign(IntType& lval, volatile IntType& val)
		{
#ifdef YK_X64
			InterlockedExchange64(&lval, val); 
#else
			InterlockedExchange(&lval, val); 
#endif
		}
	};

	template <class T>
	class ClassLevelThread
	{
		struct Initializer
		{   
			CRITICAL_SECTION mtx_;

			Initializer()
			{
				::InitializeCriticalSection(&mtx_);
			}
			~Initializer()
			{
				::DeleteCriticalSection(&mtx_);
			}
		};

		static Initializer initializer_;

	public:
		class Lock;
		friend class Lock;

		class Lock
		{
			Lock(const Lock&);
			Lock& operator=(const Lock&);
		public:
			Lock()
			{
				::EnterCriticalSection(&initializer_.mtx_);
			}
			explicit Lock(const ClassLevelThread&)
			{
				::EnterCriticalSection(&initializer_.mtx_);
			}
			~Lock()
			{
				::LeaveCriticalSection(&initializer_.mtx_);
			}
		};

		typedef volatile T VolatileType;
		typedef LONG IntType;

		static IntType AtomicIncrement(volatile IntType& lval)
		{
#ifdef YK_X64
			return InterlockedIncrement64(&const_cast<IntType&>(lval));
#else
			return InterlockedIncrement(&const_cast<IntType&>(lval));
#endif
		}

		static IntType AtomicDecrement(volatile IntType& lval)
		{
#ifdef YK_X64
			return InterlockedDecrement64(&const_cast<IntType&>(lval)); 
#else
			return InterlockedDecrement(&const_cast<IntType&>(lval)); 
#endif
		}

		static void AtomicAssign(volatile IntType& lval, IntType val)
		{
#ifdef YK_X64
			InterlockedExchange64(&const_cast<IntType&>(lval), val); 
#else
			InterlockedExchange(&const_cast<IntType&>(lval), val); 
#endif
		}

		static void AtomicAssign(IntType& lval, volatile IntType& val)
		{
#ifdef YK_X64
			InterlockedExchange64(&lval, val); 
#else
			InterlockedExchange(&lval, val); 
#endif
		}
	};

	template <class T> typename ClassLevelThread<T>::Initializer ClassLevelThread<T>::initializer_;
}