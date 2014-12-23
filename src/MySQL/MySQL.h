#pragma once

#include "Database.h"
#include "Query.h"

namespace MySQL {
	void Init();
	void Shutdown();

	Database *New();
	void Remove(Database *db);
	
	Query *Poll();
}