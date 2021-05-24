#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonApplication.h"
#ifdef ENABLE_GRAPHIC_ON_OFF
#include "PythonCharacterManager.h"
#endif
#define DEFAULT_VALUE_ALWAYS_SHOW_NAME		true

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
	m_poInterfaceHandler = poHandler;
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = NULL;
}

void CPythonSystem::SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight)
{
	m_WindowStatus[iIndex].isVisible	= iVisible;
	m_WindowStatus[iIndex].isMinimized	= iMinimized;
	m_WindowStatus[iIndex].ixPosition	= ix;
	m_WindowStatus[iIndex].iyPosition	= iy;
	m_WindowStatus[iIndex].iHeight		= iHeight;
}

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

	LPDIRECT3D9 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

	lpD3D->GetAdapterIdentifier(0, 0, &d3dAdapterIdentifier);
	lpD3D->GetAdapterDisplayMode(0, &d3ddmDesktop);
	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0, d3ddmDesktop.Format);

	for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
		lpD3D->EnumAdapterModes(0, d3ddmDesktop.Format, iMode, &DisplayMode);
		DWORD bpp = 0;
		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;
		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;
			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{

			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
}

int	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT DWORD *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int	CPythonSystem::GetResolutionIndex(DWORD width, DWORD height, DWORD bit)
{
	DWORD re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int	CPythonSystem::GetFrequencyIndex(int res_index, DWORD frequency)
{
	DWORD re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

DWORD CPythonSystem::GetWidth()
{
	return m_Config.width;
}

DWORD CPythonSystem::GetHeight()
{
	return m_Config.height;
}
DWORD CPythonSystem::GetBPP()
{
	return m_Config.bpp;
}
DWORD CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

int CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

int CPythonSystem::GetShadowLevel()
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::Instance().RefreshShadowLevel();
}

int CPythonSystem::IsSaveID()
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID()
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;

	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	memset(&m_Config, 0, sizeof(m_Config));

	m_Config.width				= 1024;
	m_Config.height				= 768;
	m_Config.bpp				= 32;

#if defined( LOCALE_SERVICE_WE_JAPAN )
	m_Config.bWindowed			= true;
#else
	m_Config.bWindowed			= false;
#endif

	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
	m_Config.music_volume		= 1.0f;
	m_Config.voice_volume		= 5;

	m_Config.bDecompressDDS		= 0;
#ifndef ENABLE_DISABLE_SOFTWARE_TILING
	m_Config.bSoftwareTiling	= 0;
#endif
	m_Config.iShadowLevel		= 3;
	m_Config.bViewChat			= true;
	m_Config.bAlwaysShowName	= DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;
#ifdef WJ_SHOW_MOB_INFO
	m_Config.bShowMobLevel		= true;
	m_Config.bShowMobAIFlag		= true;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
	m_Config.iEffectLevel		= 0;
	m_Config.iPrivateShopLevel	= 0;
	m_Config.iDropItemLevel		= 0;

	m_Config.bPetStatus			= true;
	m_Config.bNpcNameStatus		= true;
#endif
#ifdef ENABLE_FOG_FIX
	m_Config.bFogMode			= true;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	m_Config.bSnowModeOption = false;
	m_Config.bSnowTextureModeOption = false;
	m_Config.bNightModeOption = false;
#endif
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName()
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

#ifdef WJ_SHOW_MOB_INFO
bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobLevel(bool bIsOpen)
{
	m_Config.bShowMobLevel = bIsOpen;
}

bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}

void CPythonSystem::SetShowMobAIFlag(bool bIsOpen)
{
	m_Config.bShowMobAIFlag = bIsOpen;
}
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
int CPythonSystem::GetEffectLevel()
{
	return m_Config.iEffectLevel;
}

void CPythonSystem::SetEffectLevel(unsigned int level)
{
	m_Config.iEffectLevel = MIN(level, 5);
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetEffectLevel(level);
}

int CPythonSystem::GetPrivateShopLevel()
{
	return m_Config.iPrivateShopLevel;
}

void CPythonSystem::SetPrivateShopLevel(unsigned int level)
{
	m_Config.iPrivateShopLevel = MIN(level, 5);
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetPrivateShopLevel(level);
}

int CPythonSystem::GetDropItemLevel()
{
	return m_Config.iDropItemLevel;
}

void CPythonSystem::SetDropItemLevel(unsigned int level)
{
	m_Config.iDropItemLevel = MIN(level, 5);
}

bool CPythonSystem::IsPetStatus()
{
	return m_Config.bPetStatus;
}

void CPythonSystem::SetPetStatusFlag(int iFlag)
{
	m_Config.bPetStatus = iFlag == 0 ? true : false;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetPetStatus(!m_Config.bPetStatus);
}

