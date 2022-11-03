// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "lua.h"
#include "lauxlib.h"

#include "lmicrotar.h"
#include "microtar.h"

#include <string.h>
#include <stdbool.h>

#define LMICROTAR_VERSION "1.0.0"
#define LMICROTAR_LIBNAME "lmicrotar"

#if LUA_VERSION_NUM >= 503      /* Lua 5.3+ */

#ifndef luaL_optlong
#define luaL_optlong luaL_optinteger
#endif

#endif

#if LUA_VERSION_NUM >= 502
#define new_lib(L, l) (luaL_newlib(L, l))
#else
#define new_lib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#endif

/* luaL_openlib always used with name == NULL */
#define luaL_openlib(L, name, reg, nup) luaL_setfuncs(L,reg,nup)
/* luaL_register used once, so below expansion is OK for this case */
#define luaL_register(L, name, reg) lua_newtable(L);luaL_setfuncs(L,reg,0)

static const char *microtar_meta = ":microtar";
static const char *microtar_meta_ctx = ":microtar:ctx";

#define SC(s)   { #s, MTAR_ ## s },
static const struct {
    const char *name;
    int value;
} microtar_consts[] = {
        /// Entry types
        SC(TREG)
        SC(TLNK)
        SC(TSYM)
        SC(TCHR)
        SC(TBLK)
        SC(TDIR)
        SC(TFIFO)

        /// Return codes
        SC(ESUCCESS)
        SC(EFAILURE)
        SC(EOPENFAIL)
        SC(EREADFAIL)
        SC(EWRITEFAIL)
        SC(ESEEKFAIL)
        SC(EBADCHKSUM)
        SC(ENULLRECORD)
        SC(ENOTFOUND)

        /* terminator */
        {NULL, 0}
};

typedef struct {
    lua_State *L;
    mtar_t mtar;
    bool initialized;
} mtar_ctx;

static mtar_ctx *new_mtar_ctx(lua_State *L) {
    mtar_ctx *ctx = (mtar_ctx *) lua_newuserdata(L, sizeof(*ctx));
    ctx->L = L;
    memset(&ctx->mtar, 0, sizeof(ctx->mtar));

    luaL_getmetatable(L, microtar_meta);
    lua_setmetatable(L, -2);        /* set metatable */

    return ctx;
}

static int free_mtar_ctx(lua_State *L, mtar_ctx *ctx) {
    int ret = mtar_close(&ctx->mtar);
    memset(&ctx->mtar, 0, sizeof(ctx->mtar));
    return ret;
}

static mtar_ctx *get_mtar_ctx(lua_State *L, int index) {
    mtar_ctx *ctx = (mtar_ctx *) luaL_checkudata(L, index, microtar_meta);
    luaL_argcheck(L, ctx != NULL, 1, "`:microtar' expected");
    return ctx;
}

static mtar_ctx *check_mtar_ctx(lua_State *L, int index) {
    return get_mtar_ctx(L, index);
}

static void path_remove_cwd(char *from) {
    if (strlen(from) >= 2 && strncmp(from, "./", 2) == 0) {
        memcpy(from, from + 2, strlen(from) + 1);
    }
}

static void set_info(lua_State *L) {
    lua_pushliteral(L, "LuaMicroTar library is a thin wrapper over microtar C library");
    lua_setfield(L, -2, "_DESCRIPTION");
    lua_pushliteral(L, "LuaMicroTar " LMICROTAR_VERSION);
    lua_setfield(L, -2, "_VERSION");
}

static int _open(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    const char *mode = luaL_optlstring(L, 2, "r", NULL);
    mtar_ctx *ctx = new_mtar_ctx(L);
    int ret = mtar_open(&ctx->mtar, filename, mode);
    if (ret == MTAR_ESUCCESS) {
        ctx->initialized = true;
        return 1;
    }

    lua_pushnil(L);
    lua_pushinteger(L, ret);
    lua_pushstring(L, mtar_strerror(ret));
    return 3;
}

static int _close(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    if (!ctx->initialized) {
        luaL_argerror(L, 1, "attempt to use closed microtar handle");
        return 1;
    }
    ctx->initialized = false;
    int ret = mtar_finalize(&ctx->mtar);
    if (ret != MTAR_ESUCCESS) {
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        lua_pushstring(L, mtar_strerror(ret));
        return 3;
    }
    lua_pushinteger(L, free_mtar_ctx(L, ctx));
    return 1;
}

static int _write_file_header(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    const char *name = luaL_checkstring(L, 2);
    const int size = luaL_checkinteger(L, 3);
    int result = mtar_write_file_header(&ctx->mtar, name, size);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        return 3;
    }
    lua_pushinteger(L, result);
    return 1;
}

static int _write_dir_header(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int result = mtar_write_dir_header(&ctx->mtar, name);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        return 3;
    }
    lua_pushinteger(L, result);
    return 1;
}

