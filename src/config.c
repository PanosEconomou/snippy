/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   config.h | use lua to obtain a conifgurable list of snippets               ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#include "config.h"

#include <lua.h>
#include <lauxlib.h>

/* ─────  Helpers ───────────────────────────────────────────────────────────────── */

static int config_stage_table(lua_State* state)
{
    return 0;
}

static int config_stage_commands (lua_State* state, const struct luaL_Reg* commands,
                                  int staging_index)
{
    for (; commands->name && commands->func; commands++) {
        lua_pushvalue(state, staging_index);
        lua_pushcclosure(state, commands->func, 1);
        lua_setglobal(state, commands->name);
    }

    return 0;
}

/* ─────  Config Library  ───────────────────────────────────────────────────────── */

int config_snippet(lua_State* state)
{
    return 0;
}

int config_snippets(lua_State* state)
{
    return 0;
}

static const struct luaL_Reg config_commands[] = {
    { "snippet"     , config_snippet    },
    { "snippets"    , config_snippets   },
    { NULL, NULL}                                   /* to know when to stop a loop  */
};

/* ─────  Config Read  ──────────────────────────────────────────────────────────── */

int config_load(lua_State* state, const char* filename)
{
    int return_code = config_stage_table(state);
    if (return_code < 0) return return_code;

    return_code     = config_stage_commands(state, config_commands, 1);
    if (return_code < 0) return return_code;

    return_code     = luaL_dofile(state, filename);
    if (return_code < 0) return return_code;

    return return_code;
}
