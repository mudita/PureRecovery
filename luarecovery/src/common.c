// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "common.h"


void dump_lua_stack(lua_State *L) {
    int top = lua_gettop(L);
    printf("-->\n");
    for (int i = 1; i <= top; i++) {
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                printf("%g\n", lua_tonumber(L, i));
                break;
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
                break;
            case LUA_TNIL:
                printf("%s\n", "nil");
                break;
            default:
                printf("%p\n", lua_topointer(L, i));
                break;
        }
    }
}

void
register_module(lua_State *L, const luaL_Reg *const functions, const module_consts_t *const consts,
                const char *name) {
    luaL_checkversion(L);
    lua_newtable(L);
    luaL_setfuncs(L, functions, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, name);

    int i = 0;
    while (consts[i].name) {
        lua_pushstring(L, consts[i].name);
        lua_pushinteger(L, consts[i].value);
        lua_rawset(L, -3);
        ++i;
    }
}

void register_consts(lua_State *L, const module_consts_t *const consts, const char *name) {
    lua_newtable(L);
    int i = 0;
    while (consts[i].name) {
        lua_pushstring(L, consts[i].name);
        lua_pushinteger(L, consts[i].value);
        lua_rawset(L, -3);
        ++i;
    }
    lua_setfield(L, -2, name);
}
