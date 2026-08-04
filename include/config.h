/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   config.h | use lua to obtain a conifgurable list of snippets               ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_CONFIG_H
#define SNIPPY_CONFIG_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include <stdint.h>

#include <lua.h>                                    /* lua_State                    */
#include <lauxlib.h>                                /* luaL_Reg                     */

/* -----  Structures  ───────────────────────────────────────────────────────────── */

struct config {
    lua_State* state;                               /* lua interface entrypoint     */
    int        table;                               /* index of table on lua stack  */

    uint32_t   snippet_count;                       /* total snippets loaded        */
    uint32_t   literal_count;                       /* total literal snippets       */
    uint32_t   callback_count;                      /* total callback snippets      */
    uint32_t   trigger_bytes;                       /* total trigger characters     */
    uint32_t   literal_bytes;                       /* total literal expansion char */
};

/* ─────  Config Library  ───────────────────────────────────────────────────────── */

/*
 * In config.lua call as:
 * snippet { trigger = "trigger", expand = "expand "} or
 * snippet (
 *   { trigger = "trigger", expand = "expand "},
 *   { trigger = "trigger", expand = "expand "}
 * )
 */
int config_snippet(lua_State* state);

/*
 * In config.lua call as:
 * snippets {
 *   { trigger = "trigger", expand = "expand "},
 *   { trigger = "trigger", expand = "expand "},
 * }
 */
int config_snippets(lua_State* state);

/* ─────  Config  ───────────────────────────────────────────────────────────────── */

int  config_init(struct config* config, const char* filename);
void config_free(struct config* config);

/* ─────  Config Read  ──────────────────────────────────────────────────────────── */

int config_load(struct config* config, const char* filename);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */

