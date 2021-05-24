#include <cassert>
#include <cstdlib>

#include "fsm.h"

CFSM::CFSM()
{
	m_stateInitial.Set(this, &CFSM::BeginStateInitial, &CFSM::StateInitial, &CFSM::EndStateInitial);

	m_pCurrentState = static_cast<CState*>(&m_stateInitial);
	m_pNewState = NULL;
}

void CFSM::Update()
{
	if (m_pNewState)
	{
		m_pCurrentState->ExecuteEndState();
		m_pCurrentState = m_pNewState;
		m_pNewState = 0;

		m_pCurrentState->ExecuteBeginState();
	}

	m_pCurrentState->ExecuteState();
}

bool CFSM::IsState(CState& State) const
{
	return (m_pCurrentState == &State);
}

bool CFSM::GotoState(CState& NewState)
{
	if (IsState(NewState) && m_pNewState == &NewState)
		return true;

	m_pNewState = &NewState;
	return true;
}

