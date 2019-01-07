#include "lua.h"
#include "lauxlib.h"
#include <esp_system.h>
#include "esp_heap_caps.h"
#include "freertos/task.h"
#include "string.h"
#include "sys/time.h"

#include "esp_log.h"

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

static int lget_sd_read_perf(lua_State *L) {
	const char* file_name = luaL_checkstring(L, 1);
	int mode = luaL_checknumber(L, 2); //0 - sram, 1 - spiram

	struct timeval tv;
	char* buff;
	int read_size = 4096;

	unsigned long time_ms_start;
	unsigned long time_ms_stop;
	unsigned long total_size = 0;
	float read_speed_per_second = 0;

	FILE * f = fopen(file_name, "rb");

	if (f != NULL){
		if(mode == 0)
			buff = heap_caps_malloc(read_size, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);
		else
			buff = heap_caps_malloc(read_size, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);

		if(buff != NULL){
			gettimeofday(&tv, NULL);
			time_ms_start = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
			while(1){
				int n = fread(buff, 1, read_size, f);
				total_size += n;
				if(n < read_size)
					break;
			}

			ESP_LOGI("Perf", "total_size: %lu", total_size);
			gettimeofday(&tv, NULL);
			time_ms_stop = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
		
			read_speed_per_second = total_size/(float)((time_ms_stop - time_ms_start)/1000);
			ESP_LOGI("Perf", "time consumption: %lu ms", time_ms_stop - time_ms_start);
			ESP_LOGI("Perf", "read speed: %f byte/second", read_speed_per_second);

			free(buff);
		}
		fclose(f);
	}

	lua_pushnumber(L, read_speed_per_second);
	return 1;
}

static int lget_sd_write_perf(lua_State *L) {
	const char* file_name = luaL_checkstring(L, 1);

	struct timeval tv;
	char* buff;
	int write_size = 4096;

	unsigned long time_ms_start;
	unsigned long time_ms_stop;
	unsigned long total_size = 0;
	float write_speed_per_second = 0;

	FILE * f = fopen(file_name, "wb");

	if (f != NULL){
		buff = heap_caps_malloc(write_size, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);

		if(buff != NULL){
			gettimeofday(&tv, NULL);
			time_ms_start = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
			while(total_size < 20*1024*1024){
				int n = fwrite(buff, 1, write_size, f);
				total_size += n;
			}

			ESP_LOGI("Perf", "total_size: %lu", total_size);
			gettimeofday(&tv, NULL);
			time_ms_stop = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
		
			write_speed_per_second = total_size/(float)((time_ms_stop - time_ms_start)/1000);
			ESP_LOGI("Perf", "time consumption: %lu ms", time_ms_stop - time_ms_start);
			ESP_LOGI("Perf", "write speed: %f byte/second", write_speed_per_second);

			free(buff);
		}
		fclose(f);
	}

	lua_pushnumber(L, write_speed_per_second);
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
  { LSTRKEY( "get_sd_read_perf" ),      LFUNCVAL( lget_sd_read_perf  )},
  { LSTRKEY( "get_sd_write_perf" ),      LFUNCVAL( lget_sd_write_perf  )},

  { LNILKEY, LNILVAL }
};

int luaopen_sys(lua_State *L) {
	return 0;
}
	   
MODULE_REGISTER_ROM(SYS, sys, sys_map, luaopen_sys, 1);

#endif