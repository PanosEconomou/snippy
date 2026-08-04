/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   config.h | use lua to obtain a conifgurable list of snippets               ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#include "config.h"
#include "ring.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

/* ─────  Helpers ───────────────────────────────────────────────────────────────── */

static int config_stage_table(lua_State* state)
{
    lua_newtable(state);
    return luaL_ref(state, LUA_REGISTRYINDEX);
}

static void config_stage_commands (lua_State* state, const luaL_Reg* commands,
                                  int table)
{
    for (; commands->name && commands->func; commands++) {
        lua_rawgeti(state, LUA_REGISTRYINDEX, table);
        lua_pushcclosure(state, commands->func, 1);
        lua_setglobal(state, commands->name);
    }
}

static int config_stage_snippet(lua_State* state, int idx)
{
    luaL_checktype(state, idx, LUA_TTABLE);        /* Stack: ..[arg]..             */

    lua_getfield(state, idx, "trigger");           /* Stack: ..[arg]..[trigger]    */
    if (lua_type(state, -1) != LUA_TSTRING)
        return luaL_error(state, "trigger must be a string");

    size_t trigger_length = lua_objlen(state, -1);
    if (trigger_length == 0)
        return luaL_error(state, "trigger must be nonempty");
    if (trigger_length > RING_CAPACITY)
        return luaL_error(state, "trigger is too long.");
    lua_pop(state, 1);                              /* Stack: ..[arg]..             */

    lua_getfield(state, idx, "expand");             /* Stack: ..[arg]..[expand]     */
    int type = lua_type(state, -1);
    if (type != LUA_TSTRING && type != LUA_TFUNCTION)
        return luaL_error(state, "expansion must be a string or lua function");
    lua_pop(state, 1);                              /* Stack: ..[arg]..             */

    lua_pushvalue(state, lua_upvalueindex(1));      /* Stack: ..[arg]..[table]      */
    size_t table_size = lua_objlen(state, -1);

    lua_pushvalue(state, idx);                      /* Stack: ..[arg]..[table][arg] */ 
    lua_rawseti(state, -2, (int)table_size + 1);    /* Stack: ..[arg]..[table]      */
    lua_pop(state, 1);                              /* Stack: ..[arg]..             */

    return 0;
}

/* ─────  Config Library  ───────────────────────────────────────────────────────── */

int config_snippet(lua_State* state)
{
    int arguments = lua_gettop(state);
    for (int i = 1; i <= arguments; i++) config_stage_snippet(state, i);

    return 0;
}

int config_snippets(lua_State* state)
{
    luaL_checktype(state, 1, LUA_TTABLE);

    size_t table_size = lua_objlen(state, 1);
    for (size_t i = 1; i <= table_size; i++) {
        lua_rawgeti(state, 1, (int)i);
        config_stage_snippet(state, lua_gettop(state));
        lua_pop(state, 1);
    }

    return 0;
}

static const luaL_Reg config_commands[] = {
    { "snippet"     , config_snippet    },
    { "snippets"    , config_snippets   },
    { NULL, NULL}                                   /* to know when to stop a loop  */
};

/* ─────  Config Read  ──────────────────────────────────────────────────────────── */

int config_load(lua_State* state, const char* filename)
{
    int table = config_stage_table(state);
    if (table < 0) return -ENOMEM;

    config_stage_commands(state, config_commands, table);

    if (luaL_dofile(state, filename) != 0) {

        fprintf(stderr, "Config: Loading failed: %s", lua_tostring(state, -1));
        lua_pop(state, 1);

        return -EINVAL;
    }

    return table;
}
