#pragma once
#include "libCore.h"

#include <string>

namespace core
{ 
namespace util
{
	extern DLLEXPORT std::string utf8(const std::wstring &wstr);
	extern DLLEXPORT std::wstring ws(const std::string & str);
	extern DLLEXPORT void AddLog(const std::string & str);
	extern DLLEXPORT void AddLog(const std::wstring & str);
}
}