#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>

#include <locale>
#include <list>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <boost/assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

#include "../include/ModelManager.h"

#include <granny2/granny2.11.8.0.h>
#pragma comment(lib, "granny2.11.8.0.lib")

typedef std::list<fs::path>	TPathInfoList;

// �̹� ��ó���� �� MSA���Ͽ� �Ʒ� �����͵��� �ִ�. ��� �ִ� �ֵ��� ����� �� �� -_-;
static std::string s_IgnoreKeywords[] = { "ComboInputData", "AttackingData", "MotionEventData", "LoopData", };

bool FileIntoString(const fs::path& path, std::string* outString = NULL)
{
	assert(NULL != outString);

	std::ifstream fs;
	std::string line;

	fs.open(path, std::ios::in);

	// ������ �ִµ� Ȯ�ο� �����ߴٸ� -_-; �����ϵ��� �Ѵ�.
	if (!fs.is_open())
	{
		char errorMsg[255] = {0, };
		strerror_s(errorMsg, sizeof(errorMsg), errno);

		printf("[FAIL] %s\n\t%s \n", path.string().c_str(), errorMsg);

		return true;
	}

	outString->clear();

	fs.clear();

	while(std::getline(fs, line))
		outString->append(line);

	return true;
}

/**
	�̹� MSA������ �ִ��� �˻��ϰ�, �ִٸ� ��ó�� �����Ͱ� �����ϱ� ������ ����� �� �Ǵ� �������� �˻� �� ����
	@return true: �����ؾ� ��, false: ����ᵵ ��
*/
bool IsNeedIgnoreMSA(const fs::path& msaPath)
{
	if (false == fs::is_regular_file(msaPath))
		return false;

	std::string fileContent;
	if (false == FileIntoString(msaPath, &fileContent))
		return false;

	// �̹� �����ϴ� MSA ���Ͽ� Ignore Keyword�� �ϳ��� �ִٸ�, �����ص� ���ٰ� ������.
	for (size_t i = 0; i < _countof(s_IgnoreKeywords); ++i)
		if (fileContent.find(s_IgnoreKeywords[i]) != std::string::npos)
			return true;

	return false;
}

// �Էµ� ���Ͽ��� Accumulation���� ���ؾ� �ϴ����� �����Ѵ�.
bool IsNeedCalcAccumulation(const fs::path& path)
{
	// Accumulation���� ���ؾ� �ϴ� ���ϵ�. �ٸ� ��ǵ��� Accumulation ���� �־ ���� �� �ȴٰ� ��..
	static std::string s_NeedCalcAccumulations[] = {"walk", "run" };

	// ���� �̸��� walk, run ���� ������ Accumulation �ʿ�
	const std::string filename = boost::algorithm::to_lower_copy(path.string());

	for (size_t i = 0; i < _countof(s_NeedCalcAccumulations); ++i)
	{
		if (filename.find(s_NeedCalcAccumulations[i]) != std::string::npos)
			return true;
	}

	return false;
}


enum EResult
{
	EResult_OK,			///< ����
	EResult_Ignore,		///< ���� (�̹� MSA ������ �ְ�, ������ ������ ����̰ų�, ���ϸ��̼� ������ ���� ��� ����)
	EResult_Fail		///< ������ ���ų�, �ִϸ��̼� ������ ���ų� ���...
};


