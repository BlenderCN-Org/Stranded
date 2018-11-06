
#include "StdAfx.h"
#include "KChinese.h"
CKChinese::CKChinese(void) : m_nOld(0)
{
	memset(m_szName, 0, 64);
}
CKChinese::~CKChinese(void)
{
}
int CKChinese::GetOld(void) const
{
	return m_nOld;
}
void CKChinese::SetOld(int nOld)
{
	this->m_nOld = nOld;
}
const char* CKChinese::GetName(void) const
{
	return m_szName;
}
void CKChinese::SetName(const char* szName)
{
	strncpy(m_szName, szName, 64);
}
/* 导出函数定义 */
IKPerson* __cdecl GetIKPerson(void)
{
	IKPerson* pIKPerson = new CKChinese();
	return pIKPerson;
}