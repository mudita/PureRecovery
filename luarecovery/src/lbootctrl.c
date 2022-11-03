// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/// Boot control API
// @module recovery.bootctrl

#include "lbootctrl.h"
#include "common.h"
#include <hal/boot_control.h>

/***
 System slot
 @table slot
 @field a slot a
 @field b slot b
 */
static const module_consts_t system_slot[] = {
        {"a",  Slot_A},
        {"b",  Slot_B},
        {NULL, 0}
};

static const module_consts_t consts[] = {
        {NULL, 0}
};

/**
 * Returns the number of available slots.
 * For instance, a system with a single set of partitions would return
 * 1, a system with A/B would return 2, A/B/C -> 3...
 * @function get_slots_number
 * @return number of available system slot
 */
static int _get_slots_number(lua_State *L) {
    lua_pushinteger(L, get_slots_number());
    return 1;
}

/**
 * Returns the value letting the system know
 * whether the current slot is A or B. It is assumed that if the current slot
 * is A, then the block devices underlying B can be accessed directly
 * without any risk of corruption.
 * @function get_current_slot
 * @return @{slot}
 */
static int _get_current_slot(lua_State *L) {
    lua_pushinteger(L, get_current_slot());
    return 1;
}

/**
 * Marks the current slot
 * as having booted successfully
 * @function mark_as_succesful
 */
static int _mark_as_successful(lua_State *L) {
    const int ret = mark_as_successful();
    if (ret != 0) {
        luaL_error(L, "mark_as_successful failed with code:%d", ret);
    }
    return 1;
}

/**
 * Marks the slot passed in parameter as
 * the active boot slot.
 * @function mark_as_active
 * @param slot @{slot}
 */
static int _mark_as_active(lua_State *L) {
    const int ret = mark_as_active(luaL_checkinteger(L, 1));
    if (ret != 0) {
        luaL_error(L, "mark_as_active failed with code:%d", ret);
    }
    return 1;
}

/**
 * Marks the slot passed in parameter as
 * an unbootable. This can be used while updating the contents of the slot's
 * partitions, so that the system will not attempt to boot a known bad set up.
 * @function mark_as_unbootable
 * @param slot @{slot}
 */
static int _mark_as_unbootable(lua_State *L) {
    const int ret = mark_as_unbootable(luaL_checkinteger(L, 1));
    if (ret != 0) {
        luaL_error(L, "mark_as_unbootable failed with code:%d", ret);
    }
    return 1;
}

/**
 * Decreases boot attempt counter of the currently active slot by one.
 * If it is already 0, then such an operation have no effect.
 * @function decrease_boot_attempt
 * @return 0 if success, otherwise negative error code
 */
static int _decrease_boot_attempt(lua_State *L) {
    const int ret = decrease_boot_attempt();
    if (ret != 0) {
        luaL_error(L, "_decrease_boot_attempt failed with code:%d", ret);
    }
    return 1;
}

/**
 * Returns if the slot passed in parameter is bootable.
 * @function is_bootable
 * @return true or false
 */
static int _is_bootable(lua_State *L) {
    lua_pushinteger(L, is_bootable(luaL_checkinteger(L, 1)));
    return 1;
}

/**
 * Returns if the slot passed in parameter has
 * been marked as successful using mark_as_successful.
 * @function is_successful
 * @return true or false
 */
static int _is_successful(lua_State *L) {
    lua_pushinteger(L, is_successful(luaL_checkinteger(L, 1)));
    return 1;
}

/**
 * Returns the active slot to boot into on the next boot.
 * @function get_active
 * @return active @{slot}
 */
static int _get_active(lua_State *L) {
    lua_pushinteger(L, get_active());
    return 1;
}

/**
 * Returns the string suffix used by partitions that
 * correspond to the slot number passed in parameter.
 * @function get_suffix
 * @param slot @{slot}
 * @return prefix string
 */
static int _get_suffix(lua_State *L) {
    lua_pushstring(L, get_suffix(luaL_checkinteger(L, 1)));
    return 1;
}

/**
 * Returns the os binary name.
 * @function get_os
 * @return string containing os binary name
 */
static int _get_os(lua_State *L) {
    lua_pushstring(L, get_os());
    return 1;
}

/**
 * Returns the recovery binary name.
 * @function get_recovery
 * @return string containing recovery binary name
 */
static int _get_recovery(lua_State *L) {
    lua_pushstring(L, get_recovery());
    return 1;
}

/**
 * Returns the directory that stores binaries(os,recovery).
 * @function get_binary_dir
 * @return string containing binary directory name
 */
static int _get_binary_dir(lua_State *L) {
    lua_pushstring(L, get_binary_dir());
    return 1;
}

/**
 * Returns how many boot attempts left for specified slot.
 * @function get_boot_attempts_left
 * @param slot @{slot}
 * @return number of boot attempts left
 */
static int _get_boot_attempts_left(lua_State *L) {
    lua_pushinteger(L, get_boot_attempts_left(luaL_checkinteger(L, 1)));
    return 1;
}


static const struct luaL_Reg functions[] = {
        {"get_slots_number",       _get_slots_number},
        {"get_current_slot",       _get_current_slot},
        {"mark_as_successful",     _mark_as_successful},
        {"mark_as_active",         _mark_as_active},
        {"mark_as_unbootable",     _mark_as_unbootable},
        {"decrease_boot_attempt",  _decrease_boot_attempt},
        {"get_boot_attempts_left", _get_boot_attempts_left},
        {"is_bootable",            _is_bootable},
        {"is_successful",          _is_successful},
        {"get_active",             _get_active},
        {"get_suffix",             _get_suffix},
        {"get_os",                 _get_os},
        {"get_recovery",           _get_recovery},
        {"get_binary_dir",         _get_binary_dir},
        {NULL, NULL}
};

LUALIB_API int luaopen_lbootctrl(lua_State *L) {
    register_module(L, functions, consts, "lbootctrl");
    register_consts(L, system_slot, "slot");
    return 1;
}