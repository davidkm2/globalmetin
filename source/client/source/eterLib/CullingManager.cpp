#include "StdAfx.h"
#include "CullingManager.h"
#include "GrpObjectInstance.h"
#ifdef COUNT_SHOWING_SPHERE
int showingcount = 0;
#endif

void CCullingManager::RayTraceCallback(const Vector3d &/*p1*/,          
							  const Vector3d &/*dir*/,          
							  float distance,
							  const Vector3d &/*sect*/,
							  SpherePack *sphere)
{
	if (m_RayFarDistance<=0.0f || m_RayFarDistance>=distance)
	{
#ifdef SPHERELIB_STRICT
		if (sphere->IS_SPHERE)
			puts("CCullingManager::RayTraceCallback");
#endif
		m_list.push_back((CGraphicObjectInstance *)sphere->GetUserData());
	}
}
void CCullingManager::VisibilityCallback(const Frustum &/*f*/,SpherePack *sphere,ViewState state)
{
#ifdef SPHERELIB_STRICT
		if (sphere->IS_SPHERE)
			puts("CCullingManager::VisibilityCallback");
#endif

	CGraphicObjectInstance * pInstance = (CGraphicObjectInstance*)sphere->GetUserData();

	if (state == VS_OUTSIDE)
	{
#ifdef COUNT_SHOWING_SPHERE
		if (pInstance->isShow())
		{
			Tracef("SH : %p  ",sphere->GetUserData());
			showingcount--;
			Tracef("show size : %5d\n",showingcount);
		}

#endif
		pInstance->Hide();
	}
	else
	{
#ifdef COUNT_SHOWING_SPHERE
		if (!pInstance->isShow())
		{
			Tracef("HS : %p  ",sphere->GetUserData());
			showingcount++;
			Tracef("show size : %5d\n",showingcount);
		}
#endif
		pInstance->Show();
	}
}

void CCullingManager::RangeTestCallback(const Vector3d &/*p*/,float /*distance*/,SpherePack *sphere,ViewState state)
{
#ifdef SPHERELIB_STRICT
		if (sphere->IS_SPHERE)
			puts("CCullingManager::RangeTestCallback");
#endif
	if (state!=VS_OUTSIDE)
	{
		m_list.push_back((CGraphicObjectInstance *)sphere->GetUserData());

	}

}

void CCullingManager::Reset()
{
	m_Factory->Reset();
}

void CCullingManager::Update()
{
	m_Factory->Process();

}

void CCullingManager::Process()
{
	UpdateViewMatrix();
	UpdateProjMatrix();
	BuildViewFrustum();
	m_Factory->FrustumTest(GetFrustum(), this);

}

CCullingManager::CullingHandle CCullingManager::Register(CGraphicObjectInstance * obj)
{
	assert(obj);
#ifdef COUNT_SHOWING_SPHERE
	Tracef("CR : %p  ",obj);
	showingcount++;
	Tracef("show size : %5d\n",showingcount);
#endif
	Vector3d center;
	float radius;
	obj->GetBoundingSphere(center,radius);
	return m_Factory->AddSphere_(center,radius,obj, false);
}

void CCullingManager::Unregister(CullingHandle h)
{
#ifdef COUNT_SHOWING_SPHERE
	if (((CGraphicObjectInstance*)h->GetUserData())->isShow())
	{
		Tracef("DE : %p  ",h->GetUserData());
		showingcount--;
		Tracef("show size : %5d\n",showingcount);
	}
#endif
	m_Factory->Remove(h);
}

CCullingManager::CCullingManager()
{
	m_Factory = new SpherePackFactory(
		10000,
		6400,
		1600,
		400
		);
}

CCullingManager::~CCullingManager()
{
	delete m_Factory;
}

void CCullingManager::FindRange(const Vector3d &p, float radius)
{
	m_list.clear();
	m_Factory->RangeTest(p, radius, this);
}

void CCullingManager::FindRay(const Vector3d &p1, const Vector3d &dir)
{
	m_RayFarDistance = -1;
	m_list.clear();
	m_Factory->RayTrace(p1,dir,this);
}

void CCullingManager::FindRayDistance(const Vector3d &p1, const Vector3d &dir, float distance)
{
	m_RayFarDistance = distance;
	m_list.clear();
	m_Factory->RayTrace(p1,dir,this);
}