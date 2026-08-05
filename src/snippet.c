/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   snippet.c | Loads stores and processes snippets.                           ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#include "snippet.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* ─────  Snippet Set  ──────────────────────────────────────────────────────────── */

void snippet_triggers_free(struct snippet_triggers* triggers)
{
    if (triggers->blob) free(triggers->blob);
    *triggers = (struct snippet_triggers){0};
}

void snippet_expansions_free(struct snippet_expansions* expansions)
{
    if (expansions->literal) free(expansions->literal);
    *expansions = (struct snippet_expansions){0};
}

void snippet_set_free(struct snippet_set* snippets)
{
    if (snippets->list) free(snippets->list);

    trie_free(&snippets->trie);
    snippet_triggers_free(&snippets->triggers);
    snippet_expansions_free(&snippets->expansions);

    *snippets = (struct snippet_set){0};
}

/* ─────  Snippet Accessors  ────────────────────────────────────────────────────── */

int snippet_set_insert(struct snippet_set*  snippets, struct snippet_data* data)
{
    if (snippets->capacity <= snippets->write) return -ENOMEM;

    if (snippets->triggers.capacity < 
        snippets->triggers.write + data->trigger.length) return -ENOMEM;

    struct snippet snip = {
        .type    = data->type,
        .trigger = {
            .start  = snippets->triggers.write,
            .length = data->trigger.length,
        },
    };
    
    memcpy(snippets->triggers.blob + snippets->triggers.write, 
           data->trigger.text,
           data->trigger.length);

    snippets->triggers.write += data->trigger.length;

    switch (snip.type) {
        case SNIPPET_LITERAL:
        if (snippets->expansions.capacity < 
                snippets->expansions.write + data->expansion.length) {

                snippets->triggers.write -= data->trigger.length;
                return -ENOMEM;
            }

        snip.expansion.literal.start  = snippets->expansions.write;
        snip.expansion.literal.length = data->expansion.length;

        memcpy(snippets->expansions.literal + snippets->expansions.write, 
               data->expansion.text,
               data->expansion.length);

        snippets->expansions.write += data->expansion.length;

        break;

        case SNIPPET_CALLBACK:
        snip.expansion.callback       = data->callback;

        break;
    }

    snippets->list[snippets->write] = snip;

    int return_code = trie_insert(&snippets->trie, snippets->write, 
                            data->trigger.text, data->trigger.length);

    if (return_code < 0) {
        snippets->triggers.write -= data->trigger.length;
        if (snip.type == SNIPPET_LITERAL) 
            snippets->expansions.write -= data->expansion.length;
        return return_code;
    }
    
    return snippets->write++;
}

/* ─────  Useful Methods  ───────────────────────────────────────────────────────── */

int snippet_set_expand(const struct snippet_set* snippets, 
                            uint32_t snippet, unsigned char* buffer, size_t length)
{
    size_t output_length = 0;
    struct snippet snip = snippets->list[snippet];
    switch(snip.type) {
        case SNIPPET_LITERAL:
        output_length = snip.expansion.literal.length;
        if (output_length > length) output_length = length;
        memcpy(buffer, 
               snippets->expansions.literal + snip.expansion.literal.start, 
               output_length);

        break;

        case SNIPPET_CALLBACK:
        output_length = snippets->expansions.expander.expand(
                                    snippets->expansions.expander.context,
                                    snip.expansion.callback, buffer, length);
        break;
    }

    return (int)output_length;
}


/* ──────────────────────────────────────────────────────────────────────────────── */
