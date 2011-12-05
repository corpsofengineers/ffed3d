#include <math.h>
#include "ffe3d.h"
#include "../ffecfg.h"
#include <time.h>
#include <d3d9.h>

#define LUA_DIR "Scripts/"
scriptSystem* scriptSystem::_self = NULL;

extern int test;
extern float test2;
extern char *test3[256];


char *scriptSystem::nonLuaScr [256]= 
{
	"test/int", "test2/float", "test3/str", "", "", "", "", "", 
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", 
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", ""
};

void *scriptSystem::nonLua [256]= 
{
	&test, &test2, test3, NULL, NULL, NULL, NULL, NULL, 
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

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
		//serching ttx scripts and parse into game engine
	char *path = new char [MAX_PATH];
	CfgStruct ttx;

	for (int i = 0; i < 100; i++)
	{
		sprintf (path, "Models/%i/ttx.ini", i);	
		//MessageBox (0, path, 0, 0);
		DWORD dwAttr = GetFileAttributes (path);
	
		if (dwAttr != 0xFFFFFFFF)
		{
			ShipDef_t *ship_def = GetModel(i)->Shipdef_ptr;
			CfgOpen (&ttx, path);
			CfgFindSection (&ttx, "SHIP");
			CfgGetKeyVal (&ttx, "forwardthrust", (int *)&ship_def->ForwardThrust);
			CfgGetKeyVal (&ttx, "rearthrust", (int *)&ship_def->RearThrust);
			CfgGetKeyVal (&ttx, "lasers", (int *)&ship_def->Lasers);
			CfgGetKeyVal (&ttx, "fuelscoop", (int *)&ship_def->FuelScoop);
			CfgGetKeyVal (&ttx, "mass", (int *)&ship_def->Mass);
			CfgGetKeyVal (&ttx, "capacity", (int *)&ship_def->Capacity);
			CfgGetKeyVal (&ttx, "price", (int *)&ship_def->Price);
			CfgGetKeyVal (&ttx, "scale", (int *)&ship_def->Scale);
			CfgGetKeyVal (&ttx, "description", (int *)&ship_def->Description);
			CfgGetKeyVal (&ttx, "crew", (int *)&ship_def->Crew);
			CfgGetKeyVal (&ttx, "missiles", (int *)&ship_def->Missiles);
			CfgGetKeyVal (&ttx, "drive", (int *)&ship_def->Drive);
			CfgGetKeyVal (&ttx, "integraldrive", (int *)&ship_def->IntegralDrive);
			CfgGetKeyVal (&ttx, "elitebonus", (int *)&ship_def->EliteBonus);
			
			CfgGetKeyVal (&ttx, "frontMount_x", (int *)&ship_def->frontMount_x);
			CfgGetKeyVal (&ttx, "frontMount_y", (int *)&ship_def->frontMount_y);	
			CfgGetKeyVal (&ttx, "frontMount_z", (int *)&ship_def->frontMount_z);

			CfgGetKeyVal (&ttx, "backMount_x", (int *)&ship_def->backMount_x);
			CfgGetKeyVal (&ttx, "backMount_y", (int *)&ship_def->backMount_y);
			CfgGetKeyVal (&ttx, "backMount_z", (int *)&ship_def->backMount_z);

			CfgGetKeyVal (&ttx, "leftMount_x", (int *)&ship_def->leftMount_x);
			CfgGetKeyVal (&ttx, "leftMount_y", (int *)&ship_def->leftMount_y);
			CfgGetKeyVal (&ttx, "leftMount_z", (int *)&ship_def->leftMount_z);

			CfgGetKeyVal (&ttx, "rightMount_x", (int *)&ship_def->rightMount_x);
			CfgGetKeyVal (&ttx, "rightMount_y", (int *)&ship_def->rightMount_y);
			CfgGetKeyVal (&ttx, "rightMount_z", (int *)&ship_def->rightMount_z);

			CfgClose (&ttx);
		}

	}


	luaVM = lua_open();

	consoleText = new char* [256];
	conTextCount = 0;
	//luaL_openlibs (luaVM); Not Need for now

	//Pushind Basic Methods
	lua_pushcfunction (luaVM, scriptSystem::randomize); lua_setglobal (luaVM, "Randomize");
	lua_pushcfunction (luaVM, scriptSystem::setColor); lua_setglobal (luaVM, "ColorRGB");
	lua_pushcfunction (luaVM, scriptSystem::print); lua_setglobal (luaVM, "print");
	lua_pushcfunction (luaVM, scriptSystem::math_sqrt); lua_setglobal (luaVM, "sqrt");
	lua_pushcfunction (luaVM, scriptSystem::logic_and); lua_setglobal (luaVM, "logic_and");
	lua_pushcfunction (luaVM, scriptSystem::logic_shiftright); lua_setglobal (luaVM, "logic_shiftright");
	lua_pushcfunction (luaVM, scriptSystem::logic_shiftleft); lua_setglobal (luaVM, "logic_shiftleft");

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
		char* log = new char[strlen("error: ")+strlen(error)];
		strcpy (log, "error: ");
		strcat (log, error);
		AddToLog (log);
		ret = 1;
	}

	return ret;
}

