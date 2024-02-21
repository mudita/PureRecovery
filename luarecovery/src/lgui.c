// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/// GUI related API
// @module recovery.gui

#include "lgui.h"
#include "lauxlib.h"
#include "common.h"
#include <hal/ED028TC1.h>
#include <hal/display.h>
#include <boot/board.h>

/***
 Display raw (binary blob) graphic
 @function display_raw_img
 @param width image width (unused for now)
 @param height image height (unused for now)
 @param data raw image data
 */
static int _display_raw_img(lua_State *L) {
    size_t len = 0;
    const char *data = luaL_checklstring(L, 3, &len);

    const EinkStatus_e result = eink_display_image((uint8_t *) data, len);
    if (result != EinkOK) {
        luaL_error(L, "eink_display_image failed with code: %d", result);
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

/***
 Draw filled rectangle
 @function draw_rectangle
 @param x x coordinate of the beginning point of the rectangle
 @param y y coordinate of the beginning point of the rectangle
 @param width rectangle width
 @param height rectangle height
 @param color rectangle color (black/white)
 */
static int _draw_rectangle(lua_State *L) {
    const EinkFrame_t frame = {
            .x = luaL_checkinteger(L, 1),
            .y = luaL_checkinteger(L, 2),
            .w = luaL_checkinteger(L, 3),
            .h = luaL_checkinteger(L, 4)
    };
    const bool black = luaL_checkinteger(L, 5);
    eink_write_rectangle(&frame, black);
    return 1;
}

/***
 Draw simple rectangle-based progress bar
 @function draw_progress_bar
 @param x x coordinate of the beginning point of progress bar
 @param y y coordinate of the beginning point of progress bar
 @param width progress bar width
 @param base_height height of empty rectangle forming progress bar
 @param progress_height height of full rectangle forming progress bar (must be greater than base_height)
 @param percent value to represent on progress bar
 */
static int _draw_progress_bar(lua_State *L) {
    const bool black = true;
    const uint16_t percent_max = 100;

    const uint16_t base_height = luaL_checkinteger(L, 4);
    const uint16_t progress_height = luaL_checkinteger(L, 5);
    if (base_height >= progress_height) {
        luaL_error(L, "progress height must be greater than base height");
        return 1;
    }

    const EinkFrame_t base_rect = {
            .x = luaL_checkinteger(L, 1),
            .y = luaL_checkinteger(L, 2),
            .w = luaL_checkinteger(L, 3),
            .h = base_height
    };
    eink_write_rectangle(&base_rect, black);

    EinkFrame_t progress_rect;
    progress_rect.x = base_rect.x;
    progress_rect.h = progress_height;

    const uint16_t percent_to_show = MIN(luaL_checkinteger(L, 6), percent_max);
    const uint16_t height_offset = (progress_rect.h - base_rect.h) / 2;
    progress_rect.y = base_rect.y - height_offset; // Center progress rectangle on base rectangle
    progress_rect.w = base_rect.w * percent_to_show / percent_max;
    eink_write_rectangle(&progress_rect, black);

    return 1;
}

/***
 Refresh the screen using deep refresh LUT (GC16)
 @function refresh
 */
static int _refresh(lua_State *L) {
    UNUSED(L);
    const EinkFrame_t frame = {
            .x = 0,
            .y = 0,
            .w = BOARD_EINK_DISPLAY_RES_Y,
            .h = BOARD_EINK_DISPLAY_RES_X
    };
    eink_refresh_text(&frame, REFRESH_DEEP);
    return 1;
}

/***
 Refresh the screen using fast refresh LUT (DU2)
 @function refresh_fast
 */
static int _refresh_fast(lua_State *L) {
    UNUSED(L);
    const EinkFrame_t frame = {
            .x = 0,
            .y = 0,
            .w = BOARD_EINK_DISPLAY_RES_Y,
            .h = BOARD_EINK_DISPLAY_RES_X
    };
    eink_refresh_text(&frame, REFRESH_FAST);
    return 1;
}

static const module_consts_t consts[] = {
        {NULL, 0}
};

static const struct luaL_Reg functions[] = {
        {"display_raw_img",     _display_raw_img},
        {"draw_rectangle",      _draw_rectangle},
        {"draw_progress_bar",   _draw_progress_bar},
        {"refresh",             _refresh},
        {"refresh_fast",        _refresh_fast},
        {"clear",               _clear},
        {NULL, NULL}
};

LUALIB_API int luaopen_lgui(lua_State *L) {
    register_module(L, functions, consts, "gui");
    return 1;
}
