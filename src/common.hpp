#ifndef __COMMON_HPP__
#define __COMMON_HPP__
#ifdef _WIN32
#pragma once
#endif

#pragma warning(disable : 4786)
#pragma warning(disable : 4503)

#undef _WIN32_WINNT
#undef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef _UNICODE
#undef UNICODE

#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <ctime>

#ifdef _WIN32
#define NOMINMAX
#endif

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>
#else
#include <wctype.h>
#include <string.h>
#include <stdint.h>

#define _stricmp strcasecmp
#ifndef _WIN32
#define stricmp strcasecmp
#endif

#define _cdecl
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef uint32_t UINT;
typedef int64_t __int64;

typedef std::map<std::string, std::string> DefinesMap;
typedef std::map<std::wstring, std::wstring> WStringWStringMap;
#endif

#ifdef _USE_WIDE_STRING
typedef std::wstring PopString;
#define _S(x) L##x

#ifndef _WIN32
inline int _wtoi(const wchar_t *str)
{
  return (int)wcstol(str, 0, 10);
}
#endif

#define popstrncmp wcsncmp
#define popstrcmp wcscmp
#ifdef _WIN32
#define popstricmp wcsicmp
#else
#define popstricmp wcscasecmp
#endif
#define popsscanf swscanf
#define popatoi _wtoi
#define popstrcpy wcscpy

#define PopStringToStringFast(x) WStringToString(x)
#define PopStringToWStringFast(x) (x)
#define StringToPopStringFast(x) StringToWString(x)
#define WStringToPopStringFast(x) (x)

#else

typedef std::string PopString;
#define _S(x) x

#define popstrncmp strncmp
#define popstrcmp strcmp
#define popstricmp stricmp
#define popsscanf sscanf
#define popatoi atoi
#define popstrcpy strcpy

#define PopStringToStringFast(x) (x)
#define PopStringToWStringFast(x) StringToWString(x)
#define StringToPopStringFast(x) (x)
#define WStringToPopStringFast(x) WStringToString(x)

#endif

#define LONG_BIGE_TO_NATIVE(l)                                                                                         \
	(((l >> 24) & 0xFF) | ((l >> 8) & 0xFF00) | ((l << 8) & 0xFF0000) | ((l << 24) & 0xFF000000))
#define WORD_BIGE_TO_NATIVE(w) (((w >> 8) & 0xFF) | ((w << 8) & 0xFF00))
#define LONG_LITTLEE_TO_NATIVE(l) (l)
#define WORD_LITTLEE_TO_NATIVE(w) (w)

#define LENGTH(anyarray) (sizeof(anyarray) / sizeof(anyarray[0]))

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef __int64 int64;

typedef std::map<std::string, std::string> DefinesMap;
typedef std::map<std::wstring, std::wstring> WStringWStringMap;
typedef PopString::value_type PopChar;
#define HAS_PopChar

namespace PopLib
{

const ulong POPWORK_RAND_MAX = 0x7FFFFFFF;

extern bool gDebug;

int Rand();
int Rand(int range);
float Rand(float range);
void SRand(ulong theSeed);
extern std::string vformat(const char *fmt, va_list argPtr);
extern std::wstring vformat(const wchar_t *fmt, va_list argPtr);
extern std::string StrFormat(const char *fmt...);
extern std::wstring StrFormat(const wchar_t *fmt...);
bool CheckFor98Mill();
bool CheckForVista();
std::string GetAppDataFolder();
void SetAppDataFolder(const std::string &thePath);
std::string URLEncode(const std::string &theString);
std::string StringToUpper(const std::string &theString);
std::wstring StringToUpper(const std::wstring &theString);
std::string StringToLower(const std::string &theString);
std::wstring StringToLower(const std::wstring &theString);
std::wstring StringToWString(const std::string &theString);
std::string WStringToString(const std::wstring &theString);
PopString StringToPopString(const std::string &theString);
PopString WStringToPopString(const std::wstring &theString);
std::string PopStringToString(const PopString &theString);
std::wstring PopStringToWString(const PopString &theString);
std::string Upper(const std::string &theData);
std::wstring Upper(const std::wstring &theData);
std::string Lower(const std::string &theData);
std::wstring Lower(const std::wstring &theData);
std::string Trim(const std::string &theString);
std::wstring Trim(const std::wstring &theString);
bool StringToInt(const std::string theString, int *theIntVal);
bool StringToDouble(const std::string theString, double *theDoubleVal);
bool StringToInt(const std::wstring theString, int *theIntVal);
bool StringToDouble(const std::wstring theString, double *theDoubleVal);
int StrFindNoCase(const char *theStr, const char *theFind);
bool StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength = 10000000);
PopString CommaSeperate(int theValue);
std::string Evaluate(const std::string &theString, const DefinesMap &theDefinesMap);
std::string XMLDecodeString(const std::string &theString);
std::string XMLEncodeString(const std::string &theString);
std::wstring XMLDecodeString(const std::wstring &theString);
std::wstring XMLEncodeString(const std::wstring &theString);

bool Deltree(const std::string &thePath);
bool FileExists(const std::string &theFileName);
void MkDir(const std::string &theDir);
std::string GetFileName(const std::string &thePath, bool noExtension = false);
std::string GetFileDir(const std::string &thePath, bool withSlash = false);
std::string RemoveTrailingSlash(const std::string &theDirectory);
std::string AddTrailingSlash(const std::string &theDirectory, bool backSlash = false);
time_t GetFileDate(const std::string &theFileName);
std::string GetCurDir();
std::string GetFullPath(const std::string &theRelPath);
std::string GetPathFrom(const std::string &theRelPath, const std::string &theDir);
bool AllowAllAccess(const std::string &theFileName);

inline void inlineUpper(std::string &theData)
{
	// std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

	int aStrLen = (int)theData.length();
	for (int i = 0; i < aStrLen; i++)
	{
		theData[i] = toupper(theData[i]);
	}
}

inline void inlineUpper(std::wstring &theData)
{
	// std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

	int aStrLen = (int)theData.length();
	for (int i = 0; i < aStrLen; i++)
	{
		theData[i] = towupper(theData[i]);
	}
}

inline void inlineLower(std::string &theData)
{
	std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
}

inline void inlineLower(std::wstring &theData)
{
	std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
}

inline void inlineLTrim(std::string &theData, const std::string &theChars = " \t\r\n")
{
	theData.erase(0, theData.find_first_not_of(theChars));
}

inline void inlineLTrim(std::wstring &theData, const std::wstring &theChars = L" \t\r\n")
{
	theData.erase(0, theData.find_first_not_of(theChars));
}

inline void inlineRTrim(std::string &theData, const std::string &theChars = " \t\r\n")
{
	theData.resize(theData.find_last_not_of(theChars) + 1);
}

inline void inlineTrim(std::string &theData, const std::string &theChars = " \t\r\n")
{
	inlineRTrim(theData, theChars);
	inlineLTrim(theData, theChars);
}

struct StringLessNoCase
{
	bool operator()(const std::string &s1, const std::string &s2) const
	{
		return _stricmp(s1.c_str(), s2.c_str()) < 0;
	}
};

} // namespace PopLib

//We don't have debug.hpp in PopPak so we define DBG_ASSERTE here
#define DBG_ASSERTE(exp)  

#endif