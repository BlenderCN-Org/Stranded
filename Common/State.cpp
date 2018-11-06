#include "State.h"
#include "Context.h"

CState::CState()
{

}

CState::~CState()
{
}

bool CState::ChangeState(CContext* con, CState* st)
{
	return con->ChangeState(st);
}

//////////////////////////////////////////////////////////////////////////
CConcreteStateA::CConcreteStateA()
{

}

CConcreteStateA::~CConcreteStateA()
{

}

void CConcreteStateA::OperationInterface(CContext*)
{

}

void CConcreteStateA::OperationChangeState(CContext* con)
{
	OperationInterface(con);

	this->ChangeState(con, new CConcreteStateB());
}

//////////////////////////////////////////////////////////////////////////
CConcreteStateB::CConcreteStateB()
{

}

CConcreteStateB::~CConcreteStateB()
{

}

void CConcreteStateB::OperationInterface(CContext* con)
{

}

void CConcreteStateB::OperationChangeState(CContext* con)
{
	OperationInterface(con);

	this->ChangeState(con, new CConcreteStateA());
}
