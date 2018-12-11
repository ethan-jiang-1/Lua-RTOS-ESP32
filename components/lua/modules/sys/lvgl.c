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

#include "esp_log.h"

#if CONFIG_LUA_RTOS_LUA_USE_LVGL

/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#if !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#if !defined(L_MODEEXT)
#define L_MODEEXT   "b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
static int l_checkmode (const char *mode) {
  return (*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&
         (*mode != '+' || (++mode, 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

static char* TAG = "lvgl";
static char* LUA_LVGLHANDLE = "LVGL*";

typedef struct{
    int fd;
    bool closed;
}vfs_handler_t;

static vfs_handler_t *newfile (lua_State *L) {
  vfs_handler_t *p = (vfs_handler_t *)lua_newuserdata(L, sizeof(vfs_handler_t));
  luaL_setmetatable(L, LUA_LVGLHANDLE);
  return p;
}

static int l_open(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  vfs_handler_t *p = newfile(L);
  const char *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  p->fd = open(filename, O_RDWR);
  p->closed = false;

  return (p->fd < 0) ? luaL_fileresult(L, 0, filename) : 1;
}

static int l_write(lua_State *L) {
    vfs_handler_t *p  = (vfs_handler_t *)luaL_checkudata(L, 1, LUA_LVGLHANDLE);
    const char* data = luaL_checkstring(L, 2);

    if (!data) {
    		return luaL_error(L, "data missing");
    }

    int ret = write(p->fd, data, strlen(data));

    lua_pushinteger(L, ret);
    return 1;
}


static int l_read(lua_State *L) {
    // int fd = luaL_checkinteger(L, 1);
    // int length = luaL_checkinteger(L, 2);
    // char* p;
    // luaL_Buffer b;
    // luaL_buffinit(L, &b);
    // p = luaL_prepbuffsize(&b, length);

    // int ret = read(fd, p, length);
    // luaL_addsize(&b, ret);
    // luaL_pushresult(&b);

    // if (ret <= 0)
    // {
    //     lua_pop(L, 1);
    //     lua_pushnil(L);
    // } 

    // return 1;

    return 0;
}

static int l_close(lua_State *L) {
    vfs_handler_t *p  = (vfs_handler_t *)luaL_checkudata(L, 1, LUA_LVGLHANDLE);
    
    if(!p->closed){
        ESP_LOGI(TAG, "closed fd: %d", p->fd);
        p->closed = true;
        close(p->fd);
    }

    return 0;
}

static int f_gc (lua_State *L) {
  return l_close(L);
}

#include "modules.h"

static const LUA_REG_TYPE fops_maps[] = {
    { LSTRKEY( "write" ),           LFUNCVAL( l_write ) },
    { LSTRKEY( "read" ),            LFUNCVAL( l_read  ) },
    { LSTRKEY( "close" ),           LFUNCVAL( l_close ) }, 
    { LSTRKEY( "__metatable" ),     LROVAL  ( fops_maps ) },
    { LSTRKEY( "__index"     ),     LROVAL  ( fops_maps ) },
    { LSTRKEY( "__gc" ),            LFUNCVAL( f_gc   ) },
    { LNILKEY, LNILVAL }
};


static const LUA_REG_TYPE lvgl_map[] = {
    { LSTRKEY( "close"      ),            LFUNCVAL( l_close   ) },
    { LSTRKEY( "open"       ),            LFUNCVAL( l_open    ) },
    // { "__index", luaL_io_index },
    { LNILKEY, LNILVAL }
};


int luaopen_lvgl(lua_State *L) {

    luaL_newmetarotable(L, LUA_LVGLHANDLE, (void *)fops_maps);

    return 0;
}
	   
MODULE_REGISTER_ROM(LVGL, lvgl, lvgl_map, luaopen_lvgl, 1);

#endif
