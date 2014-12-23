-- Returns new database object or false followed by an error string
mysql.New( {
	Host = MySQL Host,
	User = MySQL Username,
	Pass = MySQL Password,
	Name = MySQL Database,
	Port = MySQL Port
} )

-- Polls for any completed queries and handles their callbacks
mysql.Poll()


Database:Query(format, arguments, ..., row_callback(row, insert_id), failure(error, sql_stirng))
Database:QueryAll(format, arguments, ..., set_callback(set, insert_id), failure(error, sql_stirng))
-- "%" chars in format will be replaced with arguments following the format, e.g. db:Query("SELECT %, %", "First", 2) will select "First" and 2

Database:RawQuery(sql, row_callback(row, insert_id), failure(error, sql_stirng))
Database:RawQueryAll(sql, row_callback(row, insert_id), failure(error, sql_stirng))

-- Returns a safe representation of a string to place in a query. Not neccesary unless you are using RawQuery*
Database:Escape(data)