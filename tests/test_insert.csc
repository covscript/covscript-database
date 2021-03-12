import csdbc_mysql

function show(dat)
    foreach it in dat[0] do system.out.print(it.name+"\t")
    system.out.println("")
    foreach row in dat
        foreach it in row do system.out.print("("+it.data+")\t")
        system.out.println("")
    end
end

var db = csdbc_mysql.connect("./ds_config.json")

var stmt = db.prepare("insert into test (id, name) values (?, ?)")
stmt.bind(0, "2")
stmt.bind(1, "john")
stmt.just_exec()
show(db.exec("select * from test"))

stmt = db.prepare("update test set name=? where id=?")
stmt.bind(0, "jackson")
stmt.bind(1, "2")
stmt.just_exec()
show(db.exec("select * from test"))

stmt = db.prepare("delete from test where id=?")
stmt.bind(0, "2")
stmt.just_exec()
show(db.exec("select * from test"))