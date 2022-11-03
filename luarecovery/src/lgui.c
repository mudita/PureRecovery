// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/// GUI related API
// @module recovery.gui

#include "lgui.h"
#include "lauxlib.h"
#include "common.h"
#include <hal/ED028TC1.h>
#include <hal/display.h>

/***
 Display raw(binary blob) graphic
 @function display_raw_img
 @param width image width
 @param height image height
 @param data raw image data
 */
static int _display_raw_img(lua_State *L) {
    size_t len = 0;
    const int width = luaL_checkinteger(L, 1);
    const int height = luaL_checkinteger(L, 2);
    const char *data = luaL_checklstring(L, 3, &len);

    const EinkStatus_e result = EinkDisplayImage(0, 0, width, height, (uint8_t *) data);
    if (result != EinkOK) {
        luaL_error(L, "EinkDisplayImage failed with code:", result);
    }
    return 1;
}

/***
 Clear the display
 @function clear
 */
static int _clear(lua_State *L) {
    UNUSED(L);
    EinkClearScreen();
    return 1;
}


static int _draw_rectangle(lua_State *L) {
    const int x = luaL_checkinteger(L, 1);
    const int y = luaL_checkinteger(L, 2);
    const int w = luaL_checkinteger(L, 3);
    const int h = luaL_checkinteger(L, 4);
    const bool colour = luaL_checkinteger(L, 5);
    eink_write_rectangle(x, y, w, h, colour);
    return 1;
}

static int _refresh(lua_State *L) {
    UNUSED(L);
    eink_refresh_text(0, 0, 600, 480);
    return 1;
}

static const module_consts_t consts[] = {
        {NULL, 0}
};

static const struct luaL_Reg functions[] = {
        {"display_raw_img", _display_raw_img},
        {"draw_rectangle",  _draw_rectangle},
        {"refresh",         _refresh},
        {"clear",           _clear},
        {NULL, NULL}
};

LUALIB_API int luaopen_lgui(lua_State *L) {
    register_module(L, functions, consts, "gui");
    return 1;
}
