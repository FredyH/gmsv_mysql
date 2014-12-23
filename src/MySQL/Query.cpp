#include "Query.h"

#include <winsock.h>
#include <mysql.h>

namespace MySQL {

	Query::Query(const char *sql, unsigned long sql_len) {
		this->sql = _strdup(sql);
		this->sql_len = sql_len;
	}

	Query::~Query() {
		delete sql;

		if (success)
			mysql_free_result(res);
		else
			delete error;
	}

	void Query::Free() {
		delete this; // I argue that this could be for applications that could be using different heaps
	}

	void Query::SetResults(MYSQL_RES *res) {
		this->res = res;
		success = true;
	}

	void Query::SetError(const char *error) {
		this->error = _strdup(error);
		success = false;
	}

	my_ulonglong Query::GetRowCount() {
		return mysql_num_rows(res);
	}

	Row Query::FetchRow() {
		return static_cast<Row>(mysql_fetch_row(res));
	}

	unsigned int Query::GetFieldCount() {
		return mysql_num_fields(res);
	}

	const char *Query::GetFieldName(int i) {
		MYSQL_FIELD *field = mysql_fetch_field_direct(res, i);
		
		return field->name;
	}

	unsigned long Query::GetFieldLength(int i) {
		return mysql_fetch_lengths(res)[i];
	}

	my_ulonglong Query::GetInsertID() {
		return insert_id;
	}

}