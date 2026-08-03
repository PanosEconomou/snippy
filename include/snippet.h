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

/* ─────  Constants  ────────────────────────────────────────────────────────────── */

#define SNIPPET_NONE 0xFFFFFFFFu            /* max for uint32_t, to act as NULL     */

/* ─────  Snippet Types  ────────────────────────────────────────────────────────── */

enum snippet_type {
    SNIPPET_LITERAL,
    SNIPPET_LUA,
};

enum snippet_parse {
    SNIPPET_OK,
    SNIPPET_ERROR,
};

/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct snippet_trigger {
        uint32_t start;                     /* blob index for trigger start         */
        uint16_t length;                    /* length of trigger                    */
};

struct snippet_expansion {
    union {                                 /* only access one based on type        */
        struct {
            uint32_t start;                 /* blob index for literal snippet start */
            uint16_t length;                /* length of literal expansion          */
        } literal;

        int expand_lua;                     /* index to lua expand callback         */
    };
};

struct snippet {
    enum   snippet_type      type;          /* default: SNIPPET_LITERAL             */
    struct snippet_trigger   trigger;       /* what you need to trigger the snippet */

    struct snippet_expansion expansion;     /* different expansion based on type    */
};

struct snippet_triggers {                   /* storage for trigges                  */
    uint32_t       capacity;                /* max number of stored triggers        */
    uint32_t       write;                   /* next available trigger index         */
    unsigned char* triggers;                /* string with all triggers             */
};

struct snippet_expansions {                 /* storage for expansions               */
    uint32_t       capacity;                /* max number of stored expansions      */
    uint32_t       write;                   /* next available expansion index       */
    unsigned char* literal;                 /* string with all literal expansions   */
    int*           lua;                     /* lua callback id array                */
};

struct snippet_set {
    uint32_t        capacity;               /* max number of snippets               */
    uint32_t        write;                  /* next available snippet index         */
    struct snippet* snippets;               /* list of snippets                     */

    struct trie     trie;                   /* trie to search for them              */

    struct snippet_triggers   triggers;     /* trigger storage                      */
    struct snippet_expansions expansions;   /* expansion storage                    */

};

/* ─────  Snippet Set  ──────────────────────────────────────────────────────────── */

int  snippet_set_init(struct snippet_set* snippets, uint32_t capacity);
void snippet_set_free(struct snippet_set* snippets);

/* ─────  Snippet Accessors  ────────────────────────────────────────────────────── */

/* TODO: idk find some accessor methods I guess... */

/* ─────  Mutation Methods  ─────────────────────────────────────────────────────── */

int snippet_set_insert_literal(struct snippet_set*  snippets,
                               const unsigned char* trigger,   uint32_t trigger_length,
                               const unsigned char* expansion, uint32_t expansion_length);

uint32_t snippet_set_parse_ring(const struct snippet_set* snippets, 
                                const struct ring*        ring);

enum snippet_parse snippet_set_execute(const struct snippet_set* snippets, 
                                       uint32_t snippet);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
