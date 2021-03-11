import database
import test

var config = test.read_config()
var db = database.connect_ds(config.name, config.user, config.pass)

var stmt = db.prepare("insert into test (id, name) values (?, ?)")
stmt.bind(0, "2")
stmt.bind(1, "john")
stmt.just_execute()
test.show(db.execute("select * from test"))

stmt = db.prepare("update test set name=? where id=?")
stmt.bind(0, "jackson")
stmt.bind(1, "2")
stmt.just_execute()
test.show(db.execute("select * from test"))

stmt = db.prepare("delete from test where id=?")
stmt.bind(0, "2")
stmt.just_execute()
test.show(db.execute("select * from test"))