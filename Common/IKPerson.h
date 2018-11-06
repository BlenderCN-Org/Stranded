#ifndef _IKPERSON_H_
#define _IKPERSON_H_
#ifdef DLL_EXPORT
#define DLL_API	extern "C" __declspec(dllexport)
#else
#define DLL_API extern "C" __declspec(dllimport)
#endif
/*
	�������ӿ�������ã�
	�ܲ��ö�̬���÷�ʽʹ�������
*/
class IKPerson
{
public:
	virtual ~IKPerson(void)	//���ڻ��࣬��ʾ���������������Ǹ���ϰ�ߣ�ע�⣬Ϊʲô��google��
	{
	}
	virtual int GetOld(void) const = 0;
	virtual void SetOld(int nOld) = 0;
	virtual const char* GetName(void) const = 0;
	virtual void SetName(const char* szName) = 0;
};
/* ������������ */
DLL_API IKPerson* _cdecl GetIKPerson(void);
typedef IKPerson* (__cdecl *PFNGetIKPerson)(void);
#endif