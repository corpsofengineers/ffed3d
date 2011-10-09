#include "ffescr.h"
#include <time.h>
#include <d3d9.h>

#define LUA_DIR "Scripts/"
scriptSystem* scriptSystem::_self = NULL;

scriptSystem* scriptSystem::getSingleton (void)
{
	if (!_self)
		_self = new scriptSystem;

	return _self;
}

void scriptSystem::FreeInst (void)
{
	delete this;
}

scriptSystem::scriptSystem (void):
	luaVM(0),
	error(0)
{
	luaVM = lua_open();
	
	//luaL_openlibs (luaVM); Not Need for now

	//Pushind Basic Methods
	lua_pushcfunction (luaVM, scriptSystem::randomize); lua_setglobal (luaVM, "Randomize");
	lua_pushcfunction (luaVM, scriptSystem::setColor); lua_setglobal (luaVM, "ColorRGB");

	//BuildScriptTable and
	BuildScriptsTable();
}

scriptSystem::~scriptSystem (void)
{
	lua_close (luaVM);
}

int scriptSystem::doScript (char* scriptName)
{
	int ret = 0;
	
	char* call = new char [strlen(LUA_DIR)+strlen(scriptName)];
	strcpy (call, LUA_DIR);
	strcat (call, scriptName);

	if (luaL_dofile (luaVM, call))
	{
		error = lua_tostring (luaVM, -1);
		ret = 1;
	}

	return ret;
}

void scriptSystem::BuildScriptsTable (void)
{
	WIN32_FIND_DATA fData;
	char* fPath = new char [strlen(LUA_DIR)+1];
	strcpy (fPath, LUA_DIR);
	strcat (fPath, "*");
	char* fName = NULL;

	HANDLE handle = FindFirstFile (fPath, &fData);

	fs = 0;
	as = 0;

	while (FindNextFile (handle, &fData))
	{
		if (!(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			fName = fData.cFileName;

			if (fName[0] == '#') {fs++;}
			if (fName[0] == '!') {as++;}
		}
	}

	firstscripts = new char* [fs];
	autoscripts = new char* [as];
	int f = 0;
	int a = 0;

	handle = FindFirstFile (fPath, &fData);

	while (FindNextFile (handle, &fData))
	{
		if (!(fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			fName = fData.cFileName;

			if (fName[0] == '#')
			{
				firstscripts[f] = new char [strlen(fName)];
				strcpy (firstscripts[f], fName);
				f++;
			}
			if (fName[0] == '!')
			{
				autoscripts[a] = new char [strlen(fName)];
				strcpy (autoscripts[a], fName);
				a++;
			}
		}
	}

}

int scriptSystem::doPreLaunchScripts (void)
{
	int ret = 0;

	for (int i=0;i<fs;i++)
		if (doScript (firstscripts[i])) {ret = 1;}

	return ret;
}

int scriptSystem::doAutoScripts (void)
{

	int ret = 0;
	for (int i=0; i<as;i++)
		if(doScript (autoscripts[i])) {ret = 1;}

	return ret;
}

int scriptSystem::doString (char* string)
{
	int ret = 0;
	if (luaL_dostring (luaVM, string))
	{
		error = lua_tostring (luaVM, -1);
		ret = 1;
	}

	return ret;
}

const char* scriptSystem::getLastError (void)
{
	return error;
}

//LUA basic Methods
int scriptSystem::randomize (lua_State* L)
{
	int f = lua_tointeger (L, 1);
	int t = lua_tointeger (L, 2);

	srand (time (NULL));
	int res = f + rand()%t;

	lua_pushinteger (L, res);
	return 1;
}

int scriptSystem::setColor (lua_State *L)
{
	int r, g, b;
	r = lua_tointeger (L, 1);
	g = lua_tointeger (L, 2);
	b = lua_tointeger (L, 3);

	int res = D3DCOLOR_XRGB (r, g, b);

	lua_pushinteger (L, res);
	return 1;
}

//LUA PUSHING METHODS

void scriptSystem::newInteger (char *varName, int arg)
{
	lua_pushinteger (luaVM, arg); lua_setglobal (luaVM, varName);
}

void scriptSystem::newNumber (char* varName, float arg)
{
	lua_pushnumber (luaVM, arg); lua_setglobal (luaVM, varName);
}

void scriptSystem::newString (char* varName, char* arg)
{
	lua_pushstring (luaVM, arg); lua_setglobal (luaVM, varName);
}

void scriptSystem::newFunction (char* funcName, CFunction func)
{
	lua_CFunction luaFunc;
	luaFunc = (lua_CFunction)func;
	lua_pushcfunction (luaVM, luaFunc); lua_setglobal (luaVM, funcName);
}

void scriptSystem::newClass (void)
{
	lua_newtable (luaVM);
	_top = lua_gettop (luaVM);
}

void scriptSystem::newChildInteger (char* varName, int arg)
{
	lua_pushstring (luaVM, varName);
	lua_pushinteger (luaVM, arg);
	lua_settable (luaVM, _top);
}

void scriptSystem::newChildNumber(char *varName, float arg)
{
	lua_pushstring (luaVM, varName);
	lua_pushnumber (luaVM, arg);
	lua_settable (luaVM, _top);
}

void scriptSystem::newChildString(char *varName, char *arg)
{
	lua_pushstring (luaVM, varName);
	lua_pushstring (luaVM, arg);
	lua_settable (luaVM, _top);
}

void scriptSystem::newChildFunction (char* funcName, CFunction arg)
{
	lua_CFunction luaFunc;
	luaFunc = (lua_CFunction)arg;
	lua_pushstring (luaVM, funcName);
	lua_pushcfunction (luaVM, luaFunc);
	lua_settable (luaVM, _top);
}

void scriptSystem::registerClass (char* className)
{
	lua_setglobal (luaVM, className);
}

//LUA GET METHOD`S

char* scriptSystem::getAsString (int arg_id)
{
	const char* lua_res = lua_tostring (luaVM, arg_id);
	char* res = new char [strlen (lua_res)];
	strcpy (res, lua_res);
	return res;
}

int scriptSystem::getAsInteger (int arg_id)
{
	int res = lua_tointeger (luaVM, arg_id);
	return res;
}

float scriptSystem::getAsNumber (int arg_id)
{
	float res = lua_tonumber (luaVM, arg_id);
	return res;
}

char* scriptSystem::getParentAsString (char* varName)
{
	lua_getfield (luaVM, 1, varName);
	const char* lua_res = lua_tostring (luaVM, -1);
	char* res = new char [strlen (lua_res)];
	strcpy (res, lua_res);
	return res;
}

int scriptSystem::getParentAsInteger (char* varName)
{
	lua_getfield (luaVM, 1, varName);
	int res = lua_tointeger (luaVM, -1);
	return res;
}

float scriptSystem::getParentAsNumber (char* varName)
{
	lua_getfield (luaVM, 1, varName);
	float res = lua_tonumber (luaVM, -1);
	return res;
}