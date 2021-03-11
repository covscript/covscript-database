import database
import test

var config = test.read_config()
var db = database.connect_ds(config.name, config.user, config.pass)

loop
    system.out.print("> ")
    try
        test.show(db.execute(system.in.getline()))
    catch e
        system.out.println(e.what)
    end
end