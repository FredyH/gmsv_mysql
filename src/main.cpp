#include <GarrysMod/Lua/Interface.h>

#include "MySQL/MySQL.h"
#include "MySQL/Database.h"
#include "MySQL/Query.h"

#include "LuaDatabase.h"

using namespace GarrysMod;

int Poll(lua_State *state) {
	while (MySQL::Query *query = MySQL::Poll()) {
		LuaDatabase::QueryUserData *ud = reinterpret_cast<LuaDatabase::QueryUserData *>(query->userdata);

		// TODO: Make this not look shitizzle. Maybe optimize out the repeated MySQL API calls

		if (query->success) {
			if (ud->callback_success != 0) {

				if (ud->multi) {
					LUA->ReferencePush(ud->callback_success);
					LUA->CreateTable();

					int index = 1;

					while (MySQL::Row row = query->FetchRow()) {
						LUA->PushNumber(static_cast<double>(index++));
						LUA->CreateTable();

						for (unsigned int i = 0; i < query->GetFieldCount(); i++) {
							LUA->PushString(query->GetFieldName(i));
							LUA->PushString(row[i], query->GetFieldLength(i));
							LUA->RawSet(-3);
						}

						LUA->RawSet(-3);
					}

					LUA->PushNumber(static_cast<double>(query->insert_id));

					LUA->Call(2, 0);
				} else {
					while (MySQL::Row row = query->FetchRow()) {
						LUA->ReferencePush(ud->callback_success);
						LUA->CreateTable();

						for (unsigned int i = 0; i < query->GetFieldCount(); i++) {
							LUA->PushString(query->GetFieldName(i));
							LUA->PushString(row[i], query->GetFieldLength(i));
							LUA->RawSet(-3);
						}

						LUA->PushNumber(static_cast<double>(query->insert_id));

						LUA->Call(2, 0);
					}
				}
			}
		} else {
			if (ud->callback_error != 0) {
				LUA->ReferencePush(ud->callback_error);
					LUA->PushString(query->error);
					LUA->PushString(query->sql);
				LUA->Call(2, 0);
			} else {
				LUA->PushSpecial(Lua::SPECIAL_GLOB);
					LUA->GetField(-1, "ErrorNoHalt");
						LUA->PushString("MySQL Query Failed\nQuery: ");
						LUA->PushString(query->sql);
						LUA->PushString("\nError: ");
						LUA->PushString(query->error);
						LUA->PushString("\n");
						LUA->PushString(ud->traceback);
						LUA->PushString("\n");
					LUA->Call(7, 0);
				LUA->Pop();
			}
		}

		if (ud->callback_success != 0) {
			LUA->ReferenceFree(ud->callback_success);
		}

		if (ud->callback_error != 0) {
			LUA->ReferenceFree(ud->callback_error);
		}

		delete ud->traceback;
		delete ud;

		query->Free();
	}

	return 0;
}

GMOD_MODULE_OPEN() {
	MySQL::Init();
	LuaDatabase::Register(state);

	LUA->PushSpecial(Lua::SPECIAL_GLOB);
		LUA->CreateTable();

			LUA->PushCFunction(LuaDatabase::New);
			LUA->SetField(-2, "New");

			LUA->PushCFunction(Poll);
			LUA->SetField(-2, "Poll");

		LUA->SetField(-2, "mysql");
	LUA->Pop();

	return 0;
}

GMOD_MODULE_CLOSE() {
	MySQL::Shutdown();	

	return 0;
}