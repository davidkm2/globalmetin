#pragma once

#include <string>
#include <algorithm>

inline bool str_to_number(bool& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = (strtol(in, nullptr, 10) != 0);
	return true;
}

inline bool str_to_bool(bool& out, const std::string &in) 
{
	out = in.at(0) == '1';
	return true;
}

inline bool str_to_number(int8_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int8_t>(strtol(in, NULL, 10));
	return true;
}

inline bool str_to_number(uint8_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint8_t>(strtoul(in, NULL, 10));
	return true;
}

inline bool str_to_number(int16_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int16_t>(strtol(in, NULL, 10));
	return true;
}

inline bool str_to_number(uint16_t& out, const char *in)
{
	if (0 == in || 0 == in[0])	
		return false;

	out = static_cast<uint16_t>(strtoul(in, NULL, 10));
	return true;
}

inline bool str_to_number(int32_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int32_t>(strtol(in, NULL, 10));
	return true;
}

inline bool str_to_number(uint32_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint32_t>(strtoul(in, NULL, 10));
	return true;
}

inline bool str_to_number(int64_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int64_t>(strtoull(in, NULL, 10));
	return true;
}

inline bool str_to_number(uint64_t& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint64_t>(strtoull(in, NULL, 10));
	return true;
}

inline bool str_to_number(float& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<float>(std::stof(in));
	return true;
}

inline bool str_to_number(double& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<double>(strtod(in, NULL));
	return true;
}

inline bool str_to_number(long double& out, const char *in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<long double>(strtold(in, NULL));
	return true;
}


