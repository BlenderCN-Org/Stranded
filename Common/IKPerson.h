#ifndef _IKPERSON_H_
#define _IKPERSON_H_
#ifdef DLL_EXPORT
#define DLL_API	extern "C" __declspec(dllexport)
#else
#define DLL_API extern "C" __declspec(dllimport)
#endif
/*
	设计这个接口类的作用：
	能采用动态调用方式使用这个类
*/
class IKPerson
{
public:
	virtual ~IKPerson(void)	//对于基类，显示定义虚析构函数是个好习惯（注意，为什么请google）
	{
	}
	virtual int GetOld(void) const = 0;
	virtual void SetOld(int nOld) = 0;
	virtual const char* GetName(void) const = 0;
	virtual void SetName(const char* szName) = 0;
};
/* 导出函数声明 */
DLL_API IKPerson* _cdecl GetIKPerson(void);
typedef IKPerson* (__cdecl *PFNGetIKPerson)(void);
#endif