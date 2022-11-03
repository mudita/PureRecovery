// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "lua.h"

#ifdef __cplusplus
extern "C"
{
#endif

LUALIB_API int luaopen_lbootctrl(lua_State *L);

#ifdef __cplusplus
}
#endif