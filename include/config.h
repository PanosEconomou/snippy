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

int config_snippet  (lua_State* state);
int config_snippets (lua_State* state);

/* ─────  Config Read  ──────────────────────────────────────────────────────────── */

int config_load (lua_State* state, const char* filename);

/* ──────────────────────────────────────────────────────────────────────────────── */

#endif