static int _write_data(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    const char *data = luaL_checkstring(L, 2);
    const int size = luaL_checkinteger(L, 3);
    int result = mtar_write_data(&ctx->mtar, data, size);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        return 3;
    }
    lua_pushinteger(L, result);
    return 1;
}

static int _next(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    int result = mtar_next(&ctx->mtar);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        return 3;
    }
    lua_pushinteger(L, result);
    return 1;
}

static int _find(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    const char *name = luaL_checkstring(L, 2);
    mtar_header_t head;
    int result = mtar_find(&ctx->mtar, name, &head);
    if (result == MTAR_ESUCCESS) {
        lua_newtable(L);

        lua_pushliteral(L, "mode");
        lua_pushnumber(L, head.mode);
        lua_settable(L, -3);

        lua_pushliteral(L, "owner");
        lua_pushnumber(L, head.owner);
        lua_settable(L, -3);

        lua_pushliteral(L, "size");
        lua_pushnumber(L, head.size);
        lua_settable(L, -3);

        lua_pushliteral(L, "mtime");
        lua_pushnumber(L, head.mtime);
        lua_settable(L, -3);

        lua_pushliteral(L, "type");
        lua_pushnumber(L, head.type);
        lua_settable(L, -3);

        lua_pushliteral(L, "name");
        lua_pushstring(L, head.name);
        lua_settable(L, -3);

        lua_pushliteral(L, "linkname");
        lua_pushstring(L, head.linkname);
        lua_settable(L, -3);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        return 3;
    }
}

static int _read_header(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    mtar_header_t head;
    int result = mtar_read_header(&ctx->mtar, &head);
    if (result == MTAR_ESUCCESS) {
        lua_newtable(L);

        lua_pushliteral(L, "mode");
        lua_pushnumber(L, head.mode);
        lua_settable(L, -3);

        lua_pushliteral(L, "owner");
        lua_pushnumber(L, head.owner);
        lua_settable(L, -3);

        lua_pushliteral(L, "size");
        lua_pushnumber(L, head.size);
        lua_settable(L, -3);

        lua_pushliteral(L, "mtime");
        lua_pushnumber(L, head.mtime);
        lua_settable(L, -3);

        lua_pushliteral(L, "type");
        lua_pushnumber(L, head.type);
        lua_settable(L, -3);

        lua_pushliteral(L, "name");
        path_remove_cwd(head.name);
        lua_pushstring(L, head.name);
        lua_settable(L, -3);

        lua_pushliteral(L, "linkname");
        lua_pushstring(L, head.linkname);
        lua_settable(L, -3);
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        return 3;
    }
}

static int _read_data(lua_State *L) {
    mtar_ctx *ctx = check_mtar_ctx(L, 1);
    const int size = luaL_checkinteger(L, 2);
    char *data = calloc(1, size);
    if (data == NULL) {
        lua_pushnil(L);
        lua_pushfstring(L, "Allocation failed");
        return 2;
    }
    int result = mtar_read_data(&ctx->mtar, data, size);
    if (result != MTAR_ESUCCESS) {
        lua_pushnil(L);
        lua_pushinteger(L, result);
        lua_pushstring(L, mtar_strerror(result));
        free(data);
        return 3;
    }
    lua_pushlstring(L, data, size);
    free(data);
    return 1;
}

static int _gc(lua_State *L) {
    mtar_ctx *ctx = get_mtar_ctx(L, 1);
    if (ctx->initialized) {
        _close(L);
    }
    return 0;
}

static void create_meta(lua_State *L, const char *name, const luaL_Reg *lib) {
    luaL_newmetatable(L, name);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);               /* push metatable */
    lua_rawset(L, -3);                  /* metatable.__index = metatable */

    /* register metatable functions */
    luaL_openlib(L, NULL, lib, 0);

    /* remove metatable from stack */
    lua_pop(L, 1);
}

static const struct luaL_Reg microtarlib[] = {
        {"close",             _close},
        {"write_file_header", _write_file_header},
        {"write_dir_header",  _write_dir_header},
        {"write_data",        _write_data},
        {"next",              _next},
        {"find",              _find},
        {"read_header",       _read_header},
        {"read_data",         _read_data},
        {"__gc",              _gc},
        {NULL, NULL}
};

static const struct luaL_Reg microtarlibctx[] = {
        {"open", _open},
        {NULL, NULL}
};

LUALIB_API int luaopen_lmicrotar(lua_State *L) {
    create_meta(L, microtar_meta, microtarlib);
    create_meta(L, microtar_meta_ctx, microtarlibctx);

    luaL_getmetatable(L, microtar_meta_ctx);

    luaL_register(L, LMICROTAR_LIBNAME, microtarlibctx);
    set_info(L);

    {
        int i = 0;
        /* add constants to global table */
        while (microtar_consts[i].name) {
            lua_pushstring(L, microtar_consts[i].name);
            lua_pushinteger(L, microtar_consts[i].value);
            lua_rawset(L, -3);
            ++i;
        }
    }
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);

    return 1;
}