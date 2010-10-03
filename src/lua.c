// Lua.c
#include "src/common.h"
#include "src/lua.h"

// Is the luaCallback currently enabled?
int luaCallback_enabled(luaCallback* l) {
	return l->enabled;
}

// Add a callback to the interface
luaCallback* luaInterface_addCallback(luaInterface* i, const char* name) {
	luaCallback* l = &i->callbackArray[i->callbackCount++];
	l->name = name;
	return l;
}

// Create a luaInterface
luaInterface* luaInterface_create() {
	luaInterface* i = malloc(sizeof(luaInterface));
	i->callbackCount = 0;
	memset(&i->callbackArray[0], 0, sizeof(luaCallback) * MAX_CALLBACKS);
	return i;
}

// Find a luaCallback
luaCallback* luaInterface_findCallback(luaInterface* interface, const char* name) {
	for (int i = 0; i < MAX_CALLBACKS; i++) {
		if (strcmp(interface->callbackArray[i].name, name) == 0)
			return &interface->callbackArray[i];
	}
	return NULL;
}

// Register a luaCallback
void luaInterface_registerCallback(luaInterface* i, const char* name, const char* func) {
	luaCallback* callBack = luaInterface_findCallback(i, name);
	callBack->func = func;
	callBack->enabled = true;
}

int LUA_registerCallback(lua_State* l) {
	printf("Register callback!\n");
	return 0;
}

void lua_registerFunction(lua_State* l, lua_CFunction func, const char* name) {
    lua_pushcfunction(l, func);
    lua_setglobal(l, name);
}
