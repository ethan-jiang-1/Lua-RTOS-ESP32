#include "lua.h"
#include "lauxlib.h"
#include <esp_system.h>
#include "esp_heap_caps.h"
#include "freertos/task.h"
#include "string.h"
#include "sys/time.h"

#if CONFIG_LUA_RTOS_LUA_USE_SYS

static int lget_freemem(lua_State *L) {
	int free_mem = esp_get_free_heap_size();
	lua_pushinteger(L, free_mem);

	return 1;
}

static int lget_freeiram(lua_State *L){
	int free_iram = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
	lua_pushinteger(L, free_iram);

	return 1;
}

#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) )
static int lget_taskinfo(lua_State *L) {

	char * pcWriteBuffer = malloc(2048);

	if(pcWriteBuffer != NULL){
		memset(pcWriteBuffer, 0, 2048);

		vTaskList(pcWriteBuffer);
		lua_pushstring(L, pcWriteBuffer);

		free(pcWriteBuffer);
	}else{
		lua_pushnil(L);
	}

	return 1;
}

#endif

static int lget_time_ms(lua_State *L) {
	struct timeval tv;
	char buf[16] = {0};
	gettimeofday(&tv, NULL);

	unsigned long time_ms = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
	sprintf(buf, "%lu", time_ms);

	lua_pushstring(L, buf);
	return 1;
}



#include "modules.h"

static const LUA_REG_TYPE sys_map[] =
{
  { LSTRKEY( "get_freemem" ),      LFUNCVAL( lget_freemem  ) },
  { LSTRKEY( "get_freeiram" ),      LFUNCVAL( lget_freeiram  ) },
  { LSTRKEY( "get_time_ms" ),      LFUNCVAL( lget_time_ms  ) },
#if ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) )
  { LSTRKEY( "get_taskinfo" ),      LFUNCVAL( lget_taskinfo  ) },
#endif

  { LNILKEY, LNILVAL }
};

int luaopen_sys(lua_State *L) {
	return 0;
}
	   
MODULE_REGISTER_ROM(SYS, sys, sys_map, luaopen_sys, 1);

#endif