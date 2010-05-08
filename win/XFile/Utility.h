#pragma once

#include <string>
#include <sstream>
#include <d3d9.h>
#include <assert.h>

// Template to convert standard types into strings
template <class T>
std::string ToString(const T & t)
{
	std::ostringstream oss;
	oss.clear();
	oss << t;
	return oss.str();
}

/*
	Some useful functions gathered together into a class that is never instantiated
	I could have used a namespace or simply declared the functions outside of a class
	but I prefer this method using static functions as it gathers them together nicely
*/
class CUtility
{
private:
	~CUtility(void){};
public:
	CUtility(void){};
	
	static bool FailedHr(HRESULT hr);
	static void DebugString(const std::string &str);
	static char* DuplicateCharString(const char* c_str);
	static bool FindFile(std::string *filename);
	static bool DoesFileExist(const std::string &filename);
	static void SplitPath(const std::string& inputPath, std::string* pathOnly, std::string* filenameOnly);
	static std::string GetTheCurrentDirectory();
};
