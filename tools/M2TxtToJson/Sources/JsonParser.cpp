#include "Parser.h"
#include "../M2Stuffs/TextFileLoader.h"

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

static bool ParseMobDropInfoFromJson(const std::wstring &content)
{
    std::ofstream f("mob_drop_info_extracted.txt", std::ofstream::out | std::ofstream::app);
    try
    {
        
        boost::property_tree::wptree root;
        std::wistringstream wiss(content);
        boost::property_tree::read_json(wiss, root);

        for (auto context = root.begin(); context != root.end(); ++context)
        {
            auto key = context->first;
            auto value = context->second.get_value<std::wstring>();

            auto grouptree = root.get_child(key);
            auto mob = grouptree.get<std::wstring>(L"Mob");
		    auto type = grouptree.get<std::wstring>(L"Type");

            printf("Group: %ls Mob: %ls Type: %ls\n", key.c_str(), mob.c_str(), type.c_str());
            f << "Group: " << WstringToString(key) << " Mob: " << WstringToString(mob) << " Type: " << WstringToString(type) << std::endl;

            for (auto group = grouptree.begin(); group != grouptree.end(); ++group)
            {
                std::wstring dropkey = group->first; 
                if (group->second.get_value<std::wstring>().size() == 0)
                {
                    auto droptree = grouptree.get_child(dropkey);

                    for (auto dropctx = droptree.begin(); dropctx != droptree.end(); ++dropctx)
                    {
                        auto dropctxkey = WstringToString(dropctx->first);
                        auto dropctxvalue = WstringToString(dropctx->second.get_value<std::wstring>());            

//                        printf("Drop key: %ls Key: %s Value: %s\n", dropkey.c_str(), dropctxkey.c_str(), dropctxvalue.c_str());       
                        f << "Drop key: " << WstringToString(dropkey) << " Key: " << dropctxkey << " Value: " << dropctxvalue << std::endl;
                    }
                }
            }
        }
    }
    catch (std::exception & e)
    {
        printf("Exception handled! Error: %s\n", e.what());
        f.close();
        return false;
    }
    f.close();

    return true;
}

bool NParser::ParseJson(int filetype, const std::string &filename)
{
    printf("ParseJson | File type: %d!\n", filetype);

    auto wfilename = StringToWstring(filename);
	std::wifstream in(wfilename.c_str(), std::ios_base::binary);
	in.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	auto content = std::wstring(std::istreambuf_iterator<wchar_t>(in), std::istreambuf_iterator<wchar_t>());

    auto ret = false;
    switch (filetype)
    {
        case FILE_TYPE_MOBDROPINFO:
        {
            ret = ParseMobDropInfoFromJson(content);
        } break;

        default:
            printf("Unknown file type: %d\n", filetype);
            break;
    }

    in.close();
    return ret;
}

