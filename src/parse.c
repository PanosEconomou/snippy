/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   parse.c | Interface to create snippet sets from a config file.             ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include "parse.h"
#include "config.h"
#include "ring.h"
#include "snippet.h"
#include "trie.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <lua.h>

/* -----  Helpers  ──────────────────────────────────────────────────────────────── */

static int parse_init_snippet_set(struct config* config, 
                                  struct snippet_set* snippets)
{
    int return_code = 0;

    *snippets = (struct snippet_set){0};

    snippets->capacity   = config->snippet_count;

    snippets->write      = 0;

    snippets->list       = calloc(snippets->capacity, sizeof(struct snippet));
    if (!snippets->list) { return_code = -ENOMEM; goto error_list; }

    snippets->trie       = (struct trie){0};
    return_code = trie_init(&snippets->trie, config->trigger_bytes);
    if (return_code < 0) goto error_trie;

    snippets->triggers   = (struct snippet_triggers) {
        .capacity = config->trigger_bytes,
        .write    = 0,
        .blob     = calloc(config->trigger_bytes, sizeof(unsigned char)),
    };
    if (!snippets->triggers.blob) { return_code = -ENOMEM; goto error_triggers; }

    snippets->expansions = (struct snippet_expansions) {
        .capacity = config->literal_bytes,
        .write    = 0,
        .literal  = calloc(config->literal_count, sizeof(unsigned char)),
        .expander = (struct snippet_expander){0},
    };
    if (!snippets->expansions.literal) { return_code = -ENOMEM; goto error_literal; }

    return 0;

error_literal:
    free(snippets->triggers.blob);
error_triggers:
    trie_free(&snippets->trie);
error_trie:
    free(snippets->list);
error_list:
    *snippets = (struct snippet_set){0};
    return return_code;
}

static int parse_create_expander(struct config* config, 
                                 struct snippet_expander* expander)
{
    return 0;
}

static void cast_unsigned(lua_State* state, int idx, struct snippet_text* out)
{

    size_t length;
    const char* text = lua_tolstring(state, idx, &length);

    out->text = (unsigned char*) text;
    out->length = (uint16_t) length;
}

static int parse_table(struct config* config, struct snippet_set* snippets)
{


    lua_rawgeti(config->state, LUA_REGISTRYINDEX, config->table);   /* lua: [table] */

    struct snippet_data data    = {0};
    size_t table_size = lua_objlen(config->state, -1);
    for (size_t i = 1; i <= table_size; i++) {
        lua_rawgeti(config->state, -1, i);              /* [table][ith]             */
        int i_i = lua_gettop(config->state);

        lua_getfield(config->state, i_i, "trigger");    /* [table][ith][trigger]    */
        int i_trigger = lua_gettop(config->state);

        lua_getfield(config->state, i_i, "expansion");  /* ...[trigger][expansion]  */
        int i_expansion = lua_gettop(config->state);

        cast_unsigned(config->state, i_trigger, &data.trigger);

        int expansion_type = lua_type(config->state, i_expansion);
        switch (expansion_type) {
            case LUA_TSTRING:
                cast_unsigned(config->state, i_expansion, &data.expansion);
            case LUA_TFUNCTION:
                data.callback = 0;
        }


        lua_pop(config->state, 3);                      /* [table]                  */
    }

    return 0;
}

/* ─────  Useful Methods  ───────────────────────────────────────────────────────── */

int parse_config(struct config* config, struct snippet_set* snippets)
{
    int return_code = 0;
    return_code = parse_init_snippet_set(config, snippets);
    if (return_code < 0) goto error_snippets;

    return_code = parse_create_expander(config, &snippets->expansions.expander);
    if (return_code < 0) goto error_expander;

    return_code = parse_table(config, snippets);
    if (return_code < 0) goto error_table;

    return 0;

error_table:
error_expander:
error_snippets:
    snippet_set_free(snippets);
    return return_code;
}

