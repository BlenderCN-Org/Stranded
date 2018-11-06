#ifndef __PARAM_H__
#define __PARAM_H__

#include <assert.h>

// class CParam : public cocos2d::Ref
class CParam
{
public:
	virtual ~CParam() {}

	inline void SetParamCount(unsigned int count) { m_uiParamCount = count; }
	inline unsigned int GetParamCount() { return m_uiParamCount; }

	template<class R, class T> static R param_cast(T t);

	template<class T1> static CParam* CreateParam(T1 t1);
	template<class T1, class T2> static CParam* CreateParam(T1 t1, T2 t2);
	template<class T1, class T2, class T3> static CParam* CreateParam(T1 t1, T2 t2, T3 t3);
	template<class T1, class T2, class T3, class T4> static CParam* CreateParam(T1 t1, T2 t2, T3 t3, T4 t4);
	template<class T1, class T2, class T3, class T4, class T5> static CParam* CreateParam(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5);

	template<class T1> void GetParam(T1 &t1);
	template<class T1, class T2> void GetParam(T1 &t1, T2 &t2);
	template<class T1, class T2, class T3> void GetParam(T1 &t1, T2 &t2, T3 &t3);
	template<class T1, class T2, class T3, class T4> void GetParam(T1 &t1, T2 &t2, T3 &t3, T4 &t4);
	template<class T1, class T2, class T3, class T4, class T5> void GetParam(T1 &t1, T2 &t2, T3 &t3, T4 &t4, T5 &t5);

private:
	unsigned int m_uiParamCount;
};

template<class T1, class T2> class CParam2 : public CParam
{
public:
	CParam2(T1 &t1, T2 &t2) :
		v1(t1), v2(t2)
	{}

	CParam2() {}

	virtual ~CParam2() {}

	T1 v1;
	T2 v2;
};

class CNullType;
template<class T1> struct CParam2<T1, CNullType> : public CParam
{
public:
	CParam2(T1 &t1) : v1(t1) {}
	CParam2() {}
	virtual ~CParam2() {}

	T1 v1;
};

template<class R, class T> R CParam::param_cast(T t)
{
	R r = dynamic_cast<R>(t);

	assert(r && t);

	return r;
}

template<class T1> CParam* CParam::CreateParam(T1 t1)
{
	CParam* pParam = new CParam2<T1, CNullType>(t1);
	// pParam->autorelease();
	pParam->SetParamCount(1);
	return pParam;
}

template<class T1, class T2> CParam* CParam::CreateParam(T1 t1, T2 t2)
{
	CParam* pParam = new Param2<T1, T2>(t1, t2);
	// pParam->autorelease();
	pParam->SetParamCount(2);
	return pParam;
}

template<class T1, class T2, class T3> CParam* CParam::CreateParam(T1 t1, T2 t2, T3 t3)
{
	CParam2<T2, T3> t(t2, t3);

	CParam* pParam = new CParam2<T1, CParam2<T2, T3> >(t1, t);
	// pParam->autorelease();
	pParam->SetParamCount(3);
	return pParam;
}

template<class T1, class T2, class T3, class T4> CParam* CParam::CreateParam(T1 t1, T2 t2, T3 t3, T4 t4)
{
	CParam2<T3, T4> pT1(t3, t4);
	CParam2<T2, CParam2<T3, T4> > pT2(t2, pT1);

	CParam* pParam = new CParam2<T1, CParam2<T2, CParam2<T3, T4> > >(t1, pT2);
	// pParam->autorelease();
	pParam->SetParamCount(4);
	return pParam;
}

template<class T1, class T2, class T3, class T4, class T5> CParam* CParam::CreateParam(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
	CParam2<T4, T5> pT1(t4, t5);
	CParam2<T3, CParam2<T4, T5> > pT2(t3, pT1);
	CParam2<T2, CParam2<T3, CParam2<T4, T5> > > pT3(t2, pT2);

	CParam* pParam = new CParam2<T1, CParam2<T2, CParam2<T3, CParam2<T4, T5> > > >(t1, pT3);
	// pParam->autorelease();
	pParam->SetParamCount(5);
	return pParam;
}

#define PARAM1(T1) CParam2<T1, CNullType>
#define PARAM2(T1, T2) CParam2<T1, T2>
#define PARAM3(T1, T2, T3) CParam2<T1, CParam2<T2, T3> >
#define PARAM4(T1, T2, T3, T4) CParam2<T1, CParam2<T2, CParam2<T3, T4> > >
#define PARAM5(T1, T2, T3, T4) CParam2<T1, CParam2<T2, CParam2<T3, CParam2<T4, T5> > > >

template<class T1> void CParam::GetParam(T1 &t1)
{
	PARAM1(T1)* p = CParam::param_cast<PARAM1(T1)*>(this);
	t1 = p->v1;
}

template<class T1, class T2> void CParam::GetParam(T1 &t1, T2 &t2)
{
	PARAM2(T1, T2)* p = CParam::param_cast<PARAM2(T1, T2)*>(this);

	t1 = p->v1;
	t2 = p->v2;
}

template<class T1, class T2, class T3> void CParam::GetParam(T1 &t1, T2 &t2, T3 &t3)
{
	PARAM2(T1, T2, T3)* p = CParam::param_cast<PARAM3(T1, T2, T3)*>(this);

	t1 = p->v1;
	t2 = p->v2.v1;
	t3 = p->v2.v2;
}

template<class T1, class T2, class T3, class T4> void CParam::GetParam(T1 &t1, T2 &t2, T3 &t3, T4 &t4)
{
	PARAM4(T1, T2, T3, T4)* p = CParam::param_cast<PARAM4(T1, T2, T3, T4)*>(this);

	t1 = p->v1;
	t2 = p->v2.v1;
	t3 = p->v2.v2.v1;
	t3 = p->v2.v2.v2;
}

template<class T1, class T2, class T3, class T4, class T5> void CParam::GetParam(T1 &t1, T2 &t2, T3 &t3, T4 &t4, T5 &t5)
{
	PARAM5(T1, T2, T3, T4, T5)* p = CParam::param_cast<PARAM5(T1, T2, T3, T4, T5)*>(this);

	t1 = p->v1;
	t2 = p->v2.v1;
	t3 = p->v2.v2.v1;
	t4 = p->v2.v2.v2.v1;
	t5 = p->v2.v2.v2.v2;
}

#endif
