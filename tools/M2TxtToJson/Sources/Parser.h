#pragma once

#include <string>
#include <memory>
#include <algorithm>

#include "Types.h"

inline std::wstring StringToWstring(std::string input)
{
    std::wstring output(input.begin(), input.end());
    return output;
}
inline std::string WstringToString(std::wstring input)
{
    std::string output(input.begin(), input.end());
    return output;
}


namespace NParser
{
    bool ParseTxt(int filetype, const std::string &filename, const std::string &outputfilename, bool convert);
    bool ParseJson(int filetype, const std::string &filename);
};
