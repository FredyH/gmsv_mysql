#pragma once

#include <GarrysMod/Lua/Interface.h>

namespace LuaDatabase {
	struct QueryUserData {
		int callback;
		bool multi;
		const char *traceback;
	};

	int New(lua_State *L);
	void Register(lua_State *L);
}