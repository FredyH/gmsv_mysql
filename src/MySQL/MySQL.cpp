#include "MySQL.h"
#include "Database.h"
#include "Query.h"

#include <winsock.h>
#include <mysql.h>

#include <algorithm>
#include <vector>



namespace MySQL {

	std::vector<Database*> databases;

	void Init() {
		mysql_library_init(0, nullptr, nullptr);
	}

	void Shutdown() {
		for (auto i = databases.begin(); i != databases.end(); ++i) {
			Database *db = *i;

			delete db;
		}

		databases.clear();

		mysql_library_end();
	}

	Database *New() {
		Database *db = new Database();
		databases.push_back(db);

		return db;
	}

	void Remove(Database *db) {
		databases.erase(std::remove(databases.begin(), databases.end(), db), databases.end());
		delete db;
	}

	Query *Poll() {
		for (auto i = databases.begin(); i != databases.end(); ++i) {
			Database *db = *i;

			Query* query = db->Poll(); 

			if (query) {
				return query;
			}
		}

		return nullptr;
	}

}