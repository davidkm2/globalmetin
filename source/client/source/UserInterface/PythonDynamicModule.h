#pragma once
#include <cstdint>
#include <array>

enum EPythonModules : uint8_t
{
	PYTHON_APPLICATION,
	PYTHON_CHARACTER,
	PYTHON_CHARACTER_MGR,
	PYTHON_NETWORK,
	PYTHON_PACK,
	PYTHON_PLAYER,
	PYTHON_MODULE_MAX,
};

class CPythonDynamicModule : public CSingleton <CPythonDynamicModule>
{
public:
	CPythonDynamicModule();
	virtual ~CPythonDynamicModule() = default;

	void Initialize();
	std::string GetModule(uint8_t nModuleID) const;

private:
	std::string GenerateRandomString();
	std::array <std::string, PYTHON_MODULE_MAX> m_dynamicModuleNameArray;
};

