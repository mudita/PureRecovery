package = "lrecovery"
version = "1.0-1"
source = {
   url = "..." -- We don't have one yet
}
description = {
    summary = "Recovery utility API",
    detailed = [[
    ]],
    license = "MIT"
}
dependencies = {
    "lua >= 5.1, < 5.5"
}
build = {
    type = "builtin",
    modules = {
        lrecovery = "lrecovery.lua"
    }
}