int scriptSystem::doString (char* string)
{
	int ret = 0;
	
	if (luaL_dostring (luaVM, string))
	{
		char *comIn = new char (strlen (string));
		strcpy (comIn, string);
		strtok (comIn, " ");
		char *argIn = new char (strlen (string));
		strcpy (argIn, string);
		argIn = strpbrk (argIn, " ");

		ret = 1;

		for (int i = 0; i < 256; i++)
		{
			char *comConst = new char (strlen (scriptSystem::nonLuaScr[i]));
			strcpy (comConst, scriptSystem::nonLuaScr[i]);
			strtok (comConst, "/");

			if (!strcmp (comConst, comIn))
			{
				ret = 0;

				char *argConst = new char (strlen (scriptSystem::nonLuaScr[i]));
				strcpy (argConst, scriptSystem::nonLuaScr[i]);
				argConst = strpbrk (argConst, "/");

				if (!strcmp (argConst, "/str"))
				{
					memcpy (scriptSystem::nonLua[i], argIn+1, strlen(argIn)-1);
					break;
				}

				if (!strcmp (argConst, "/int"))
				{
				
					for (int q = 0; q < strlen (argIn); q++)
					{
						if (isalpha (argIn[q]) || argIn[q] == '.')
						{
							char *err = new char;
							sprintf (err, "%s must be a int value", comIn);
							this->AddToLog (err);
							return ret;
						}
					}
					
					int arg = atoi (argIn);
					memcpy (scriptSystem::nonLua[i], &arg, sizeof (int));
				}

				if (!strcmp (argConst, "/float"))
				{
	
					for (int q = 0; q < strlen (argIn); q++)
					{
						if (isalpha (argIn[q]))
						{
							char *err = new char;
							sprintf (err, "%s must be a float value", comIn);
							this->AddToLog (err);
							return ret;
						}
					}
					float arg = atof (argIn);
					memcpy (scriptSystem::nonLua[i], &arg, sizeof (float));
				}

				break;
			}

		}

	}


	if (ret == 1)
	{
		error = lua_tostring (luaVM, -1);
		char* log = new char[strlen("error: ")+strlen(error)];
		strcpy (log, "error: ");
		strcat (log, error);
		AddToLog (log);
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

char* scriptSystem::getLogString (int id)
{
	return consoleText[id];
}

int scriptSystem::getLogCount (void)
{
	return conTextCount;
}

void scriptSystem::AddToLog (char* text)
{
	if (conTextCount < 256)
	{
		consoleText[conTextCount] = new char [strlen(text)];
		strcpy (consoleText[conTextCount], text);
		conTextCount++;
	} else
	{
		for (int i=1;i<256;i--)
		{
			consoleText[i-1] = new char [strlen (consoleText[i])];
			strcpy (consoleText[i-1], consoleText[i]);
		}
		consoleText[255] = new char [strlen(text)];
		strcpy (consoleText[255], text);
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

int scriptSystem::print (lua_State* L)
{
	scriptSystem::getSingleton()->AddToLog ((char* )lua_tostring (L, 1));
	return 0;
}

int scriptSystem::math_sqrt (lua_State *L)
{
	u16 var = lua_tointeger (L, 1);
	u32 res = sqrt ((double)var);
	lua_pushinteger (L, res);
	return 1;
}

int scriptSystem::logic_and (lua_State *L)
{
	int a = lua_tointeger (L, 1);
	int b = lua_tointeger (L, 2);

	a &= b;

	lua_pushinteger (L, a);
	return 1;
}

int scriptSystem::logic_shiftright (lua_State *L)
{
	int var = lua_tointeger (L, 1);
	int shift = lua_tointeger (L, 2);
	var >>= shift;
	lua_pushinteger (L, var);
	return 1;
}

int scriptSystem::logic_shiftleft (lua_State *L)
{
	int var = lua_tointeger (L, 1);
	int shift = lua_tointeger (L, 2);
	var <<= shift;
	lua_pushinteger (L, var);
	return 1;
}

//LUA PUSHING METHODS

void scriptSystem::newInteger (__int64 arg)
{
	lua_pushinteger (luaVM, arg);
}

void scriptSystem::newNumber (double arg)
{
	lua_pushnumber (luaVM, arg);
}

void scriptSystem::newString (char* arg)
{
	lua_pushstring (luaVM, arg);
}

void scriptSystem::newBool (bool arg)
{
	lua_pushboolean (luaVM, arg);
}

void scriptSystem::newFunction (CFunction func)
{
	lua_CFunction luaFunc;
	luaFunc = (lua_CFunction)func;
	lua_pushcfunction (luaVM, luaFunc);
}

void scriptSystem::registerVariable (char* varName)
{
	lua_setglobal (luaVM, varName);
}

void scriptSystem::newClass (void)
{
	lua_newtable (luaVM);
	_top = lua_gettop (luaVM);
}

void scriptSystem::newChildInteger (char* varName, __int64 arg)
{
	lua_pushstring (luaVM, varName);
	lua_pushinteger (luaVM, arg);
	lua_settable (luaVM, _top);
}

void scriptSystem::newChildNumber(char *varName, double arg)
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

void scriptSystem::newChildBool (char* varName, bool arg)
{
	lua_pushstring (luaVM, varName);
	lua_pushboolean (luaVM, arg);
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

__int64 scriptSystem::getAsInteger (int arg_id)
{
	__int64 res = lua_tointeger (luaVM, arg_id);
	return res;
}

double scriptSystem::getAsNumber (int arg_id)
{
	double res = lua_tonumber (luaVM, arg_id);
	return res;
}

int scriptSystem::getFunction (char* funcName)
{
	lua_getglobal (luaVM, funcName);
	
	return lua_isfunction (luaVM, -1);
}

char* scriptSystem::getParentAsString (char* varName)
{
	lua_getfield (luaVM, 1, varName);
	const char* lua_res = lua_tostring (luaVM, -1);
	char* res = new char [strlen (lua_res)];
	strcpy (res, lua_res);
	return res;
}

__int64 scriptSystem::getParentAsInteger (char* varName)
{
	lua_getfield (luaVM, 1, varName);
	__int64 res = lua_tointeger (luaVM, -1);
	return res;
}

double scriptSystem::getParentAsNumber (char* varName)
{
	lua_getfield (luaVM, 1, varName);
	double res = lua_tonumber (luaVM, -1);
	return res;
}

int scriptSystem::getParenFunction (char* funcName)
{
	lua_getfield (luaVM, 1, funcName);

	return lua_isfunction (luaVM, -1);
}


int scriptSystem::callFunction (int argCount, int result)
{
	int ret = 0;
	
	if (lua_pcall (luaVM, argCount, result, 0))
	{
		error = lua_tostring (luaVM, -1);
		char* log = new char[strlen("error: ")+strlen(error)];
		strcpy (log, "error: ");
		strcat (log, error);
		AddToLog (log);
		ret = 1;
	}

	return ret;
}