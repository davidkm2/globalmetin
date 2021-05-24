#pragma once

class IFlyEventHandler
{
public:
	IFlyEventHandler() {}
	virtual ~IFlyEventHandler() {}
	virtual void OnSetFlyTarget() {}
	virtual void OnShoot(DWORD dwSkillIndex) {}

	virtual void OnNoTarget() {}
	virtual void OnNoArrow() {}
	virtual void OnExplodingOutOfRange() {}
	virtual void OnExplodingAtBackground() {}
	virtual void OnExplodingAtAnotherTarget(DWORD dwSkillIndex, DWORD dwVID) {}
	virtual void OnExplodingAtTarget(DWORD dwSkillIndex) {}
};