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
	friend class CState;	/* ������ CState ���п��Է��� CContext ��� private �ֶ� */

private:
	bool ChangeState(CState* state);

private:
	CState* m_pState;
};

#endif
