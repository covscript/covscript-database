import mysql_ext
import database

var config = mysql_ext.read_config()
var db = database.connect_ds(config.name, config.user, config.pass)

var lst = mysql_ext.table_list(db)
foreach it in lst do system.out.println(it)
system.out.println("")

lst = mysql_ext.column_info(db, "test")
foreach it in lst
    system.out.print(it.name + "\t")
    system.out.println(it.type.toupper())
end
system.out.println("")

var stmt = db.prepare("insert into test (id, name) values (?, ?)")
stmt.bind(0, to_string(2))
stmt.bind(1, "john")
stmt.just_exec()
mysql_ext.show(mysql_ext.exec(db, "select * from test"))

stmt = db.prepare("update test set name=? where id=?")
stmt.bind(0, "jackson")
stmt.bind(1, "2")
stmt.just_exec()
mysql_ext.show(mysql_ext.exec(db, "select * from test"))

stmt = db.prepare("delete from test where id=?")
stmt.bind(0, "2")
stmt.just_exec()
mysql_ext.show(mysql_ext.exec(db, "select * from test"))