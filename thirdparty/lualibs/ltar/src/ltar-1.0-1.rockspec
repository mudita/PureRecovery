package = "ltar"
version = "1.0-1"
source = {
   url = "..." -- We don't have one yet
}
description = {
    summary = "tar library",
    detailed = [[
        ltar is a module providing tar creating/unpacking functionality
    ]],
    license = "MIT"
}
dependencies = {
    "lua >= 5.1, < 5.5",
    "luafilesystem"
}
build = {
    type = "builtin",
    modules = {
        lmicrotar = { "lmicrotar.c", "microtar.c" },
        ltar = "ltar.lua"
    }
}
