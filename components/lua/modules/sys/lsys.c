#include "lua.h"
#include "lauxlib.h"
#include <esp_system.h>

#if CONFIG_LUA_RTOS_LUA_USE_SYS

static int lget_freemem(lua_State *L) {
	int free_mem = esp_get_free_heap_size();
	lua_pushinteger(L, free_mem);

	return 1;
}


#include "modules.h"

static const LUA_REG_TYPE sys_map[] =
{
  { LSTRKEY( "get_freemem" ),      LFUNCVAL( lget_freemem  ) },
  { LNILKEY, LNILVAL }
};

int luaopen_sys(lua_State *L) {
	return 0;
}
	   
MODULE_REGISTER_ROM(SYS, sys, sys_map, luaopen_sys, 1);

#endif