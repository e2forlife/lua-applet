/*
 * ansi_proc.c
 *
 *  Created on: Sep 15, 2021
 *      Author: CErhardt
 */
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

#include "lua.h"
#include "lprefix.h"
#include "lauxlib.h"
#include "lualib.h"

#define ANSI_SOP       ('{')
#define ANSI_EOP       ('}')

typedef struct {
	const char *cmd;
	void (*fn)(uint32_t *param, uint32_t cnt);
} ansi_cmds;

static bool ansi_enable = true;

/* ANSI Commands ========================================================== */
static void ansi_fg_color( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;

	uint32_t color = arg[0];
	if (arg[0] > 15) {
		// 256-bit color
		printf("\x1b[38;5;%dm",color);
	}
	else if (arg[0] > 7) {
		// high-intensity
		printf("\x1b[9%dm", (color-8));
	}
	else {
		printf("\x1b[3%dm",color);
	}
}

static void ansi_bg_color( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;

	uint32_t color = arg[0];
	if (arg[0] > 15) {
		// 256-bit color
		printf("\x1b[48;5;%dm",color);
	}
	else if (arg[0] > 7) {
		// high-intensity
		printf("\x1b[10%dm", (color-8));
	}
	else {
		printf("\x1b[4%dm",color);
	}
}

static void ansi_cursor_up( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;
	printf("\x1b[%dA",arg[0]);
}

static void ansi_cursor_down( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;
	printf("\x1b[%dB",arg[0]);

}

static void ansi_cursor_left( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;
	printf("\x1b[%dD",arg[0]);

}

static void ansi_cursor_right( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;
	printf("\x1b[%dC",arg[0]);

}

static void ansi_set_cursor( uint32_t *arg, uint32_t count)
{
	if (count == 0) return;
	uint32_t row = (arg[0]==0)?1:arg[0];
	uint32_t col = (count>1)?arg[1]:1;
	printf("\x1b[%d;%dH",row,col);
}

static void ansi_hide_cusor( uint32_t *arg, uint32_t count)
{
	printf("\x1b[?25l");
}

static void ansi_show_cursor( uint32_t *arg, uint32_t count)
{
	printf("\x1b[?25h");
}

static void ansi_save_cursor( uint32_t *arg, uint32_t count)
{
	printf("\x1b[s");
}

static void ansi_restore_cursor( uint32_t *arg, uint32_t count)
{
	printf("\x1b[u");
}

static void ansi_clear_screen( uint32_t *arg, uint32_t count)
{
	if (count < 1) {
		printf("\x1b[2J");
	}
	else {
		printf("\x1b[%dJ",arg[0]); // 0 == cursor to top
		                           // 1 == Cursor to bottom
		                           // 2 == Whole screen
		                           // 3 = Whole screen + buffer
	}
}

static void ansi_clear_line( uint32_t *arg, uint32_t count )
{
	if (count < 1) {
		// default
		printf("\x1b[K");
	}
	else {
		printf("\x1b[%dK",arg[0]); // 0 == cursor to EOL
		                           // 1 == Cursor to start of line
		                           // 2 == Whole line

	}
}
/* Command Assignment Table =============================================== */
/* ------------------------------------------------------------------------ */
static const ansi_cmds cmds[] =
{
	{ "c",       ansi_fg_color },
	{ "b",       ansi_bg_color },
	{ "fg",      ansi_fg_color },
	{ "bg",      ansi_bg_color },
	{ "up",      ansi_cursor_up },
	{ "down",    ansi_cursor_down },
	{ "left",    ansi_cursor_left },
	{ "right",   ansi_cursor_right },
	{ "mv",      ansi_set_cursor },
	{ "hide",    ansi_hide_cusor },
	{ "show",    ansi_show_cursor },
	{ "save",    ansi_save_cursor },
	{ "restore", ansi_restore_cursor },
	{ "csr",     ansi_clear_screen },
	{ "cln",     ansi_clear_line },
	// The last command --------------------------------------------------- */
	{ NULL, NULL}
};
/* ------------------------------------------------------------------------ */
static void ansi_exec_cmd( char *cmd, uint32_t len, uint32_t *arg, uint32_t count )
{
	ansi_cmds *tab = (ansi_cmds*)&cmds[0];

	// while there are commands in the
	while (tab->cmd != NULL) {
		if (strncasecmp(tab->cmd, cmd, len) == 0) {
			void (*fn)(uint32_t *param, uint32_t cnt) = tab->fn;
			if ((fn != NULL)&&(ansi_enable==true)) fn(arg,count);
			// once the command is executed, return to the parser
			return;
		}
		tab++;
	}
}
/* Parser ================================================================= */
/* ------------------------------------------------------------------------ */
/* string parser */
static void process_ansi_string( const char *s, uint32_t length)
{
	bool esc = false;
	char *cmd = NULL;
	uint32_t cmd_size = 0;
	uint32_t args = 0;
	uint32_t arg[2] = {0};
	for (uint32_t indx = 0;(indx<length)&&(s[indx]!=0);++indx) {
		if (esc) {
			/*
			 * when there is an "open" brace in the buffer, print
			 * the character and then exit command mode.
			 */
			if (s[indx] == ANSI_SOP) {
				putchar( (int) s[indx]);
				esc = false;
			}
			/*
			 * While the characters being read are letters,
			 * build up the command string.
			 */
			else if (isalpha((int)s[indx])) {
				if (cmd != NULL) {
					++cmd_size;
				}
				else {
					cmd = (char*)&s[indx];
					cmd_size = 1;
				}
			}
			else if (isdigit((int)s[indx])) {
				if (args == 0) {
					++args;
					arg[0] = 0;
					arg[1] = 0;
				}
				arg[args-1] = arg[args-1]*10 + (s[indx]-'0');
			}
			/*
			 * Process extra arguments: {C1,2}
			 */
			else if (s[indx] == ',') {
				// next parameter
				args++;
				if (args > 2) args = 2; // ERROR! can't have more than 2 args
				arg[args-1] = 0;
			}
			/*
			 * Process stacked commands: {C1,2;D3,4}
			 */
			else if ((s[indx] == ';')||(s[indx]==ANSI_EOP)) {
				// double up command, execute the command
				ansi_exec_cmd(cmd,cmd_size,arg,args);
				args = 0;
				cmd = NULL;
				cmd_size = 0;
				esc = !(s[indx]==ANSI_EOP);
			}
		}
		else if (s[indx] == ANSI_SOP) {
			esc = true;
		}
		else {
			// just a regular character
			putchar((int) s[indx]);
		}
	}
}
/* ------------------------------------------------------------------------ */
/* An ANSI parsing version of "print" without newline at EOL */
int ext_ansi_print( lua_State *L )
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	for (i = 1; i <= n; i++) {  /* for each argument */
		size_t l;
	    const char *s = luaL_tolstring(L, i, &l);  /* convert it to string */
	    if (i > 1)  /* not the first element? */
	      lua_writestring("\t", 1);  /* add a tab before it */
	    process_ansi_string(s,l);
	    fflush(stdout);
	}
	return 0;
}
/* ------------------------------------------------------------------------ */
int ext_ansi_enable( lua_State *L )
{
	if (lua_isnil(L,1)) ansi_enable = true;
	else ansi_enable = lua_toboolean(L,1);
	return 0;
}
