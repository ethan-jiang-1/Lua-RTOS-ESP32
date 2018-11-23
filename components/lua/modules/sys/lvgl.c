/*
 * Copyright (C) 2015 - 2018, IBEROXARXA SERVICIOS INTEGRALES, S.L.
 * Copyright (C) 2015 - 2018, Jaume Olivé Petrus (jolive@whitecatboard.org)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *     * The WHITECAT logotype cannot be changed, you can remove it, but you
 *       cannot change it in any way. The WHITECAT logotype is:
 *
 *          /\       /\
 *         /  \_____/  \
 *        /_____________\
 *        W H I T E C A T
 *
 *     * Redistributions in binary form must retain all copyright notices printed
 *       to any local or remote output device. This include any reference to
 *       Lua RTOS, whitecatboard.org, Lua, and other copyright notices that may
 *       appear in the future.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Lua RTOS, Lua fs (file system) module
 *
 */

#include "sdkconfig.h"
#include "lua.h"
#include "lauxlib.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 

#if CONFIG_LUA_RTOS_LUA_USE_LVGL

static int l_open(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    if (!path) {
    		return luaL_error(L, "path missing");
    }


    int fd = open(path, O_RDWR);

    if (fd < 0) {
        	return luaL_error(L, "lvgl path is invalid");
    }

    lua_pushinteger(L, fd);
    return 1;
}

static int l_write(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    const char* data = luaL_checkstring(L, 2);
    int length = luaL_checkinteger(L, 3);

    if (!data) {
    		return luaL_error(L, "data missing");
    }

    int ret = write(fd, data, length);

    lua_pushinteger(L, ret);
    return 1;
}


static int l_read(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    int length = luaL_checkinteger(L, 2);
    char* p;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    p = luaL_prepbuffsize(&b, length);

    int ret = read(fd, p, length);
    luaL_addsize(&b, ret);
    luaL_pushresult(&b);

    if (ret <= 0)
    {
        lua_pop(L, 1);
        lua_pushnil(L);
    } 

    return 1;
}



static int l_close(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    
    close(fd);

    return 0;
}

#include "modules.h"

static const LUA_REG_TYPE lvgl_map[] =
{
  { LSTRKEY( "open" ),      LFUNCVAL( l_open  ) },
  { LSTRKEY( "write" ),      LFUNCVAL( l_write  ) },
  { LSTRKEY( "read" ),      LFUNCVAL( l_read  ) },
  { LSTRKEY( "close" ),     LFUNCVAL( l_close ) },
  { LNILKEY, LNILVAL }
};

int luaopen_lvgl(lua_State *L) {
	return 0;
}
	   
MODULE_REGISTER_ROM(LVGL, lvgl, lvgl_map, luaopen_lvgl, 1);

#endif
