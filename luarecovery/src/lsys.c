// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

/// System related API
// @module recovery.sys

#include "lsys.h"
#include "lauxlib.h"
#include "common.h"
#include <hal/boot_reason.h>
#include <hal/delay.h>
#include <flash_bootloader.h>
#include <hal/boot_control.h>

static const module_consts_t consts[] = {
        {NULL, 0}
};

/***
 Boot reason codes
 @table boot_reason_codes
 @field update restart caused by the update request
 @field recovery restart caused by the recovery request
 @field factory restart caused by the factory reset request
 @field pgm_keys load keys request (close configuration)
 @field usb_mc_mode restart caused by the USB MSC request
 @field backup restart caused by the backup request
 @field restore restart caused by the restore request
 @field os restart caused by the OS itself
 @field unknown unknown boot reason code
 */
#define SC(s)   { #s, boot_reason_code_ ## s },
static const module_consts_t boot_reason_code[] = {
        SC(update)
        SC(recovery)
        SC(factory)
        SC(update)
        SC(unknown)
        {NULL, 0}
};

/***
 Get the boot reason
 @function boot_reason
 @return @{boot_reason_codes}
 */
static int _get_reason(lua_State *L) {
    lua_pushinteger(L, boot_reason());
    return 1;
}

/***
 Get the boot reason as a string
 @function boot_reason_str
 @return boot reason encoded as a string
 */
static int _get_reason_str(lua_State *L) {
    lua_pushstring(L, boot_reason_code_str(boot_reason()));
    return 1;
}

/***
 Set the boot reason code
 @function set_boot_reason
 @param code @{boot_reason_codes}
 */
static int _set_boot_reason(lua_State *L) {
    set_boot_reason(luaL_checkinteger(L, 1));
    return 1;
}

/**
 Sleep for specified time
 @function sleep
 @param seconds
 */
static int _sleep(lua_State *L) {
    const double sec = luaL_checknumber(L, 1);
    msleep(sec * 1000);
    return 1;
}

/***
 Get the number of second since the device start
 @function uptime
 @return seconds
 */
static int _uptime(lua_State *L) {
    UNUSED(L);
    const int jiffiess = get_jiffiess();
    lua_pushinteger(L, jiffiess);
    return 1;
}

/***
 Get the path to the slot we want to perform operations on.
 @function target_slot
 @return string containing system slot path
 */
static int _target_slot(lua_State *L) {
    UNUSED(L);
    const char *slot = get_suffix(get_active());
    lua_pushstring(L, slot);
    return 1;
}

/***
 Get the path to the currently active slot
 @function source_slot
 @return string containing system slot path
 */
static int _source_slot(lua_State *L) {
    UNUSED(L);
    const char *slot = get_suffix(get_current_slot());
    lua_pushstring(L, slot);
    return 1;
}

/***
 Get the path to the user partition
 @function user
 @return string containing user partition path
 */
static int _user(lua_State *L) {
    UNUSED(L);
    lua_pushstring(L, "/user");
    return 1;
}

/***
 Flash bootloader image into the boot partition
 @function flash_bootloader
 @param path path to the bootloader image
 */
static int _flash_bootloader(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    const int ret = flash_bootloader(path);
    if (ret != 0) {
        luaL_error(L, "Flashing bootloader failed with code:%d", ret);
    }
    return 1;
}

/***
 Change the existing system partition type from FAT to ext4
 @function fat_to_ext4
 @return operation status
 @warning Use it wisely as it is irreversible operation
 */
static int _fat_to_ext4(lua_State *L) {
    //const char *path = luaL_checkstring(L, 1);
    //TODO:
    return 1;
}

static const struct luaL_Reg functions[] = {
        {"boot_reason",      _get_reason},
        {"boot_reason_str",  _get_reason_str},
        {"set_boot_reason",  _set_boot_reason},
        {"sleep",            _sleep},
        {"uptime",           _uptime},
        {"target_slot",      _target_slot},
        {"source_slot",      _source_slot},
        {"user",             _user},
        {"flash_bootloader", _flash_bootloader},
        {"fat_to_ext4",      _fat_to_ext4},
        {NULL, NULL},
};

LUALIB_API int luaopen_lsys(lua_State *L) {
    register_module(L, functions, consts, "sys");
    register_consts(L, boot_reason_code, "boot_reason_codes");
    return 1;
}