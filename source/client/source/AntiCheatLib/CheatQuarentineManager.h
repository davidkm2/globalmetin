#pragma once
#include "../EterBase/Singleton.h"
#include <memory>
#include <vector>
#include <string>

typedef struct _PROCESS_MD5_SCAN_DATA
{
	char	szProcessName[MAX_PATH];
	char	szProcessMd5[128];
}SProcessMd5ScanData, *PProcessMd5ScanData;

class CCheatQuarentineMgr : public CSingleton <CCheatQuarentineMgr>
{
public:
	CCheatQuarentineMgr();
	virtual ~CCheatQuarentineMgr() = default;
	
	void ProcessQuarentineList(int32_t iIdx, bool bSingle);

	void AppendProcessMd5(const std::string & strProcessName, const std::string & strMd5, bool bBlacklisted);
	void CheckProcessMd5Hashes(bool bSingle);

private:
	std::vector < std::shared_ptr <SProcessMd5ScanData> > m_vProcessMd5List;
	std::vector < std::shared_ptr <SProcessMd5ScanData> > m_vProcessMd5Blacklist;
};

