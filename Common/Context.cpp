#include "Context.h"
#include "State.h"

CContext::CContext()
{

}

CContext::CContext(CState* state)
{
	this->m_pState = state;
}

CContext::~CContext()
{
	if (m_pState)
	{
		delete m_pState;
		m_pState = nullptr;
	}
}

void CContext::OperationInterface()
{
	if (m_pState)
	{
		m_pState->OperationInterface(this);
	}
}

void CContext::OperationChangeState()
{
	if (m_pState)
	{
		m_pState->OperationChangeState(this);
	}
}

bool CContext::ChangeState(CState* state)
{
	this->m_pState = state;

	return true;
}
