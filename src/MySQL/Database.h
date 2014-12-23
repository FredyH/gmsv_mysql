#pragma once

#include "Query.h"

#include <winsock.h>
#include <mysql.h>

#include <deque>
#include <thread>
#include <mutex>

namespace MySQL {

	const int CONNECTION_COUNT = 2;

	class Database {
		
		MYSQL *connection[CONNECTION_COUNT];

		bool removing = false;
		int remove_count = 0;

		int first_connection = false;
		std::mutex mutex;
		std::deque<Query *>queued_queries;
		std::deque<Query *>completed_queries;

	public:
		Database();
		~Database();

		const char *Connect(const char *server, const char *username, const char *password, const char *database, const int port);
		void Database::Query(const char *sql, unsigned long sql_len, void *userdata);
		unsigned long Escape(char *buf, const char *data, unsigned long len);

		bool IsConnected();

		MySQL::Query *Database::Poll();
		void Worker(MYSQL *connection);
	};

}