// �Էµ� ������ �м��� MSA������ ����� �Լ�. ���α׷��� �������� main logic
EResult MakeMSA(const fs::path& filePath, std::string* outMsg = 0)
{
	const int axisCount = 3;
	bool bIsAccumulationMotion = true;
	granny_real32 duration = 0.0f;
	granny_matrix_4x4 modelMatrix = {0, };

	assert(0 != outMsg);

	*outMsg = "OK";

	if (false == CModel::IsGrannyFile(filePath))
		return EResult_Fail;

	fs::path basePath = filePath.parent_path();

	fs::path msaPath = basePath / (filePath.stem().string() + ".msa");

	if (IsNeedIgnoreMSA(msaPath))
	{
		*outMsg = "FAIL - Complicated MSA";
		return EResult_Ignore;
	}


	granny_file* grannyFile = GrannyReadEntireFile(filePath.string().c_str());
	granny_file_info* fileInfo = GrannyGetFileInfo(grannyFile);

	if (1 != fileInfo->AnimationCount)
	{
		*outMsg = "IGNORE - NO Animation";

		if (1 < fileInfo->AnimationCount)
			*outMsg = "IGNORE - Too many animations";

		if (0 < fileInfo->ModelCount)
		{
			CModelManager::Instance().RegisterModel(filePath);
			*outMsg = "IGNORE - MODEL FILE(NO Animation)";
		}

		GrannyFreeFile(grannyFile);
		return EResult_Ignore;
	}

	// �׷��� �ִϸ��̼� ����� ��Ư�ؼ�, �ִϸ��̼� Ű�� Ȯ���ϱ� ����� �ִϸ��̼��� ���� ��� �� ���� �Ѵ�.
	// �ִϸ��̼� ���Ͽ� �� ������ �ƿ� ��� ���� ã�Ƽ� �а�, �� �𵨿� �ִϸ��̼��� ������.

	CModel* modelWrapper = CModelManager::Instance().GetModel(filePath);
	if (0 == modelWrapper)
	{
		// ���� ���ٸ� �� �ܰ� ���� �������� ã�ƺ�
		modelWrapper = CModelManager::Instance().GetModel(filePath.parent_path());

		// �׷��� ������ ���� ���� �� ���� �������� �� ������ �˻� �� ����, ������ ����ϵ��� ��.
		if (0 == modelWrapper)
		{
			modelWrapper = CModelManager::Instance().AutoRegisterAndGetModel(filePath, 2);
		}

		// �̷��� ���ٸ�-_- ��
		if (0 == modelWrapper)
		{
			*outMsg = "FAIL - Can't find model file";
			GrannyFreeFile(grannyFile);
			return EResult_Fail;
		}
	}


	// �ִϸ��̼��� �� �� �̻��� ��찡 ������.. -_-; �ϴ��� �Ѱ���� ������. �� ��� ���Ͽ� ���� �ִϸ��̼��� �ִ� ���� ���ٴ� ��.
	for (int i = 0; i < fileInfo->AnimationCount; ++i)
	{
		granny_animation* animation = fileInfo->Animations[i];
		granny_model* model = modelWrapper->GetModel();
		const int boneCount = model->Skeleton->BoneCount;
		duration = animation->Duration;

		// Accumulation���� ���ؾ� �Ѵٸ�, �ִϸ��̼��� ������Ѻ����ν� ���� ���� ���ϵ��� �Ѵ�.
		// Granny�� Motion ���� ������ translate, rotation Ű ���� ���������� �ʾƼ� ��Ȯ�� ���� ���Ϸ��� ���� ��� �� ���� ��. (���ڵ��ϴ� ����� �����ٵ� �ϴ� ������Ѻ��°� ���ϴ� �н�-_-)
		if (IsNeedCalcAccumulation(filePath))
		{
			int trackIndex = -1, bip01Index = -1;

			if (!GrannyFindTrackGroupForModel(animation, model->Name, &trackIndex))
				trackIndex = 0;

			if (1 > animation->TrackGroupCount)
			{
				*outMsg = "FAIL - Invalid Track Group Count";
				GrannyFreeFile(grannyFile);
				return EResult_Fail;
			}
			
			if (!GrannyFindBoneByName(model->Skeleton, "Bip01", &bip01Index))
				bip01Index = 0;

			granny_model_instance* modelInstance = GrannyInstantiateModel(model);

			granny_local_pose* localPose = GrannyNewLocalPose(boneCount);
			granny_world_pose* worldPose = GrannyNewWorldPose(boneCount); 

			//granny_control* control = GrannyPlayControlledAnimation(0.0f, animation, modelInstance);
			granny_controlled_animation_builder *builder = GrannyBeginControlledAnimation(0.0f, animation);
				GrannySetTrackGroupTarget(builder, trackIndex, modelInstance);
				GrannySetTrackGroupAccumulation(builder, trackIndex, GrannyConstantExtractionAccumulation);
			granny_control* control = GrannyEndControlledAnimation(builder);

			// �ִϸ��̼� ���� position
			GrannySetControlClock(control, 0.0f);

			modelMatrix[0][0] = modelMatrix[1][1] = modelMatrix[2][2] = modelMatrix[3][3] = 1.0f;
			GrannyUpdateModelMatrix(modelInstance, 0.0f, (granny_real32*)modelMatrix, (granny_real32*)modelMatrix, false);

			modelMatrix[0][0] = modelMatrix[1][1] = modelMatrix[2][2] = modelMatrix[3][3] = 1.0f;
			GrannyUpdateModelMatrix(modelInstance, animation->Duration - 0.000001f, (granny_real32*)modelMatrix, (granny_real32*)modelMatrix, false);

			// ��ȭ���� �뷫 50������ ���� ���ڸ� ����̶�µ�..? Ȯ���ʿ�
			if (40.0f < fabs(modelMatrix[3][1]))
				bIsAccumulationMotion = true;

			GrannyFreeControlOnceUnused(control);
			GrannyFreeCompletedModelControls(modelInstance);
			GrannyFreeLocalPose(localPose);
			GrannyFreeWorldPose(worldPose);

			GrannyFreeModelInstance(modelInstance);
		}
	}

	GrannyFreeFile(grannyFile);


#if 1
	FILE* fp = 0;
	fopen_s(&fp, msaPath.string().c_str(), "wt");

	if (0 == fp)
	{
		*outMsg = "FAIL - Can't write MSA file";
		return EResult_Fail;
	}


	// MSA ����.. -_-; �� �� �������� �����ؾ� �ϴ� ��..?? 
	fprintf(fp, "ScriptType               MotionData\n");
	fprintf(fp, "\n");

	fprintf(fp, "MotionFileName           \"%s\"\n", filePath.string().c_str());
	fprintf(fp, "MotionDuration           %f\n", duration);

	// Accumulation ���� Y�ุ �����ؾ� �Ѵٰ� ��
	if (bIsAccumulationMotion)
	fprintf(fp, "Accumulation             %.2f\t%.2f\t%.2f\n", 0.0f, modelMatrix[3][1], 0.0f);

	fprintf(fp, "\n");

	fclose(fp);
#endif

	return EResult_OK;
}

