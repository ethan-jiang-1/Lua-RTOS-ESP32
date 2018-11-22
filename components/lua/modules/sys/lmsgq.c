#include "sdkconfig.h"
#include "lua.h"
#include "lauxlib.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string.h>

#include "esp_log.h"
#include "msgq.h"

#if CONFIG_LUA_RTOS_LUA_USE_MSGQ

static char* TAG = "lmsgq";

static int l_open(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    if (!path) {
    		return luaL_error(L, "path missing");
    }

    int fd = open(path, O_RDWR);

    if (fd < 0) {
        	return luaL_error(L, "msgq path is invalid");
    }

    ESP_LOGD(TAG, "open %s fd: %d", path, fd);

    lua_pushinteger(L, fd);
    return 1;
}

static int l_write(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    const char* data = luaL_checkstring(L, 2);
    int length = luaL_checkinteger(L, 3);
    msg_data_t msg = {0};
    char* msg_data;

    ESP_LOGD(TAG, "l_write: %d", length);

    if (!data) {
    	return luaL_error(L, "data missing");
    }

    msg_data = malloc(strlen(data) + 1);
    if(!msg_data){
        return luaL_error(L, "write mem allocate error");
    }
    memset((void *)msg_data, 0, strlen(data) + 1);
    strcpy(msg_data, data);
    msg.data = msg_data;
    
    int ret = write(fd, &msg, strlen(data));

    lua_pushinteger(L, ret);
    return 1;
}


static int l_read(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    // int length = luaL_checkinteger(L, 2);
    msg_data_t msg = {0};

    int ret = read(fd, &msg, sizeof(msg_data_t));
    ESP_LOGD(TAG, "read fd: %d len: %d", fd, ret);

    if (ret <= 0){
        lua_pop(L, 1);
        lua_pushnil(L);
    }else{
        lua_pushstring(L, msg.data);
        free(msg.data);
    }

    return 1;
}


static int l_close(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    
    close(fd);

    return 0;
}

#include "modules.h"

static const LUA_REG_TYPE msgq_map[] =
{
  { LSTRKEY( "open" ),      LFUNCVAL( l_open  ) },
  { LSTRKEY( "write" ),      LFUNCVAL( l_write  ) },
  { LSTRKEY( "read" ),      LFUNCVAL( l_read  ) },
  { LSTRKEY( "close" ),     LFUNCVAL( l_close ) },
  { LNILKEY, LNILVAL }
};

int luaopen_msgq(lua_State *L) {
	return 0;
}
	   
MODULE_REGISTER_ROM(MSGQ, msgq, msgq_map, luaopen_msgq, 1);

#endif
