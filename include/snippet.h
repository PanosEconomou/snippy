/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   snippet.h | Loads stores and processes snippets.                           ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_SNIPPET_H
#define SNIPPY_SNIPPET_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include "trie.h"
#include "ring.h"
#include <stdint.h>

/* ─────  Snippet Types  ────────────────────────────────────────────────────────── */

enum snippet_type {
    SNIPPET_STATIC,
    SNIPPET_LUA,
};

/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct snippet {
    enum snippet_type type;                 /* Default: SNIPPET_STATIC              */

    struct {
        uint32_t start;                     /* blob index for trigger start         */
        uint16_t length;                    /* length of trigger                    */
    } trigger;

    union {                                 /* only access one based on type        */
        struct {
            uint32_t start;                 /* blob index for static snippet start  */
            uint16_t length;                /* length of static expansion           */
            
        } expand_static;

        int expand_lua;                     /* index to lua expand callback         */
    };
};

struct snippet_set {
    struct snippet* snippets;                     /* list of snippets               */
    struct trie*    trie;                         /* trie to search for them        */

    unsigned char* triggers_blob;                 /* string with all triggers       */

    unsigned char* expansions_static_blob;        /* string with all expands        */
    int*           expansions_lua;                /* lua callback id array          */
};

/* ─────  Snippet Set  ──────────────────────────────────────────────────────────── */

int  snippet_set_init();
void snippet_set_free();

/* ─────  Mutation Methods  ─────────────────────────────────────────────────────── */

int snippet_set_insert(struct snippet_set* snippets, const struct snippet* snippet);


#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
