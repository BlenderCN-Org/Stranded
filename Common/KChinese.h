#pragma once
#define DLL_EXPORT
#include "ikperson.h"
class CKChinese :
	public IKPerson
{
public:
	CKChinese(void);
	~CKChinese(void);
	virtual int GetOld(void) const;
	virtual void SetOld(int nOld);
	virtual const char* GetName(void) const;
	virtual void SetName(const char* szName);
private:
	int m_nOld;
	char m_szName[64];
};
