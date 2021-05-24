#include "StdAfx.h"
#include "../EterBase/Error.h"
#include "../EterLib/Camera.h"
#include "../EterLib/AttributeInstance.h"
#include "../GameLib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"
#include "../CWebBrowser/CWebBrowser.h"

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#include <functional>

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
m_bCursorVisible(TRUE),
m_bLiarCursorOn(false),
m_iCursorMode(CURSOR_MODE_HARDWARE),
m_isWindowed(false),
m_isFrameSkipDisable(false),
m_poMouseHandler(NULL),
m_dwUpdateFPS(0),
m_dwRenderFPS(0),
m_fAveRenderTime(0.0f),
m_dwFaceCount(0),
m_fGlobalTime(0.0f),
m_fGlobalElapsedTime(0.0f),
m_dwLButtonDownTime(0),
m_dwLastIdleTime(0)
{
#ifndef __TEST__
	SetEterExceptionHandler();
#endif

	CTimer::Instance().UseCustomTime();
	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd=0.0f;

	m_dwFaceAccCount=0;
	m_dwFaceAccTime=0;

	m_dwFaceSpdSum=0;
	m_dwFaceSpdCount=0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable=false;
	else
		m_isFrameSkipDisable=true;
}

void CPythonApplication::NotifyHack(const char* c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(UINT eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}
void CPythonApplication::RenderGame()
{
	float fAspect=m_kWndMgr.GetAspect();
	float fFarClip=m_pyBackground.GetFarClip();

	m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);

	CCullingManager::Instance().Process();

	m_kChrMgr.Deform();
	m_kEftMgr.Update();

	m_pyBackground.RenderCharacterShadowToTexture();

	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

	long lx, ly;
	m_kWndMgr.GetMousePosition(lx, ly);
	m_pyGraphic.SetCursorPosition(lx, ly);

	m_pyBackground.RenderSky();

	m_pyBackground.RenderBeforeLensFlare();

	m_pyBackground.RenderCloud();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();

	m_pyBackground.SetCharacterDirLight();
	m_kChrMgr.Render();

	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	m_pyBackground.RenderSnow();
	m_pyBackground.RenderEffect();

	m_pyBackground.EndEnvironment();

	m_kEftMgr.Render();
	m_pyItem.Render();
	m_FlyingManager.Render();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();

	m_pyBackground.RenderAfterLensFlare();

	return;
}

void CPythonApplication::UpdateGame()
{
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

		s.SetPerspective(30.0f,fAspect, 100.0f, fFarClip);
		s.BuildViewFrustum();
	}

	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	m_kChrMgr.Update();
	m_kEftMgr.UpdateSound();
	m_FlyingManager.Update();
	m_pyItem.Update(ptMouse);
	m_pyPlayer.Update();

	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

void CPythonApplication::SkipRenderBuffering(DWORD dwSleepMSec)
{
	m_dwBufSleepSkipTime=ELTimer_GetMSec()+dwSleepMSec;
}

