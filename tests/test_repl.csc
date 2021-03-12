import codec.json as json
import database

function show(r)
    try
        var columns = r.column_count()
        foreach i in range(columns) do system.out.print(r.column_name(i) + "\t")
        system.out.println("")
        while !r.done()
            foreach i in range(columns) do system.out.print("(" + r.get(i) + ")\t")
            system.out.println("")
        end
    catch e
        system.out.println(e.what)
    end
end

function read_config()
    var ifs = iostream.ifstream("./ds_config.json")
    var config = json.from_stream(ifs)
    return json.to_var(config)
end

var config = read_config()
var db = database.connect_ds(config.name, config.user, config.pass)

loop
    system.out.print("> ")
    try
        show(db.exec(system.in.getline()))
    catch e
    end
end