#include "sdkconfig.h"
#include "lua.h"
#include "lauxlib.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <string.h>

#if LUA_USE_ROTABLE
#include "lrotable.h"
#endif

#include "esp_log.h"
#include "msgq.h"

#if CONFIG_LUA_RTOS_LUA_USE_MSGQ

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

static char* TAG = "lmsgq";
static char* LUA_MSGQHANDLE = "MSGQ*";

typedef struct{
    int fd;
    bool closed;
}vfs_handler_t;

static vfs_handler_t *newfile (lua_State *L) {
  vfs_handler_t *p = (vfs_handler_t *)lua_newuserdata(L, sizeof(vfs_handler_t));
  luaL_setmetatable(L, LUA_MSGQHANDLE);
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
    vfs_handler_t *p  = (vfs_handler_t *)luaL_checkudata(L, 1, LUA_MSGQHANDLE);
    const char* data = luaL_checkstring(L, 2);
    // int length = luaL_checkinteger(L, 3);
    msg_data_t msg = {0};
    char* msg_data;

    // ESP_LOGD(TAG, "l_write: %d", length);

    if (!data) {
    	return luaL_error(L, "data missing");
    }

    msg_data = malloc(strlen(data) + 1);
    if(msg_data != NULL){
        memset((void *)msg_data, 0, strlen(data) + 1);
        strcpy(msg_data, data);
        msg.data = msg_data;
        
        int ret = write(p->fd, &msg, strlen(data));

        if(ret == 0){
            free(msg_data);
        } else{
            lua_pushvalue(L, 1);
            return 1;
        }
    }

    return luaL_fileresult(L, 0, NULL);
}


static int l_read(lua_State *L) {
    vfs_handler_t *p  = (vfs_handler_t *)luaL_checkudata(L, 1, LUA_MSGQHANDLE);

    msg_data_t msg = {0};

    int ret = read(p->fd, &msg, sizeof(msg_data_t));
    ESP_LOGD(TAG, "read fd: %d len: %d", p->fd, ret);

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
    vfs_handler_t *p  = (vfs_handler_t *)luaL_checkudata(L, 1, LUA_MSGQHANDLE);
    
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


static const LUA_REG_TYPE msgq_map[] = {
    { LSTRKEY( "close"      ),            LFUNCVAL( l_close   ) },
    { LSTRKEY( "open"       ),            LFUNCVAL( l_open    ) },
    // { "__index", luaL_io_index },
    { LNILKEY, LNILVAL }
};


int luaopen_msgq(lua_State *L) {

    luaL_newmetarotable(L, LUA_MSGQHANDLE, (void *)fops_maps);

    return 0;
}
	   
MODULE_REGISTER_ROM(MSGQ, msgq, msgq_map, luaopen_msgq, 1);

#endif