bool CPythonApplication::Process()
{
	ELTimer_SetFrameMSec();

	DWORD dwStart = ELTimer_GetMSec();

	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / std::max<DWORD>(1, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;

	DWORD updatestart = ELTimer_GetMSec();

	m_pyNetworkStream.Process();

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

	UpdateKeyboard();

	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);
	}

	__UpdateCamera();

	CResourceManager::Instance().Update();

	OnCameraUpdate();
	OnMouseUpdate();

	OnUIUpdate();

	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = ((float)dt / (float)uiFrameTime) * uiFrameTime;

		if ( dt >= 500 )
		{
			s_uiNextFrameTime += nAdjustTime;
			printf("FrameSkip Adjusting... %d\n",nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}

		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	if (m_isFrameSkipDisable)
		s_bFrameSkip = false;

	if (!s_bFrameSkip)
	{
		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		DWORD dwRenderStartTime = ELTimer_GetMSec();

		bool canRender = true;

		if (m_isMinimizedWnd)
		{
			canRender = false;
		}
		else
		{
			if (m_pyGraphic.IsLostDevice())
			{
				CPythonBackground& rkBG = CPythonBackground::Instance();
				rkBG.ReleaseCharacterShadowTexture();

				if (m_pyGraphic.RestoreDevice())
					rkBG.CreateCharacterShadowTexture();
				else
					canRender = false;
			}
		}

		if (!IsActive())
		{
			SkipRenderBuffering(3000);
		}

		if (!canRender)
		{
			SkipRenderBuffering(3000);
		}
		else
		{
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin())
			{

				m_pyGraphic.ClearDepthBuffer();

#ifdef __TEST__
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();

				m_pyGraphic.End();

				m_pyGraphic.Show();

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;
				s_dwRenderRangeTime += m_dwCurRenderTime;
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime-s_dwRenderCheckTime>1000)
				{
					m_fAveRenderTime=float(double(s_dwRenderRangeTime)/double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime=ELTimer_GetMSec();
					s_dwRenderRangeTime=0;
					s_dwRenderRangeFrame=0;
				}

				DWORD dwCurFaceCount=m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						DWORD dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{
							if (dwBufRenderTime>58)
								dwBufRenderTime=64;
							else if (dwBufRenderTime>42)
								dwBufRenderTime=48;
							else if (dwBufRenderTime>26)
								dwBufRenderTime=32;
							else if (dwBufRenderTime>10)
								dwBufRenderTime=16;
							else
								dwBufRenderTime=8;
						}

						m_fAveRenderTime=s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd=(m_dwFaceAccCount/m_dwFaceAccTime);

					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio=0.3f;
						s_fAveRenderTime=(s_fAveRenderTime*(100.0f-fRatio)+std::max<float>(16.0f, m_dwCurRenderTime)*fRatio)/100.0f;
						float fFar=25600.0f;
						float fNear=MIN_FOG;
						double dbAvePow=double(1000.0f/s_fAveRenderTime);
						double dbMaxPow=60.0;
						float fDistance=std::max<float>(fNear+(fFar-fNear)*(dbAvePow)/dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	int rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate )
	{
		s_uiLoad -= rest;
		Sleep(rest);
	}

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
		case CGraphicDevice::DEVICESTATE_NULL:
			return DEVICE_STATE_FALSE;

		case CGraphicDevice::DEVICESTATE_BROKEN:
			return DEVICE_STATE_SKIP;

		case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
			if (!m_grpDevice.Reset())
				return DEVICE_STATE_SKIP;
			break;
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_BAD_DRIVER, ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER, "WARN_BAD_DRIVER"));
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_NO_TNL, ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL, "WARN_NO_TNL"));

	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit,frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:

		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 9 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:

		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);

			}
			return true;
		}
		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{
	while (1)
	{
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;

			m_dwLastIdleTime=ELTimer_GetMSec();
		}
	}
}

#ifdef ENABLE_MULTI_LANGUAGE
#include "../eterPack/EterPackManager.h"
bool CPythonApplication::SplitLineByStr(string str, const char * c_szDelimeter, CTokenVector* pstTokenVector)
{
	pstTokenVector->reserve(30);
	pstTokenVector->clear();

	DWORD basePos = 0;

	do
	{
		int beginPos = str.find_first_not_of(c_szDelimeter, basePos);

		if (beginPos < 0)
			return false;

		int endPos;

		if (str[beginPos] == '"')
		{
			++beginPos;
			endPos = str.find("\"", beginPos);

			if (endPos < 0)
				return false;

			basePos = endPos + 1;
		}
		else
		{
			endPos = str.find(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pstTokenVector->push_back(str.substr(beginPos, endPos - beginPos));
		if (int(str.find_first_not_of(c_szDelimeter, basePos)) < 0)
			break;

	} while (basePos < str.length() && basePos > 0);

	return true;
}

bool CPythonApplication::LoadLocaleQuest(const char* c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(kFile, c_szFileName, &pvData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
#endif
		return false;
	
	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	for (int i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!SplitLineByStr(kTextFileLoader.GetLineString(i), "\t", &kTokenVector))
		{
			TraceError("LoadLocaleQuestError: %d (%s)", i, kTokenVector[1].c_str());
			continue;
		}

		if (kTokenVector.size() < 1)
		{
			TraceError("LoadLocaleQuest: invalid line %d (%s).", i, c_szFileName);
			continue;
		}

		int idx = atoi(kTokenVector[0].c_str());
		m_locale_quest.insert(make_pair(idx, kTokenVector[1]));
	}

	return true;
}
string CPythonApplication::FindLocaleQuest(DWORD idx)
{
	string str = "";
	if (m_locale_quest.count(idx) > 0)
		str = m_locale_quest.at(idx);

	return str;
}

string CPythonApplication::Convert_arg(const char * c_szFormat ...)
{
	char buf[256];
	va_list args;

	va_start(args, c_szFormat);
	_vsnprintf(buf, sizeof(buf), c_szFormat, args);
	va_end(args);

	string ret(buf);
	return ret;
}

bool CPythonApplication::LoadLocaleString(const char* c_szFileName)
{
	const VOID* pvData;
	CMappedFile kFile;
#ifdef ENABLE_FOX_FS
	if (!CFileSystem::Instance().Get(kFile, c_szFileName, &pvData, __FUNCTION__))
#else
	if (!CEterPackManager::Instance().Get(kFile, c_szFileName, &pvData))
#endif
		return false;
	
	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	for (int i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		if (!SplitLineByStr(kTextFileLoader.GetLineString(i), "\t", &kTokenVector))
			continue;

		if (kTokenVector.size() < 1)
		{
			TraceError("LoadLocaleString: invalid line %d (%s).", i, c_szFileName);
			continue;
		}

		int idx = atoi(kTokenVector[0].c_str());
		m_locale_string.insert(make_pair(idx, kTokenVector[1]));
	}

	return true;
}

string CPythonApplication::FindLocaleString(DWORD idx)
{
	string str = "";
	if (m_locale_string.count(idx) > 0)
	{
		str = m_locale_string.at(idx);

	}

	return str;
}
#endif
bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer&	rkNPCMgr	= CPythonNonPlayer::Instance();
	CItemManager&		rkItemMgr	= CItemManager::Instance();
	CPythonSkill&		rkSkillMgr	= CPythonSkill::Instance();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	CPythonSkillPet&    rkSkillPetMgr = CPythonSkillPet::Instance();
#endif
#ifdef ENABLE_MULTI_LANGUAGE
	CPythonApplication& rkApp = CPythonApplication::Instance();
#endif
#ifdef ENABLE_MONSTER_CARD
	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
#endif
	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
#ifdef ENABLE_MULTI_LANGUAGE
	char szItemDesc2[256];
	char szItemList2[256];
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	char szSkillPetFileName[256];
#endif
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
#ifdef ENABLE_MONSTER_CARD
	char szMonsterCard[256];
#endif

	snprintf(szItemProto, sizeof(szItemProto), "%s/item_proto", localePath);
	snprintf(szItemDesc, sizeof(szItemDesc), "%s/itemdesc.txt", localePath);
#ifdef ENABLE_MULTI_LANGUAGE
	snprintf(szItemList, sizeof(szItemList) , "locale/tr/item_list.txt");
	snprintf(szItemDesc2, sizeof(szItemDesc2), "%s/itemdesc2.txt", localePath);
	snprintf(szItemList2, sizeof(szItemList2), "locale/tr/item_list2.txt");
#else
	snprintf(szItemList, sizeof(szItemList), "%s/item_list.txt", localePath);
#endif
	snprintf(szMobProto, sizeof(szMobProto), "%s/mob_proto", localePath);
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName), "%s/SkillDesc.txt", localePath);
#ifdef ENABLE_MULTI_LANGUAGE
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName), "locale/tr/SkillTable.txt");
#else
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName), "%s/SkillTable.txt", localePath);
#endif
	snprintf(szInsultList, sizeof(szInsultList), "%s/insult.txt", localePath);
