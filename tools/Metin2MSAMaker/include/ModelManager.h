#pragma once
#include <granny2/granny2.11.8.0.h>
#include <filesystem>
namespace fs = std::filesystem;

class CModel
{
protected:
	friend class CModelManager;

	CModel();
	virtual ~CModel();

	bool Load(const fs::path& path);
	void Destroy();

public:
	static bool IsGrannyFile(const fs::path& path);
	static bool	IsGrannyModelFile(const fs::path& path);

public:
	granny_model*		GetModel() const { return m_model; };

private:
	granny_model*			m_model;
	granny_model_instance*	m_modelInstance;

	granny_file*			m_file;
	granny_file_info*		m_fileInfo;
};

class CModelManager
{
protected:
	CModelManager();

public:
	typedef	boost::unordered_map<std::string, CModel*>	TModelCache;	

public:
	virtual ~CModelManager();
	static CModelManager& Instance()
	{
		static CModelManager instance;
		return instance;
	}

	CModel*		RegisterModel(const fs::path& path);
	CModel*		GetModel(const fs::path& path);

	// findDepth : �˻� ���� ����. 1�̸� ���� path�� ���� ���丮������ �˻��Ѵ�.
	CModel*		AutoRegisterAndGetModel(const fs::path& path, int findDepth = 1);
	
	void		Destroy();

private:
	TModelCache		m_modelMap;

};
