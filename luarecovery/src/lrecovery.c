// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/// Re-export of the internal submodules in order to have them gathered in one namespace
// @module recovery

#include <version.h>
#include "lrecovery.h"
#include "lauxlib.h"
#include "common.h"

#include "lgui.h"
#include "lsys.h"
#include "lkeyboard.h"
#include "lbootctrl.h"

static const char *recovery_name = "lrecovery";

typedef int (*submodule_open_t)(lua_State *L);

/***
 Get recovery version
 @function version
 @return version string in 'major.minor.patch' format
 */
static int _version(lua_State *L) {
    lua_pushstring(L, VERSION);
    return 1;
}

/***
 Get recovery git branch name
 @function branch
 @return branch name
 */
static int _branch(lua_State *L) {
    lua_pushstring(L, GIT_BRANCH);
    return 1;
}

/***
 Get recovery revision hash
 @function revision
 @return git revision hash
 */
static int _revision(lua_State *L) {
    lua_pushstring(L, GIT_REV);
    return 1;
}

static const struct luaL_Reg lib[] = {
        {"version",  _version},
        {"branch",   _branch},
        {"revision", _revision},
        {NULL, NULL},
};

static submodule_open_t submodules[] = {
        luaopen_lgui,
        luaopen_lsys,
        luaopen_lkeyboard,
        luaopen_lbootctrl
};

static void register_submodules(lua_State *L) {
    for (uint32_t i = 0; i < ARRAY_SIZE(submodules); i++) {
        submodules[i](L);
        lua_pop(L, 1);
    }
}

LUALIB_API int luaopen_lrecovery(lua_State *L) {
    luaL_newlib(L, lib);
    lua_pushvalue(L, -1);
    lua_setglobal(L, recovery_name);

    register_submodules(L);
    return 1;
}