#ifdef ENABLE_GROWTH_PET_SYSTEM
#ifdef ENABLE_MULTI_LANGUAGE
	snprintf(szSkillPetFileName, sizeof(szSkillPetFileName), "locale/tr/pet_skill.txt");
#else
	snprintf(szSkillPetFileName, sizeof(szSkillPetFileName), "%s/pet_skill.txt", localePath);
#endif
#endif
#ifdef ENABLE_MONSTER_CARD
#ifdef ENABLE_MULTI_LANGUAGE
	snprintf(szMonsterCard, sizeof(szMonsterCard), "locale/tr/monster_card.txt");
#else
	snprintf(szMonsterCard, sizeof(szMonsterCard), "%s/monster_card.txt", localePath);
#endif
#endif

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	rkSkillPetMgr.Destroy();
#endif

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);
	}

#ifdef ENABLE_MULTI_LANGUAGE
	if (!rkItemMgr.LoadItemList(szItemList2))
	{
		TraceError("LoadLocaleData - LoadItemList2(%s) Error", szItemList2);
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc2))
	{
		Tracenf("LoadLocaleData - LoadItemDesc2(%s) Error", szItemDesc2);
	}
#endif

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szMobProto);
		return false;
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (!rkSkillPetMgr.RegisterSkillPet(szSkillPetFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillPet(%s) Error", szSkillPetFileName);
		return false;
	}
#endif

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);
	}

	if (LocaleService_IsYMIR())
	{
		char szEmpireTextConvFile[256];
		for (DWORD dwEmpireID=1; dwEmpireID<=3; ++dwEmpireID)
		{
			sprintf(szEmpireTextConvFile, "%s/lang%d.cvt", localePath, dwEmpireID);
			if (!rkNetStream.LoadConvertTable(dwEmpireID, szEmpireTextConvFile))
			{
				TraceError("LoadLocaleData - CPythonNetworkStream::LoadConvertTable(%d, %s) FAILURE", dwEmpireID, szEmpireTextConvFile);
			}
		}
	}

#ifdef ENABLE_ACCE_SYSTEM
	char szItemScale[256];
#ifdef ENABLE_MULTI_LANGUAGE
	snprintf(szItemScale, sizeof(szItemScale), "locale/tr/item_scale.txt");
#else
	snprintf(szItemScale, sizeof(szItemScale), "%s/item_scale.txt", localePath);
