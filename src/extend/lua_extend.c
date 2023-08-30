/*
 * lua_delay.c
 *
 *  Created on: Jun 16, 2021
 *      Author: CErhardt
 */
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#ifdef WIN32
/* When windows is being used as the host OS, include the windows headers, and
 * the console IO headers to access functions like Sleep() and kbhit() 
 */
#include <Windows.h>
#include <conio.h>

#else

#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>

int kbhit() 
{
	struct termios old, current;

	tcgetattr(0, &old); /* grab old terminal i/o settings */
  	memcpy(&current, &old, sizeof(struct termios)); /* make new settings same as old settings */
  	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set no echo mode */
  	tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */

    int bytesWaiting;
    ioctl(0, FIONREAD, &bytesWaiting);

	tcsetattr(0, TCSANOW, &old);

    return bytesWaiting;
}

#define Sleep( ms )              usleep( ms * 1000 )

int getch( void )
{
	struct termios old, current;
	int ch = 0;

	if (kbhit() ) {
		tcgetattr(0, &old); /* grab old terminal i/o settings */
		memcpy(&current, &old, sizeof(struct termios)); /* make new settings same as old settings */
		current.c_lflag &= ~ICANON; /* disable buffered i/o */
		current.c_lflag &= ~ECHO; /* set no echo mode */
		tcsetattr(0, TCSANOW, &current); /* use these new terminal i/o settings now */
		ch = getchar();
		tcsetattr(0, TCSANOW, &old);
	}
	return ch;
}
#endif


#include "lua.h"
#include "lprefix.h"
#include "lauxlib.h"
#include "lualib.h"

int ext_ansi_print( lua_State *L );
int ext_ansi_enable( lua_State *L );