bool CPythonSystem::IsNpcNameStatus()
{
	return m_Config.bNpcNameStatus;
}

void CPythonSystem::SetNpcNameStatusFlag(int iFlag)
{
	m_Config.bNpcNameStatus = iFlag == 0 ? true : false;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.SetNpcNameStatus(!m_Config.bNpcNameStatus);
}
#endif

#ifdef ENABLE_FOG_FIX
bool CPythonSystem::GetFogMode()
{
	return m_Config.bFogMode;
}

void CPythonSystem::SetFogMode(bool bIsOpen)
{
	m_Config.bFogMode = bIsOpen;
}
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
bool CPythonSystem::GetSnowModeOption()
{
	return m_Config.bSnowModeOption;
}

void CPythonSystem::SetSnowModeOption(bool bIsOpen)
{
	m_Config.bSnowModeOption = bIsOpen;
}

bool CPythonSystem::GetSnowTextureModeOption()
{
	return m_Config.bSnowTextureModeOption;
}

void CPythonSystem::SetSnowTextureModeOption(bool bIsOpen)
{
	m_Config.bSnowTextureModeOption = bIsOpen;
}

bool CPythonSystem::GetNightModeOption()
{
	return m_Config.bNightModeOption;
}

void CPythonSystem::SetNightModeOption(bool bIsOpen)
{
	m_Config.bNightModeOption = bIsOpen;
}
#endif

#ifndef ENABLE_DISABLE_SOFTWARE_TILING
bool CPythonSystem::IsAutoTiling()
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling=1;
	else
		m_Config.bSoftwareTiling=2;
}

bool CPythonSystem::IsSoftwareTiling()
{
	if (m_Config.bSoftwareTiling==1)
		return true;

	return false;
}
#endif

bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

bool CPythonSystem::LoadConfig()
{
	FILE * fp = NULL;

	if (NULL == (fp = fopen("metin2.cfg", "rt")))
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fp))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width		= atoi(value);
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height	= atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp		= atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME")) {
			if(strchr(value, '.') == 0) { 
				m_Config.music_volume = pow(10.0f, (-1.0f + (((float) atoi(value)) / 5.0f)));
				if(atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			} else
				m_Config.music_volume = atof(value);
		} else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = (char) atoi(value);
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "PRE_LOADING_DELAY_TIME"))
			g_iLoadingDelayTime = atoi(value);
		else if (!stricmp(command, "WINDOWED"))
		{
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		}
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
#ifndef ENABLE_DISABLE_SOFTWARE_TILING
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
#endif
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
#ifdef WJ_SHOW_MOB_INFO
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
		else if (!stricmp(command, "EFFECT_LEVEL"))
			m_Config.iEffectLevel = atoi(value);
		else if (!stricmp(command, "PRIVATE_SHOP_LEVEL"))
			m_Config.iPrivateShopLevel = atoi(value);
		else if (!stricmp(command, "DROP_ITEM_LEVEL"))
			m_Config.iDropItemLevel = atoi(value);
		else if (!stricmp(command, "PET_STATUS"))
			m_Config.bPetStatus = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NPC_NAME_STATUS"))
			m_Config.bNpcNameStatus = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_FOG_FIX
		else if (!stricmp(command, "FOG_MODE_ON"))
			m_Config.bFogMode = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		else if (!stricmp(command, "NIGHT_MODE_ON"))
			m_Config.bNightModeOption = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_MODE_ON"))
			m_Config.bSnowModeOption = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_TEXTURE_MODE"))
			m_Config.bSnowTextureModeOption = atoi(value) == 1 ? true : false;
#endif
	}

	if (m_Config.bWindowed)
	{
		unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (m_Config.width >= screen_width)
		{
			m_Config.width = screen_width;
		}
		if (m_Config.height >= screen_height)
		{
			m_Config.height = screen_height;
		}
	}

	m_OldConfig = m_Config;

	fclose(fp);
	return true;
}

bool CPythonSystem::SaveConfig()
{
	FILE *fp;

	if (NULL == (fp = fopen("metin2.cfg", "wt")))
		return false;

	fprintf(fp, "WIDTH						%d\n"
				"HEIGHT						%d\n"
				"BPP						%d\n"
				"FREQUENCY					%d\n"
				"SOFTWARE_CURSOR			%d\n"
				"OBJECT_CULLING				%d\n"
				"VISIBILITY					%d\n"
				"MUSIC_VOLUME				%.3f\n"
				"VOICE_VOLUME				%d\n"
				"GAMMA						%d\n"
				"IS_SAVE_ID					%d\n"
				"SAVE_ID					%s\n"
				"PRE_LOADING_DELAY_TIME		%d\n"
				"DECOMPRESSED_TEXTURE		%d\n"
#ifdef WJ_SHOW_MOB_INFO
				"SHOW_MOBLEVEL		%d\n"
				"SHOW_MOBAIFLAG		%d\n"
#endif
,
				m_Config.width,
				m_Config.height,
				m_Config.bpp,
				m_Config.frequency,
				m_Config.is_software_cursor,
				m_Config.is_object_culling,
				m_Config.iDistance,
				m_Config.music_volume,
				m_Config.voice_volume,
				m_Config.gamma,
				m_Config.isSaveID,
				m_Config.SaveID,
				g_iLoadingDelayTime,
				m_Config.bDecompressDDS
#ifdef WJ_SHOW_MOB_INFO
				,m_Config.bShowMobLevel,
				m_Config.bShowMobAIFlag
#endif
);

	if (m_Config.bWindowed == 1)
		fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);
	if (m_Config.bViewChat == 0)
		fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);
	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fp, "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);
	if (m_Config.bShowDamage == 0)
		fprintf(fp, "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);
	if (m_Config.bShowSalesText == 0)
		fprintf(fp, "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);

#ifdef ENABLE_GRAPHIC_ON_OFF
	fprintf(fp, "EFFECT_LEVEL			%d\n", m_Config.iEffectLevel);
	fprintf(fp, "PRIVATE_SHOP_LEVEL		%d\n", m_Config.iPrivateShopLevel);
	fprintf(fp, "DROP_ITEM_LEVEL		%d\n", m_Config.iDropItemLevel);

	if (m_Config.bPetStatus	== 0)
		fprintf(fp, "PET_STATUS			%d\n", m_Config.bPetStatus);

	if (m_Config.bNpcNameStatus	== 0)
		fprintf(fp, "NPC_NAME_STATUS	%d\n", m_Config.bNpcNameStatus);
#endif
#ifdef ENABLE_FOG_FIX
	fprintf(fp, "FOG_MODE_ON		%d\n", m_Config.bFogMode);
#endif
#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
	fprintf(fp, "NIGHT_MODE_ON		%d\n", m_Config.bNightModeOption);
	fprintf(fp, "SNOW_MODE_ON		%d\n", m_Config.bSnowModeOption);
	fprintf(fp, "SNOW_TEXTURE_MODE		%d\n", m_Config.bSnowTextureModeOption);
#endif
	fprintf(fp, "USE_DEFAULT_IME		%d\n", m_Config.bUseDefaultIME);
#ifndef ENABLE_DISABLE_SOFTWARE_TILING
	fprintf(fp, "SOFTWARE_TILING		%d\n", m_Config.bSoftwareTiling);
#endif
	fprintf(fp, "SHADOW_LEVEL			%d\n", m_Config.iShadowLevel);
	fprintf(fp, "\n");

	fclose(fp);
	return true;
}

bool CPythonSystem::LoadInterfaceStatus()
{
	FILE * File;
	File = fopen("interface.cfg", "rb");

	if (!File)
		return false;

	fread(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
	return true;
}

void CPythonSystem::SaveInterfaceStatus()
{
	if (!m_poInterfaceHandler)
		return;

	PyCallClassMemberFunc(m_poInterfaceHandler, "OnSaveInterfaceStatus", Py_BuildValue("()"));

	FILE * File;

	File = fopen("interface.cfg", "wb");

	if (!File)
	{
		TraceError("Cannot open interface.cfg");
		return;
	}

	fwrite(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
}

bool CPythonSystem::isInterfaceConfig()
{
	return m_isInterfaceConfig;
}

const CPythonSystem::TWindowStatus & CPythonSystem::GetWindowStatusReference(int iIndex)
{
	return m_WindowStatus[iIndex];
}

void CPythonSystem::ApplyConfig() 
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;

		switch (m_Config.gamma)
		{
			case 0: val = 0.4f;	break;
			case 1: val = 0.7f; break;
			case 2: val = 1.0f; break;
			case 3: val = 1.2f; break;
			case 4: val = 1.4f; break;
		}

		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}

	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem()
{
	CSoundManager& rkSndMgr = CSoundManager::Instance();
	rkSndMgr.SetMusicVolume(m_Config.music_volume);
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);
}

void CPythonSystem::Clear()
{
	SetInterfaceHandler(NULL);
}

CPythonSystem::CPythonSystem()
{
	memset(&m_Config, 0, sizeof(TConfig));

	m_poInterfaceHandler = NULL;

	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();

	if (LoadInterfaceStatus())
		m_isInterfaceConfig = true;
	else
		m_isInterfaceConfig = false;
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler==NULL && "CPythonSystem MUST CLEAR!");
}