#endif
	if (!rkItemMgr.LoadItemScale(szItemScale))
		Tracenf("LoadLocaleData: error while loading %s.", szItemScale);
#endif

#ifdef ENABLE_MULTI_LANGUAGE
	char szLocaleQuest[256];
	snprintf(szLocaleQuest, sizeof(szLocaleQuest), "%s/locale_quest.txt", localePath);
	if (!rkApp.LoadLocaleQuest(szLocaleQuest))
		TraceError("LoadLocaleData: error while loading %s.", szLocaleQuest);
	char szLocaleString[256];
	snprintf(szLocaleString, sizeof(szLocaleString), "%s/locale_string.txt", localePath);
	if (!rkApp.LoadLocaleString(szLocaleString))
		TraceError("LoadLocaleData: error while loading %s.", szLocaleString);
#endif

#ifdef ENABLE_MONSTER_CARD
	if (!rkPlayer.LoadMonsterCard(szMonsterCard))
		TraceError("LoadMonsterCard: error(%s)", szMonsterCard);
#endif

	return true;
}
unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION |   WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject * poSelf, const char * c_szName, int width, int height, int Windowed)
{
	Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	if (FindWindow(NULL, c_szName))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);

#ifdef ENABLE_RENDER_TARGET
	CCameraManager::Instance().AddCamera(CCameraManager::DEFAULT_MODEL_RENDER_CAMERA);
#endif

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon( GetInstance(), MAKEINTRESOURCE( IDI_METIN2 ) ), IDC_CURSOR_NORMAL))
	{

		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
	{
		CPythonIME::Instance().UseDefaultIME();
	}
	if (!m_pySystem.IsWindowed() && (m_pySystem.IsUseDefaultIME() || LocaleService_IsEUROPE()))
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;

			if (bAnotherWindow)
			{
				RECT rc;

				GetClientRect(&rc);

				int windowWidth = rc.right - rc.left;
				int windowHeight = (rc.bottom - rc.top);

				CMSApplication::SetPosition(GetScreenWidth() - windowWidth, GetScreenHeight() - 60 - windowHeight);
			}
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

		if (!CreateCursors())
		{

			TraceError("CMSWindow::Cursors Create Error");
			SET_EXCEPTION("CREATE_CURSOR");
			return false;
		}

		if (!m_pySystem.IsNoSoundCard())
		{

			if (!m_SoundManager.Create())
			{
			}
		}

#ifndef ENABLE_DISABLE_SOFTWARE_TILING
		extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

		if (!m_pySystem.IsAutoTiling())
			GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();
#endif
		if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
			return false;

		GrannyCreateSharedDeformBuffer();

#ifndef ENABLE_DISABLE_SOFTWARE_TILING
		if (m_pySystem.IsAutoTiling())
		{
			if (m_grpDevice.IsFastTNL())
			{
				m_pyBackground.ReserveSoftwareTilingEnable(false);
			}
			else
			{
				m_pyBackground.ReserveSoftwareTilingEnable(true);
			}
		}
		else
		{
			m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());
		}
#endif

		SetVisibleMode(true);

		if (m_isWindowFullScreenEnable) 
		{
			SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
		}

		if (!InitializeKeyboard(GetWindowHandle()))
			return false;

		m_pySystem.GetDisplaySettings();
		if (m_pySystem.IsSoftwareCursor())
			SetCursorMode(CURSOR_MODE_SOFTWARE);
		else
			SetCursorMode(CURSOR_MODE_HARDWARE);
		if (!m_netDevice.Create())
		{

			TraceError("NetDevice::Create failed");
			SET_EXCEPTION("CREATE_NETWORK");
			return false;
		}

		CAnticheatManager::Instance().InitializeAnticheatRoutines(GetInstance(), GetWindowHandle());

		if (!m_grpDevice.IsFastTNL())
			CGrannyLODController::SetMinLODMode(true);

		m_pyItem.Create();
		DefaultFont_Startup();

		CPythonIME::Instance().Create(GetWindowHandle());
		CPythonIME::Instance().SetText("", 0);
		CPythonTextTail::Instance().Initialize();
		m_LightManager.Initialize();

		CGraphicImageInstance::CreateSystem(32);
		STICKYKEYS sStickKeys;
		memset(&sStickKeys, 0, sizeof(sStickKeys));
		sStickKeys.cbSize = sizeof(sStickKeys);
		SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
		m_dwStickyKeysFlag = sStickKeys.dwFlags;
		sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
		CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
		CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
		return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}
void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}
time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

void CPythonApplication::Destroy()
{
	WebBrowser_Destroy();
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();
	DefaultFont_Cleanup();
	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();
	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();
	m_SoundManager.Destroy();
	m_grpDevice.Destroy();
	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}

