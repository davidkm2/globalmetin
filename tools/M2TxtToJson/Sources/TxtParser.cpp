#include "Parser.h"
#include "../M2Stuffs/TextFileLoader.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

/* Sample
  "group_name_1": 
  {
    "mob": 104,
    "type": "kill-drop-limit-thiefgloves",
    "drops": 
    {
      "sword" :
      {
        "vnum": 19,
        "count": 1,
        "rate": 0.2
      },
      "armor" :
      {
        "vnum": 11259,
        "count": 1,
        "rate": 0.85
      }
    }
  }
*/

static bool ParseMobDropInfoFromTxt(std::shared_ptr <CTextFileLoader> loader, const std::string &outputfilename, bool convert)
{
    printf("ParseMobDropInfo | File type: mob_drop_info\n");

    boost::property_tree::wptree root;

	for (uint32_t i = 0; i < loader->GetChildNodeCount(); ++i)
	{
		std::string stName("");
		loader->GetCurrentNodeName(&stName);

        if (strncmp(stName.c_str(), "kr_", 3) == 0)
            continue;

        loader->SetChildNode(i);

		std::string strType("");
		if (!loader->GetTokenString("type", &strType))
		{
			printf("Syntax error: no type (kill|drop), node %s(%d)\n", stName.c_str(), i);
			loader->SetParentNode();
			return false;
		}

        int32_t iMobVnum = 0;
        if (!loader->GetTokenInteger("mob", &iMobVnum))
		{
			printf("Syntax error: no mob vnum, node %s(%d)\n", stName.c_str(), i);
			loader->SetParentNode();
			return false;
		}

        int32_t iKillDrop = -1;
        if (strType == "kill")
		{
			if (!loader->GetTokenInteger("kill_drop", &iKillDrop))
            {
                printf("Syntax error: no kill drop count, node %s(%d)\n", stName.c_str(), i);
                loader->SetParentNode();
                return false;
            }
		}

        int32_t iLevelLimit = 0;
        if (strType == "limit")
		{
			if (!loader->GetTokenInteger("level_limit", &iLevelLimit))
			{
                printf("Syntax error: no level_limit, node %s(%d)\n", stName.c_str(), i);
                loader->SetParentNode();
                return false;
			}
		}

        printf("Node: %d > %s [%s] %d %d %d\n", i, stName.c_str(), strType.c_str(), iMobVnum, iKillDrop, iLevelLimit);

		if (iKillDrop == 0)
		{
			loader->SetParentNode();
			continue;
		}

        if (strType != "kill" && strType != "drop" && strType != "limit" && strType != "thiefgloves")
        {
            printf("Syntax error: invalid type %s (kill|drop), node %s(%d)\n", strType.c_str(), stName.c_str(), i);
			loader->SetParentNode();
			return false;
		}

        boost::property_tree::wptree node;
        if (convert)
        {
			node.put(L"Mob", iMobVnum);
			node.put(L"Type", StringToWstring(strType));
            if (iKillDrop > 0)
                node.put(L"Kill_drop", iKillDrop);
            if (iLevelLimit > 0)
                node.put(L"Level_limit", iLevelLimit);
        }

        boost::property_tree::wptree drops;

        for (int32_t k = 1; k < 256; ++k)
		{
			char buf[4];
			snprintf(buf, sizeof(buf), "%d", k);

            TTokenVector* pTok = nullptr; 
        	if (loader->GetTokenVector(buf, &pTok))
			{
                if (pTok->size() < 3 || pTok->size() > 4)
                {
                    printf("Wrong drop context size: %u node %s(%d)\n", pTok->size(), stName.c_str(), i);
                    loader->SetParentNode();
                    return false;
                }

                printf("tok: %s-%s-%s\n", pTok->at(0).c_str(), pTok->at(1).c_str(), pTok->at(2).c_str());

                if (convert)
                {
                    drops.put(L"item", StringToWstring(pTok->at(0)));
                    drops.put(L"count", StringToWstring(pTok->at(1)));
                    drops.put(L"rate", StringToWstring(pTok->at(2)));
                    if (pTok->size() > 3)
                        drops.put(L"rare", StringToWstring(pTok->at(3)));
                    node.add_child(L"RESERVED_" + std::to_wstring(k), drops);
                }
                continue;
            }
            break;
        }
   
        if (convert)
        {
//         root.add_child(L"GROUP_" + StringToWstring(stName), node);
           root.add_child(L"GROUP" + std::to_wstring(i), node);
        }

        loader->SetParentNode();
    }

    if (convert)
    {
        std::wostringstream wossBuf;
    	boost::property_tree::write_json(wossBuf, root);

        std::wofstream f(outputfilename, std::ofstream::out | std::ofstream::app);
        f << wossBuf.str() << std::endl;
        f.close();
    }

    return true;
}

bool NParser::ParseTxt(int filetype, const std::string &filename, const std::string &outputfilename, bool convert)
{
    printf("ParseTxt | File type: %d!\n", filetype);

    auto loader = std::make_shared<CTextFileLoader>();

	if (!loader->Load(filename.c_str()))
    {
        printf("File: %s can not load!\n", filename.c_str());
        loader->DestroySystem();
		return false;
    }
    printf("File: %s succesfully loaded! Node count: %u\n", filename.c_str(), loader->GetChildNodeCount());

    auto ret = false;
    switch (filetype)
    {
        case FILE_TYPE_MOBDROPINFO:
        {
            ret = ParseMobDropInfoFromTxt(loader, outputfilename, convert);
        } break;

        default:
            printf("Unknown file type: %d\n", filetype);
            break;
    }

    loader->DestroySystem();
    return ret;
}

