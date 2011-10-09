#include "lua/lua.hpp"
#include <windows.h>

typedef int (*CFunction) (void);

class scriptSystem
{
	static scriptSystem* _self;
//METHODS
public:
	static scriptSystem* getSingleton (void);
	void FreeInst (void);

	int doScript (char* scriptName);
	int doString (char* string);
	const char* getLastError (void);

	int doPreLaunchScripts();
	int doAutoScripts();

	//LUA METHODS
	static int randomize (lua_State* L);
	static int setColor (lua_State *L);

	//LUA PUSHING METHODS
	void newInteger (char* varName, int arg);
	void newNumber (char* varName, float arg);
	void newString (char* varName, char* arg);
	void newFunction (char* funcName, CFunction func);

	void newClass (void);
	void newChildInteger (char* varName, int arg);
	void newChildNumber (char* varName, float arg);
	void newChildString (char* varName, char* arg);
	void newChildFunction (char* funcName, CFunction arg);
	void registerClass (char* className);

	//LUA GET`SMETHOD
	char* getAsString (int arg_id);
	float getAsNumber (int arg_id);
	int getAsInteger (int arg_id);

	char* getParentAsString (char* varName);
	float getParentAsNumber (char* varName);
	int getParentAsInteger (char* varName);

protected:
	scriptSystem (void);
	~scriptSystem (void);

	void BuildScriptsTable (void);

//VARIABLES
public:
protected:
	lua_State* luaVM;
	int _top;
	const char* error;

	size_t fs;
	size_t as;
	char** firstscripts;
	char** autoscripts;
};