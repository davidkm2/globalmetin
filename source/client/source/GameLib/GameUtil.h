#pragma once
bool DetectCollisionDynamicSphereVSDynamicSphere(const CDynamicSphereInstance & c_rSphere1, const CDynamicSphereInstance & c_rSphere2);
bool DetectCollisionDynamicZCylinderVSDynamicZCylinder(const CDynamicSphereInstance & c_rSphere1, const CDynamicSphereInstance & c_rSphere2);
float GetDegreeFromPosition(float x, float y);
float GetDegreeFromPosition2(float sx, float sy, float ex, float ey);

float GetInterpolatedRotation(float begin, float end, float curRate);
bool IsSameDirectionRotation();
bool IsCWRotation(float begin, float end);
bool IsCCWRotation(float begin, float end);

bool IsCWAcuteAngle(float begin, float end);
bool IsCCWAcuteAngle(float begin, float end);
enum EDegree_Direction
{
	DEGREE_DIRECTION_SAME = 0,
	DEGREE_DIRECTION_RIGHT = 1,
	DEGREE_DIRECTION_LEFT = 2,
};
float GetDegreeDifference(float fSource, float fTarget);
int GetRotatingDirection(float fSource, float fTarget);
float CameraRotationToCharacterRotation(float fCameraRotation);
float CharacterRotationToCameraRotation(float fCharacterRotation);
