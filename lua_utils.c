#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>


// load a file, compile it and run it 
int LUA_LoadFile( lua_State *L, const char *fname )
{
	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0) ) {
		luaL_error(L, "Cannot load file: %s", lua_tostring(L, -1));
		return -1;
	}

	return 0;
}

// load a file, compile it and run it, call the run function in that file
int LUA_Run(lua_State *L, const char *fname )
{
	if (LUA_LoadFile(L, fname)) {
		return -1;
	}
	
	lua_getglobal(L, "run");
	// if not a function
	if (!lua_isfunction(L, -1)) {
		fprintf(stderr, "there is no function 'run'.\n");
		return -1;
	}
	
	// do this function, enter block if error
	if (lua_pcall(L, 0, 1, 0)) {
		luaL_error(L, "Failed when execute function 'run'. Error code: %s", lua_tostring(L, -1));
		return -1;
	}
	
	return LUA_GetNumber(L);
}

// lua_getTS means lua_getTableStringElement :)
char *LUA_GetTS(lua_State *L, char *t, char *element )
{
	// should be statc for the sake of returning
	static char buf[1024];
	
	// get the global table
	lua_getglobal(L, t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return NULL;
	}
	// get the specified element of the table
	lua_getfield(L, -1, (const char *)element);
	if (!lua_isstring(L, -1)) {
		fprintf(stderr, "wrong element paramenter, should be string or number.");
		return NULL;
	}
	
	sprintf(buf, "%s", (char *)lua_tostring(L, -1));
	lua_pop(L, 2);
	
	return (char *)buf;

}

// lua_getTN means lua_getTableNumberElement
int LUA_GetTN(lua_State *L, char *t, char *element )
{
	int n;
	
	// get the global table
	lua_getglobal(L, t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return -1;
	}
	// get the specified element of the table
	lua_getfield(L, -1, (const char *)element);
	if (!lua_isnumber(L, -1)) {
		fprintf(stderr, "wrong element paramenter, should be string or number.");
		return -1;
	}
	
	n = lua_tonumber(L, -1);
	lua_pop(L, 2);
	
	return n;
}

// lua_getTB means lua_getTableBooleanElement
int LUA_GetTB(lua_State *L, char *t, char *element )
{
	int b;
	
	// get the global table
	lua_getglobal(L, t);
	if (!lua_istable(L, -1)) {
		fprintf(stderr, "wrong table parameter: %s.\n", t);
		return -1;
	}
	// get the specified element of the table
	lua_getfield(L, -1, (const char *)element);
	if (!lua_isboolean(L, -1)) {
		fprintf(stderr, "wrong element paramenter, should be string or number.");
		return -1;
	}
	
	b = lua_toboolean(L, -1);
	lua_pop(L, 2);
	
	return b;
}

// get a string from lua stack
char *LUA_GetStr(lua_State *L)
{
	char *str;
	
	if (!lua_isstring(L, -1)) {
		fprintf(stderr, "The parameter is not a string.\n");
		return;
	}
	str = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);
	
	return str;
}

// push a string to lua stack
int LUA_PutStr(lua_State *L, char *str)
{
	lua_pushstring(L, (const char *)str);
	return 0;	
}

// push a char to lua stack
int LUA_PutChar(lua_State *L, char ch)
{
	lua_pushnumber(L, ch);
	return 0;
}

// get a int number from lua stack
int LUA_GetNumber(lua_State *L)
{
	int n;
	
	if (!lua_isnumber(L, -1)) {
		fprintf(stderr, "The parameter is not a number.\n");
		return -1;
	}
	n = lua_tonumber(L, -1);
	lua_pop(L, 1);
	
	return n;

}

// push a int number to lua stack
int LUA_PutNumber(lua_State *L, int n)
{
	lua_pushnumber(L, (lua_Number)n);
	return 0;
}
