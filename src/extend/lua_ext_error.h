/*
 * lua_ext_error.h
 *
 *  Created on: Jun 15, 2021
 *      Author: CErhardt
 */

#ifndef SRC_LUA_EXT_ERROR_H_
#define SRC_LUA_EXT_ERROR_H_

#include <stdint.h>

typedef struct {
	uint32_t line_number;
	char     filename[256];
	uint32_t  error_code;
	char    message[256];
} error_code_t;

void error_log(uint32_t line, char * file, uint32_t code, const char *fmt, ...);

#endif /* SRC_LUA_EXT_ERROR_H_ */
