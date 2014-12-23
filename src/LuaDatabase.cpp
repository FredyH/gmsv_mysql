#include "LuaDatabase.h"

#include "MySQL/MySQL.h"
#include "MySQL/Database.h"
#include "MySQL/Query.h"

#include <GarrysMod/Lua/Interface.h>

#include <string>
#include <sstream>

using namespace GarrysMod;

namespace LuaDatabase {

	const char *MetaName = "MySQL Database";
	const int MetaID = 0;

	MySQL::Database *GetDatabase(lua_State *state, int stack_pos) {
		Lua::UserData *ud = reinterpret_cast<Lua::UserData *>(LUA->GetUserdata(1));

		if (!ud || ud->type != MetaID || !ud->data) {
			LUA->ArgError(1, "invalid database received");
			return nullptr;
		}

		return reinterpret_cast<MySQL::Database *>(ud->data);
	}

	int New(lua_State *state) {
		LUA->CheckType(1, Lua::Type::TABLE);
		LUA->GetField(1, "Host");
		LUA->GetField(1, "User");
		LUA->GetField(1, "Pass");
		LUA->GetField(1, "Name");
		LUA->GetField(1, "Port");

		const char *server = LUA->GetString(-5);
		const char *username = LUA->GetString(-4);
		const char *password = LUA->GetString(-3);
		const char *database = LUA->GetString(-2);
		int port = 0;
		
		if (LUA->IsType(-1, Lua::Type::NUMBER))
			port = static_cast<int>(LUA->GetNumber());

		LUA->Pop(5);

		MySQL::Database *db = MySQL::New();
		
		if (const char *error = db->Connect(server, username, password, database, port)) {
			LUA->PushBool(false);
			LUA->PushString(error);

			MySQL::Remove(db);

			return 2;
		}

		Lua::UserData *ud = reinterpret_cast<Lua::UserData *>(LUA->NewUserdata(sizeof(Lua::UserData)));
		ud->data = db;
		ud->type = MetaID;

		LUA->CreateMetaTableType(MetaName, MetaID);
		LUA->SetMetaTable(-2);

		return 1;
	}

	int Remove(lua_State *state) {
		MySQL::Database *db = GetDatabase(state, 1);
		Lua::UserData *ud = reinterpret_cast<Lua::UserData *>(LUA->GetUserdata(1));

		ud->data = nullptr; // Invalidate userdata

		// Don't handle polling here no matter how much you want to, no need to cause any infinite loops.

		MySQL::Remove(db);

		return 0;
	}

	int Escape(lua_State *state) {
		MySQL::Database *db = GetDatabase(state, 1);

		unsigned int len = 0;
		const char *data = LUA->GetString(2, &len);
		char *buf = new char[len * 2 + 1]; // "In the worst case, each character may need to be encoded as using two bytes, and you need room for the terminating null byte."

		db->Escape(buf, data, len);
		LUA->PushString(buf);

		delete[] buf;

		return 1;
	}

	QueryUserData *CreateQuery(lua_State *state) {
		static int debug_traceback = 0;

		if (!debug_traceback) {
			LUA->PushSpecial(Lua::SPECIAL_GLOB);
				LUA->GetField(-1, "debug");
				LUA->GetField(-1, "traceback");
				debug_traceback = LUA->ReferenceCreate();
			LUA->Pop();
		}

		MySQL::Database *db = GetDatabase(state, 1);

		LUA->CheckString(2);

		unsigned int len = 0;
		const char *pattern = LUA->GetString(2, &len);

		int stack_offset = 0;

		std::stringstream sql;

		for (const char *p = pattern; *p; p++) {

			if (*p == '%') {
				++p;
				stack_offset++;

				switch (*p) {
					case 's': {
								  LUA->CheckType(stack_offset + 2, Lua::Type::STRING);

								  unsigned int len = 0;
								  const char *data = LUA->GetString(stack_offset + 2, &len);
								  char *buf = new char[len * 2 + 1];

								  db->Escape(buf, data, len);

								  sql << "'" << buf << "'";

								  delete[] buf;

								  break;
					}
					case 'd': {
								  LUA->CheckType(stack_offset + 2, Lua::Type::NUMBER);
								  double number = LUA->GetNumber(stack_offset + 2);

								  sql << number;

								  break;
					}
					case 'b': {
								  LUA->CheckType(stack_offset + 2, Lua::Type::BOOL);

								  if (LUA->GetBool(stack_offset + 2))
									  sql << "1";
								  else
									  sql << "0";

								  break;
					}
				}

				continue;
			}

			sql << *p;		
		}

		int callback = 0;

		if (LUA->IsType(stack_offset + 3, Lua::Type::FUNCTION)) {
			LUA->Push(stack_offset + 3);
			callback = LUA->ReferenceCreate();
		}

		QueryUserData *query = new QueryUserData;
		query->callback = callback;

		LUA->ReferencePush(debug_traceback);
			LUA->PushString("");
			LUA->PushNumber(2);
		LUA->Call(2, 1);

		query->traceback = _strdup(LUA->GetString());

		LUA->Pop();

		db->Query(sql.str().c_str(), sql.str().length(), query);

		return query;
	}

	int Query(lua_State *state) {
		QueryUserData *query = CreateQuery(state);
		query->multi = false;

		return 0;
	}

	int QueryAll(lua_State *state) {
		QueryUserData *query = CreateQuery(state);
		query->multi = true;

		return 0;
	}

	void Register(lua_State *state) {
		LUA->CreateMetaTableType(MetaName, MetaID);

			LUA->CreateTable();

				LUA->PushCFunction(Query);
				LUA->SetField(-2, "Query");

				LUA->PushCFunction(QueryAll);
				LUA->SetField(-2, "QueryAll");

				LUA->PushCFunction(Escape);
				LUA->SetField(-2, "Escape");

			LUA->SetField(-2, "__index");

			LUA->PushCFunction(Query);
			LUA->SetField(-2, "__call");

			LUA->PushCFunction(Remove);
			LUA->SetField(-2, "__gc");

			LUA->PushString(MetaName);
			LUA->SetField(-2, "MetaName");

		LUA->Pop();
	}
}