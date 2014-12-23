#pragma once

#include <winsock.h>
#include <mysql.h>

namespace MySQL {
	typedef char **Row;
	
	class Query {
	private:
		~Query();

		
	public:
		Query(const char *sql, unsigned long sql_len);

		void Free();

		void SetResults(MYSQL_RES *res);
		void SetError(const char *error);

		my_ulonglong GetRowCount();
		Row Query::FetchRow();

		unsigned int GetFieldCount();
		const char *GetFieldName(int i);
		const char **GetFieldNames();
		unsigned long Query::GetFieldLength(int i);

		my_ulonglong GetInsertID();


		const char *sql = nullptr;
		unsigned long sql_len;

		void *userdata;

		bool success;
		const char *error = nullptr;
		MYSQL_RES *res;

		my_ulonglong insert_id = 0;
	};
}