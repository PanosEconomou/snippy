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

#include <lua.h>                                    /* lua_State                    */
#include <lauxlib.h>                                /* luaL_Reg                     */

/* ─────  Config Library  ───────────────────────────────────────────────────────── */

/*
 * In config.lua call as:
 * snippet { trigger = "trigger", expand = "expand "} or
 * snippet (
 *   { trigger = "trigger", expand = "expand "},
 *   { trigger = "trigger", expand = "expand "}
 * )
 */
int config_snippet  (lua_State* state);

/*
 * In config.lua call as:
 * snippets {
 *   { trigger = "trigger", expand = "expand "},
 *   { trigger = "trigger", expand = "expand "},
 * }
 */
int config_snippets (lua_State* state);

/* ─────  Config Read  ──────────────────────────────────────────────────────────── */

int config_load (lua_State* state, const char* filename);

/* ──────────────────────────────────────────────────────────────────────────────── */

#endif

