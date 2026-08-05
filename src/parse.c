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
#include <string.h>

#include <lua.h>

/* -----  Helpers  ──────────────────────────────────────────────────────────────── */

static int parse_snippet_set_init(struct config* config, 
                                  struct snippet_set* snippets)
{
    if (config->snippet_count == 0) return -EINVAL;

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
        .literal  = calloc(config->literal_bytes, sizeof(unsigned char)),
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

static int parse_read_text(lua_State* state, int idx, struct snippet_text* out)
{

    size_t length;
    const char* text = lua_tolstring(state, idx, &length);
    if (!text) return - EINVAL;

    out->text   = (const unsigned char*) text;
    out->length = (uint16_t) length;

    return (int) length;
}

static int parse_lua_expander(void* context, int reference, unsigned char* out, 
                              size_t length )
{
    lua_State* state = context;

    lua_rawgeti(state, LUA_REGISTRYINDEX, reference);

    if (lua_pcall(state, 0, 1, 0) != 0) {
        lua_pop(state, 1);
        return -EINVAL;
    }

    struct snippet_text data = {0};
    int return_code = parse_read_text(state, -1, &data);
    if (return_code < 0){
        lua_pop(state, 1);
        return return_code;
    } 

    if (data.length > length) data.length = length;
    memcpy(out, data.text, data.length);

    lua_pop(state, 1);

    return (int)data.length;
}

static int parse_create_expander(struct config* config, 
                                 struct snippet_expander* expander)
{
    expander->context = config->state;
    expander->expand  = parse_lua_expander;
    return 0;
}

static int parse_table(struct config* config, struct snippet_set* snippets)
{
    int return_code = 0;

    lua_rawgeti(config->state, LUA_REGISTRYINDEX, config->table);   /* lua: [table] */

    size_t table_size = lua_objlen(config->state, -1);
    for (size_t i = 1; i <= table_size; i++) {

        lua_rawgeti(config->state, -1, i);              /* [table][ith]             */
        int i_i = lua_gettop(config->state);

        lua_getfield(config->state, i_i, "trigger");    /* [table][ith][trigger]    */
        int i_trigger = lua_gettop(config->state);

        lua_getfield(config->state, i_i, "expand");     /* ..[ith][trigger][expand] */
        int i_expansion = lua_gettop(config->state);

        struct snippet_data data = {0};

        int error = parse_read_text(config->state, i_trigger, &data.trigger);
        if (error < 0) {
            lua_pop(config->state, 3);
            return_code = error;
            break;
        }

        int expansion_type = lua_type(config->state, i_expansion);
        switch (expansion_type) {
            case LUA_TSTRING:
                data.type = SNIPPET_LITERAL;

                error = parse_read_text(config->state, i_expansion, &data.expansion);
                if (error < 0) {
                    lua_pop(config->state, 3);
                    return_code = error;
                }
                break;

            case LUA_TFUNCTION:
                data.type = SNIPPET_CALLBACK;

                lua_pushvalue(config->state, -1);       /* ...[expand][expand]      */
                data.callback = luaL_ref(config->state, LUA_REGISTRYINDEX);

                break;
        }                                               /* ...[trigger][expand]     */
        if (return_code < 0) break;

        return_code = snippet_set_insert(snippets, &data);

        lua_pop(config->state, 3);                      /* [table]                  */

        if (return_code < 0) {
            if (data.type == SNIPPET_CALLBACK) {
                luaL_unref(config->state, LUA_REGISTRYINDEX, data.callback);
            }
            break;
        }
    }

    lua_pop(config->state, 1);                          /*                          */

    return return_code;
}

/* ─────  Useful Methods  ───────────────────────────────────────────────────────── */

int parse_config(struct config* config, struct snippet_set* snippets)
{
    int return_code = 0;
    return_code = parse_snippet_set_init(config, snippets);
    if (return_code < 0) goto error;

    return_code = parse_create_expander(config, &snippets->expansions.expander);
    if (return_code < 0) goto error;

    return_code = parse_table(config, snippets);
    if (return_code < 0) goto error;

    return 0;

error:
    snippet_set_free(snippets);
    return return_code;
}

