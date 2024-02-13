// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include <lauxlib.h>

#define UNUSED(expr) do { (void)(expr); } while (0)
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))

#define MIN(a,b) ((a) < (b) ? (a) : (b))

typedef struct {
    const char *name;
    int value;
} module_consts_t;

void register_module(lua_State *L, const luaL_Reg *const fns, const module_consts_t *const consts, const char *name);

void register_consts(lua_State *L, const module_consts_t *const consts, const char *name);

void dump_lua_stack(lua_State *L);
