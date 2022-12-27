/*
 * lua_ext_error.c
 *
 *  Created on: Jun 15, 2021
 *      Author: CErhardt
 */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "lua.h"
#include "lprefix.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lua_ext_error.h"

FILE *error_file = NULL;

void error_log(uint32_t line, char * file, uint32_t code, const char *fmt, ...)
{
	error_code_t err = {
			.line_number = line,
			.error_code = code
	};
	strcpy(err.filename,file);
    va_list args;
    va_start(args,fmt);
    vsprintf(err.message, fmt, args);
    if (error_file != NULL) {
    	fprintf(stderr, "%08X - %s (%d) : %s\r\n",
    			err.error_code, err.filename, err.line_number,
				err.message);
    }
}