static int lua_ext_delay( lua_State* L)
{
	uint32_t ms_delay = luaL_optinteger(L,1,1);
	Sleep(ms_delay);
	return 0;
}
/* ======================================================================== */
#define swap_endian(v,bytes)   do {\
    for (uint32_t indx=0;indx<bytes/2;++indx) {\
        uint8_t t = (uint8_t)v[indx];\
        v[indx] = v[bytes-indx-1];\
        v[bytes-indx-1] = t;\
    }\
} while(0)
/* ======================================================================== */
static int conv_int64(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isinteger(L,1)) {
			// convert from int to string
			lua_Integer val = lua_tointeger(L,1);
			char *bfr = (char*)&val;
			if (big) swap_endian(bfr,8);
			lua_pushlstring(L, bfr, 8);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		int64_t* val= (int64_t*) lua_tostring(L,1);
		char *c = (char*)val;
		if (big) swap_endian(c,8);
		lua_pushinteger(L, (*val)&0xFFFFFFFFFFFFFFFF);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_int32(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isinteger(L,1)) {
			// convert from int to string
			lua_Integer val = lua_tointeger(L,1);
			char *bfr = (char*)&val;
			if (big) swap_endian(bfr,4);
			lua_pushlstring(L, bfr, 4);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		int32_t* val = (int32_t*) lua_tostring(L,1);
		char *c = (char*)val;
		if (big) swap_endian(c,4);
		lua_Integer I = *val;
		lua_pushinteger(L, I);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_int16(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isinteger(L,1)) {
		// convert from int to string
		lua_Integer val = lua_tointeger(L,1);
		char *bfr = (char*)&val;
		if (big) swap_endian(bfr,2);
		lua_pushlstring(L, bfr, 2);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		int16_t* val= (int16_t*) lua_tostring(L,1);
		if (big) swap_endian(((char*)val),2);
		lua_Integer I = *val;
		lua_pushinteger(L, I);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}

/* ======================================================================== */
static int conv_uint64(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isinteger(L,1)) {
			// convert from int to string
			lua_Integer val = lua_tointeger(L,1);
			char *bfr = (char*)&val;
			if (big) swap_endian(bfr,8);
			lua_pushlstring(L, bfr, 8);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		uint64_t* val= (uint64_t*) lua_tostring(L,1);
		char *c = (char*)val;
		if (big) swap_endian(c,8);
		lua_pushinteger(L, (*val)&0xFFFFFFFFFFFFFFFF);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_uint32(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isinteger(L,1)) {
			// convert from int to string
			lua_Integer val = lua_tointeger(L,1);
			char *bfr = (char*)&val;
			if (big) swap_endian(bfr,4);
			lua_pushlstring(L, bfr, 4);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		uint32_t* val= (uint32_t*) lua_tostring(L,1);
		char *c = (char*)val;
		if (big) swap_endian(c,4);
		lua_Integer I = *val;
		lua_pushinteger(L, I);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_float(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isnumber(L,1)) {
		// convert from int to string
		float val = lua_tonumber(L,1);
		char *bfr = (char*)&val;
		if (big) swap_endian(bfr,4);
		lua_pushlstring(L, bfr, 4);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		float* val= (float*) lua_tostring(L,1);
		char *c = (char*)val;
		if (big) swap_endian(c,4);
		lua_pushnumber(L, *val);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or number");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_uint16(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isinteger(L,1)) {
		// convert from int to string
		lua_Integer val = lua_tointeger(L,1);
		char *bfr = (char*)&val;
		if (big) swap_endian(bfr,2);
		lua_pushlstring(L, bfr, 2);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		uint16_t* val= (uint16_t*) lua_tostring(L,1);
		if (big) swap_endian(((char*)val),2);
		lua_Integer I = *val;
		lua_pushinteger(L, I);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_uint8(lua_State *L )
{
	if (lua_isinteger(L,1)) {
		// convert from int to string
		lua_Integer val = lua_tointeger(L,1);
		const char *bfr = (const char*)&val;
		lua_pushlstring(L, bfr, 1);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		lua_Integer* val= (lua_Integer*) lua_tostring(L,1);
		lua_pushinteger(L, (*val)&0xFF);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or integer");

	return 1;
}
/* ------------------------------------------------------------------------ */
static int conv_double(lua_State *L )
{
	bool big = lua_toboolean(L, 2);
	if (lua_isnumber(L,1)) {
		// convert from int to string
		double val = lua_tonumber(L,1);
		char *bfr = (char*)&val;
		if (big) swap_endian(bfr,8);
		lua_pushlstring(L, bfr, 8);
	}
	else if (lua_isstring(L,1)) {
		// convert from string to Unit32
		double* val= (double*) lua_tostring(L,1);
		if (big) swap_endian( ((char*)val), sizeof(double));
		lua_pushnumber(L, *val);
	}
	else return
			luaL_error(L,"Incompatible type for argument 1, expected string or number");

	return 1;
}

/* ------------------------------------------------------------------------ */
static int lua_encrypt(lua_State *L)
{
	uint32_t idata_len = 0;
	const char *idata = luaL_checklstring(L,1,(size_t*)&idata_len);
	uint32_t key_len = 0;
	uint32_t cs = 0;
	uint32_t ecs = 0;
	for (uint32_t indx=0;indx<idata_len;++indx)
		cs += (uint8_t)idata[indx];

	if ((lua_gettop(L) == 1) || (lua_isnil(L,2))) {
		// no key, just return input data
		lua_pushlstring(L,idata,idata_len);
		lua_pushinteger(L,cs);
		lua_pushinteger(L,cs);
	}
	else {
		const char* key = luaL_checklstring(L,2,(size_t*)&key_len);
		uint32_t ki = 0;
		luaL_Buffer bfr;

		luaL_buffinitsize(L, &bfr, idata_len);
		for(uint32_t indx = 0;indx<idata_len;++indx) {
			uint32_t val = (idata[indx]^key[ki++])&0xFF;
			luaL_addchar(&bfr, (val) );
			if (ki>=key_len) ki = 0;
			ecs += (uint8_t)val;
		}
		luaL_pushresult(&bfr);
		lua_pushinteger(L,cs);
		lua_pushinteger(L,ecs);
	}
	return 3;
}

/* ------------------------------------------------------------------------ */
static int lua_ext_getchar( lua_State *L)
{
	int v = getch();
	if (v != 0) {
		lua_pushlstring(L,(const char*)&v,1);
	}
	else lua_pushnil(L);
	return 1;
}
/* ------------------------------------------------------------------------ */
static int lua_ext_kbhit( lua_State *L )
{
	lua_pushboolean(L,kbhit());
	return 1;
}
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
LUALIB_API int luaopen_ext( lua_State *L)
{
	/*
	 * Create the global extension functions that reside within the "C"
	 * lua kernel.  These provide access to some system functions as
	 * well as some other operations that are nice in C, but are a pain
	 * using Lua (like type casting).
	 */
	lua_pushcfunction(L, lua_ext_delay);
	lua_setglobal(L, "delay");

	lua_pushcfunction(L,ext_ansi_print);
	lua_setglobal(L,"ansi");
	lua_pushcfunction(L,ext_ansi_enable);
	lua_setglobal(L,"ansi_enable");

	lua_pushcfunction(L,conv_uint64);
	lua_setglobal(L,"uint64");
	lua_pushcfunction(L,conv_uint32);
	lua_setglobal(L,"uint32");
	lua_pushcfunction(L,conv_uint16);
	lua_setglobal(L,"uint16");
	lua_pushcfunction(L,conv_uint8);
	lua_setglobal(L,"uint8");
	lua_pushcfunction(L,conv_float);
	lua_setglobal(L,"float");
	lua_pushcfunction(L,conv_double);
	lua_setglobal(L,"double");

	lua_pushcfunction(L,conv_int64);
	lua_setglobal(L,"int64");
	lua_pushcfunction(L,conv_int32);
	lua_setglobal(L,"int32");
	lua_pushcfunction(L,conv_int16);
	lua_setglobal(L,"int16");

	lua_pushcfunction(L,lua_ext_getchar);
	lua_setglobal(L,"getc");
	lua_pushcfunction(L,lua_ext_kbhit);
	lua_setglobal(L,"kbhit");

	lua_register(L,"encrypt",lua_encrypt);
	return 0;
}


