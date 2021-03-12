import csdbc, csdbc_mysql, csdbc_sqlite

# var source = csdbc_sqlite.open("./sql.db")
# var source = csdbc_mysql.connect("./db_config.json")

# var target = csdbc_sqlite.open("./sql.db")
# var target = csdbc_mysql.connect("./db_config.json")

@begin
var type_convert = {
    source.typenames[csdbc.sql_int] : target.typenames_full[csdbc.sql_int],
    source.typenames[csdbc.sql_real] : target.typenames_full[csdbc.sql_real],
    source.typenames[csdbc.sql_text] : target.typenames_full[csdbc.sql_text]
}.to_hash_map()
@end

function create_struct(table)
    var columns = source.column_info(table)
    var sql = "CREATE TABLE " + table + "("
    foreach it in columns do sql += it.name + " " + type_convert[it.type] + ","
    sql.cut(1)
    sql += ")"
    target.just_exec(sql)
end

function copy_data(table)
    var data = source.exec("SELECT * FROM " + table)
    var sql = new string
    foreach row in data
        sql = "INSERT INTO " + table + " VALUES("
        foreach i in range(row.size) do sql += "?,"
        sql.cut(1)
        sql += ")"
        var stmt = target.prepare(sql)
        foreach i in range(row.size) do stmt.bind(i, row[i].data)
        stmt.just_exec()
    end
end

# Add your code here
