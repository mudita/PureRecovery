// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/// Keyboard related API
// @module recovery.keyboard

#include "lgui.h"
#include "lauxlib.h"
#include "common.h"
#include <hal/keyboard.h>

static const module_consts_t consts[] = {
        {NULL, 0}
};

/***
 Keyboard keys
 @table keys
 @field numeric1
 @field numeric2
 @field numeric3
 @field numeric4
 @field numeric5
 @field numeric6
 @field numeric7
 @field numeric8
 @field numeric9
 @field numeric0
 @field numeric_ast
 @field numeric_pnd
 @field joy_left
 @field joy_right
 @field joy_up
 @field joy_down
 @field joy_enter
 @field fn_left
 @field fn_right
 @field vol_up
 @field vol_down
 @field torch
 @field sswitch_up
 @field sswitch_down
 @field sswitch_mid
*/
static const module_consts_t key_consts[] = {
        {"numeric1",     kbd_key_numeric1},
        {"numeric2",     kbd_key_numeric2},
        {"numeric3",     kbd_key_numeric3},
        {"numeric4",     kbd_key_numeric4},
        {"numeric5",     kbd_key_numeric5},
        {"numeric6",     kbd_key_numeric6},
        {"numeric7",     kbd_key_numeric7},
        {"numeric8",     kbd_key_numeric8},
        {"numeric9",     kbd_key_numeric9},
        {"numeric0",     kbd_key_numeric0},
        {"numeric_ast",  kbd_key_numeric_ast},
        {"numeric_pnd",  kbd_key_numeric_pnd},

        {"joy_left",     kbd_joy_left},
        {"joy_right",    kbd_joy_right},
        {"joy_up",       kbd_joy_up},
        {"joy_down",     kbd_joy_down},
        {"joy_enter",    kbd_joy_enter},

        {"fn_left",      kbd_fn_left},
        {"fn_right",     kbd_fn_right},

        {"vol_up",       kbd_vol_up},
        {"vol_down",     kbd_vol_down},
        {"torch",        kbd_torch},

        {"sswitch_up",   kbd_sswitch_up},
        {"sswitch_down", kbd_sswitch_down},
        {"sswitch_mid",  kbd_sswitch_mid},
        {NULL,           0}
};

/***
 Keyboard events
 @table events
 @field pressed  key pressed
 @field released key released
*/
static const module_consts_t events_consts[] = {
        {"pressed",  kbd_pressed},
        {"released", kbd_released},
        {NULL,       0}
};

/***
 Lock the keyboard
 @function lock
 @param lock true for lock, otherwise unlock
 */
static int _lock(lua_State *L) {
    //kbd_lock()
    return 1;
}

/***
 Poll the keyboard events without blocking
 @function poll
 @return table containing events where key is @{keys} and value is @{events}.
 Returns nil if there are no events.
*/
static int _poll(lua_State *L) {
    kbd_event_t *events = kbd_read_events();
    if (events != NULL) {
        lua_newtable(L);

        while (events->key != key_kbd_none) {
            lua_pushinteger(L, events->event);
            lua_rawseti(L, -2, events->key);
            events++;
        }
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static const struct luaL_Reg functions[] = {
        {"lock", _lock},
        {"poll", _poll},
        {NULL, NULL}
};

LUALIB_API int luaopen_lkeyboard(lua_State *L) {
    register_module(L, functions, consts, "keyboard");
    register_consts(L, key_consts, "keys");
    register_consts(L, events_consts, "events");
    return 1;
}
