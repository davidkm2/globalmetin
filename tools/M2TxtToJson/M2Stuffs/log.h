#pragma once

#include <fstream>
#include <string>
#include <stdarg.h>

static void FileLog(const std::string &filename, int level, const std::string &content)
{
	std::ofstream f(filename, std::ofstream::out | std::ofstream::app);
	f << "> - " << content << "." << std::endl;
	f.close();
}

static void sys_err(const char* fmt, ...)
{
	char context[8192] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf_s(context, fmt, args);
	va_end(args);

	FileLog("syserr.txt", 0, context);
}

static void sys_log(int level, const char* fmt, ...)
{
	char context[8192] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf_s(context, fmt, args);
	va_end(args);

	FileLog("syslog.txt", level, context);
}
