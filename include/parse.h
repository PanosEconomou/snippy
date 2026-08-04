/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   parse.h | Interface to create snippet sets from a config file.             ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_PARSE_H
#define SNIPPY_PARSE_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include "config.h"
#include "snippet.h"

#include <stdint.h>

/* ─────  Useful Methods  ───────────────────────────────────────────────────────── */

/*
 * This is supposed to
 *  - create a struct snippet_expander for lua callbacks
 *  - construct the blobs for triggers and expansions
 *  - initialize a struct snippet_set
 *  - TODO: set any other config variables
 *
 * Ideally one would call this instead of a snippet_set_init function
 * but after they have done config_load(config, filename);
 */
int parse_config(struct config* config, struct snippet_set* snippets);

/*
 * Reads stuff from the ring buffer and finds if a snippet is pressed
 *
 * @return a snippet index or SNIPPET_NONE
 */
uint32_t parse_keystrokes(const struct snippet_set* snippets, const struct ring* ring);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
