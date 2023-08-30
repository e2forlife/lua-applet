/*
 * bin_serialport.c
 *
 *  Created on: Jun 15, 2021
 *      Author: CErhardt
 */
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#include "lua.h"
#include "lprefix.h"
#include "lauxlib.h"
#include "lualib.h"

#include "serialport.h"

#ifdef SERIALPORT_H

#define TIMEOUT    (1000)    /**< global timeout for serial ops */

#define check_nl(c)    ( (c=='\r')||(c=='\n') )

/* ------------------------------------------------------------------------ */
typedef struct {
	HANDLE device;
	char   name[11];
	uint32_t baud;
	LUA_NUMBER stop;
	uint32_t parity;
	bool   initialized;
} lua_serialport_type;

#define LUA_EXT_SERIALPORT   ("_SERIALPORT_")
/* ------------------------------------------------------------------------ */
#define lua_ext_get_udata(L,n)   ( (lua_serialport_type*)luaL_checkudata(L,n,LUA_EXT_SERIALPORT))
/* ------------------------------------------------------------------------ */
static int sp_open( lua_State *L )
{
	lua_serialport_type *obj = lua_ext_get_udata(L,1);
	const char *name = luaL_checkstring(L,2);
	LUA_INTEGER p_baud = luaL_optinteger(L, 3, 9600);
	const char *p_par = luaL_optstring(L,4,"none");
	LUA_NUMBER p_stop = luaL_optinteger(L,5, 1);

	int stop_bits = one;
	int parity = off;

	if (p_stop == 1)        stop_bits = one;
	else if (p_stop == 1.5) stop_bits = onePointFive;
	else if (p_stop == 2)   stop_bits = two;
	else return luaL_error(L,"Expected number of stop bits must be 1, 1.5 or 2");

	if (strcmpi(p_par,"none")==0) parity = off;
	else if (strcmpi(p_par,"odd")==0) parity = odd;
	else if (strcmpi(p_par,"even")==0) parity = even;
	else return luaL_error(L,"Parity must be odd, even, or none");

	if (obj->initialized) {
		// have to close port first!
		return luaL_error(L,"port %s is already open",name);
	}
	// store port configuration in the port userdata
	strcpy(obj->name, name);
	obj->baud = p_baud;
	obj->stop = p_stop;
	obj->parity = parity;
	// open the port
	obj->device = openSerialPort(name, p_baud, stop_bits, parity);

	return 0;
}
/* ------------------------------------------------------------------------ */
static int sp_read( lua_State *L )
{
	luaL_Buffer bfr;

	lua_serialport_type *obj = lua_ext_get_udata(L,1);
	uint32_t size = luaL_optinteger(L,2,1);
	uint32_t timeout = luaL_optinteger(L,3,1000);
	uint32_t max_tries = luaL_optinteger(L,4,10);

	uint32_t ntries = 0;

	luaL_buffinitsize(L, &bfr, size);
	char* b = luaL_buffaddr(&bfr);
	uint32_t br = 0;
	do {
		uint32_t rb = (uint32_t) readFromSerialPort(obj->device, (char*)b+br,size-br);
		br += rb;
		if (rb == 0) {
			++ntries;
			Sleep(timeout);
		}
	} while ((br<size)&&(ntries < max_tries));
	luaL_addsize(&bfr,br);
	luaL_pushresult(&bfr);
	lua_pushboolean(L,ntries>max_tries);
	return 2;
}
/* ------------------------------------------------------------------------ */
static int sp_write( lua_State *L )
{
	lua_serialport_type *obj = lua_ext_get_udata(L,1);
	if (lua_type(L,2) != LUA_TSTRING)
		return luaL_error(L,"Argument 2 expected string.");
	uint32_t len = 0;
	char *data = (char*)lua_tolstring(L, 2, &len);

	lua_pushinteger( L, writeToSerialPort(obj->device, data, len));
	return 1;
}
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
static int sp_gc (lua_State *L)
{
  lua_serialport_type *obj = lua_ext_get_udata(L,1);

  if (obj->device != NULL) {
	  closeSerialPort(obj->device);
	  obj->device = NULL;
  }
  return 0;
}
/* ------------------------------------------------------------------------ */
static int sp_tostring (lua_State *L)
{
	lua_serialport_type *obj = lua_ext_get_udata(L,1);
	if (obj->device == NULL)
		lua_pushliteral(L, "serial (closed)");
	else
		lua_pushfstring(L, "serial (%p)", obj->device);
  return 1;
}
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

/* ------------------------------------------------------------------------ */
static const luaL_Reg lua_ext_serial_funcs[] = {
		{ "open", sp_open },
		{ "read", sp_read },
		{ "write", sp_write },
		{ "close", sp_gc },
		{NULL, NULL }
};
/* ------------------------------------------------------------------------ */
static const luaL_Reg serial_metameth[] = {
  {"__index", NULL},  /* place holder */
  {"__gc", sp_gc},
  {"__close", sp_gc},
  {"__tostring", sp_tostring},
  {NULL, NULL}
};
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* ------------------------------------------------------------------------ */
static int sp_new_serial( lua_State *L )
{
	lua_serialport_type *p = (lua_serialport_type *)lua_newuserdatauv(L, sizeof(lua_serialport_type), 0);
	memset((void*)p,0,sizeof(lua_serialport_type));
	luaL_setmetatable(L, LUA_EXT_SERIALPORT);
	return 1;
}
/* ------------------------------------------------------------------------ */
LUALIB_API int __cdecl luaopen_brooks_serial( lua_State *L)
{
	/* build the serial port extension Lua interface */
	luaL_newmetatable(L, LUA_EXT_SERIALPORT);  /* metatable for serial extension */
	luaL_setfuncs(L, serial_metameth, 0);  /* add metamethods to new metatable */
	luaL_newlibtable(L, lua_ext_serial_funcs);  /* create method table */
	luaL_setfuncs(L, lua_ext_serial_funcs, 0);  /* add file methods to method table */
	lua_setfield(L, -2, "__index");  /* metatable.__index = method table */
	lua_pop(L,1);

	/* create the "new" function for creating the serial object */
	lua_pushcfunction(L, sp_new_serial);
	lua_setglobal(L, "serial");
	return 0;
}

#endif