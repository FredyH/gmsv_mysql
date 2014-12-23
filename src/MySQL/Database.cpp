#include "Database.h"
#include "Query.h"

#include <winsock.h>
#include <mysql.h>

#include <thread>

namespace MySQL {

	Database::Database() {
		mutex.lock();
			for (int i = 0; i < CONNECTION_COUNT; i++) {
				connection[i] = mysql_init(nullptr);
			
				my_bool reconnect = 1;
				mysql_options(connection[i], MYSQL_OPT_RECONNECT, &reconnect);

				std::thread(&Database::Worker, this, connection[i]).detach();
			}
		mutex.unlock();
	}

	Database::~Database() {
		int count = -1;

		while (count != 0) { // Wait for any queued queries to begin being processed
			mutex.lock();
				count = queued_queries.size();
			mutex.unlock();
		}

		mutex.lock();
			removing = true;
			remove_count = CONNECTION_COUNT - 1;
		mutex.unlock();

		int l_remove_count = -1;

		while (l_remove_count != 0) { // Wait for workers to finish up
			mutex.lock();
				l_remove_count = remove_count;
			mutex.unlock();
		}

		while (MySQL::Query *query = Poll())
			query->Free(); // Queries not cleared by this point won't be handled by MySQL::Poll

		for (int i = 0; i < CONNECTION_COUNT; i++)
			mysql_close(connection[i]);
	}

	const char *Database::Connect(const char *server, const char *username, const char *password, const char *database, const int port) {
		for (int i = 0; i < CONNECTION_COUNT; i++) {
			if (!mysql_real_connect(connection[i], server, username, password, database, port, nullptr, CLIENT_REMEMBER_OPTIONS)) {
				mutex.unlock();
				return mysql_error(connection[i]);
			}

			if (mysql_set_character_set(connection[i], "utf8")) {
				mutex.unlock();
				return mysql_error(connection[i]);
			}
		}

		mutex.lock();
			first_connection = true;
		mutex.unlock();

		return nullptr;
	}

	void Database::Query(const char *sql, unsigned long sql_len, void *userdata) {
		MySQL::Query *query = new MySQL::Query(sql, sql_len);
		query->userdata = userdata;
		
		mutex.lock();
			queued_queries.push_back(query);
		mutex.unlock();
	}

	unsigned long Database::Escape(char *buf, const char *data, unsigned long len) {
		return mysql_real_escape_string(connection[0], buf, data, len);
	}

	MySQL::Query *Database::Poll() {
		MySQL::Query *query = nullptr;

		mutex.lock();
			int count = completed_queries.size();

			if (count > 0) {
				query = completed_queries.front();
				completed_queries.pop_front();
			}
		mutex.unlock();

		return query;
	}

	void Database::Worker(MYSQL *connection) {
		while (true) {
			int count = 0;

			mutex.lock();

				if (removing) {
					remove_count--;
					mutex.unlock();
					return;
				}

				if (!first_connection) {
					mutex.unlock();
					continue;
				}

				count = queued_queries.size();

				if (count > 0) {
					MySQL::Query *query = queued_queries.front();
					queued_queries.pop_front();

					mutex.unlock();

						int error = mysql_real_query(connection, query->sql, query->sql_len);

						if (error) {
							query->SetError(mysql_error(connection));
						} else {
							MYSQL_RES *res = mysql_store_result(connection);

							if (res) { // Successful query with data?
								query->SetResults(res);
								query->insert_id = mysql_insert_id(connection);
							} else if (*mysql_error(connection)) { // Error on storing result
								query->SetError(mysql_error(connection));
							} else { // Empty result set
								query->SetResults(nullptr);
								query->insert_id = 0;
							}
						}

					mutex.lock();

					count = queued_queries.size();
					completed_queries.push_back(query);
				}

			mutex.unlock();

			if (count == 0) // At this point, there may actually be queued queries. Oh well.
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
	}

}