int _tmain(int argc, _TCHAR* argv[])
{
	TPathInfoList pathInfoList;

	std::locale::global(std::locale("kor"));
	std::string msg;

	FILE* fpLog = 0;

	// stdout stream redirection to log file
	std::string logPath = std::string(argv[0]) + ".log";
	freopen_s(&fpLog, logPath.c_str(), "a+t", stdout);


	for (int i = 1; i < argc; ++i)
	{
		fs::path inPath  = argv[i];
		
		if (false == fs::exists(inPath))
			continue;

		const bool bIsDirectory = is_directory(inPath);

		if (CModel::IsGrannyFile(inPath))
		{
			if (CModel::IsGrannyModelFile(inPath))
			{
				// GR2 ������ ó���� ���⼭ �ϴ� ������, �� ������ ���Դٴ� �� ��ü�� 
				// ����ڰ� ���� ������ �巡�� Ȥ�� ���� �� ���̹Ƿ� �켱������ �� ���� ���� ����ϵ��� �ϱ� ����.
				CModelManager::Instance().RegisterModel(inPath);
			}
			else
				pathInfoList.push_back(inPath);
		}

		// ����ڰ� ���丮�� ������ ��쿡�� �� �������� ������ recursive Ž���� ��� gr2������ ã�� ó���Ѵ�.
		if (bIsDirectory)
		{
			for (std::filesystem::recursive_directory_iterator end, dir_iter(inPath); dir_iter != end; ++dir_iter)
			{
				const fs::path& curPath = *dir_iter;

				if (CModel::IsGrannyFile(curPath))
					pathInfoList.push_back(curPath);
			}
		}
	}

	// ������ ���� "MSA������ ������ �� GR2 ���ϵ��� ���"�� ������, ������ ������ MSA���ϵ��� ����� �α׸� �����Ѵ�.
	for (TPathInfoList::iterator iter = pathInfoList.begin(); iter != pathInfoList.end(); ++iter)
	{
		const TPathInfoList::value_type& path = *iter;

		EResult resultCode = MakeMSA(path, &msg);

		// full path ������ ���� �� �׷��� ��� ���� -_-;
		fs::path parentPath = path.parent_path();
		std::string shortPath = parentPath.parent_path().filename().string() + 
				"\\" + parentPath.filename().string() + "\\" + path.filename().string();

		tm t;
		time_t timer;
		timer = time(NULL);
		localtime_s(&t, &timer);

		printf("%04d/%02d/%02d %02d:%02d:%02d [%s] %s\n", 
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,			// ���� �ð�
			msg.c_str(), shortPath.c_str());
	}

	CModelManager::Instance().Destroy();

	return 0;
}