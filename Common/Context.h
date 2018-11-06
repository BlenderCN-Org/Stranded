#ifndef __CONTEXT_H__
#define __CONTEXT_H__

class CState;

class CContext
{
public:
	CContext();
	CContext(CState* state);
	~CContext();

	void OperationInterface();
	void OperationChangeState();

private:
	friend class CState;	/* 声明在 CState 类中可以访问 CContext 类的 private 字段 */

private:
	bool ChangeState(CState* state);

private:
	CState* m_pState;
};

#endif
