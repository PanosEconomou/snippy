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

#include <stdint.h>

/* ─────  Constants  ────────────────────────────────────────────────────────────── */

#define SNIPPET_NONE          0xFFFFFFFFu   /* max for uint32_t, to act as NULL     */
#define SNIPPET_EXPANSION_MAX 4000          /* max for wayland message is 4096 B    */

/* ─────  Snippet Types  ────────────────────────────────────────────────────────── */

enum snippet_type {
    SNIPPET_LITERAL,
    SNIPPET_CALLBACK,
};

/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct snippet_text {
    const unsigned char* text;              /* string                               */
    uint16_t       length;                  /* string length                        */
};

struct snippet_data {                       /* explicit storage for new snippets    */
    enum snippet_type type;                 /* what kind of snippet                 */
    struct snippet_text trigger;            /* trigger string                       */
    union {                                 /* only store one based on type         */
        struct snippet_text expansion;      /* expansion text if SNIPPET_LITERAL    */
        int callback;                       /* reference to expander callback       */
    };
};

struct snippet_trigger {
    uint32_t start;                         /* blob index for trigger start         */
    uint16_t length;                        /* length of trigger                    */
};

struct snippet_expander {                   /* handles expansion of callback snips  */
    void* context;                          /* data needed to facilitate callback   */
    int (*expand)(void*          context,   /* data needed to facilitate callback   */
                  int            reference, /* which snippet are we expanding?      */
                  unsigned char* out,       /* text to write if any                 */
                  size_t         length);   /* size of output buffer                */
};

struct snippet_expansion {
    union {                                 /* only store one based on type         */
        struct {
            uint32_t start;                 /* blob index for literal snippet start */
            uint16_t length;                /* length of literal expansion          */
        } literal;

        int callback;                       /* reference to expander callback       */
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
    unsigned char* blob;                    /* string with all triggers             */
};

struct snippet_expansions {                 /* storage for expansions               */
    uint32_t       capacity;                /* max number of stored expansions      */
    uint32_t       write;                   /* next available expansion index       */
    unsigned char* literal;                 /* string with all literal expansions   */
    struct snippet_expander expander;       /* how to expand callback snippets      */
};

struct snippet_set {
    uint32_t        capacity;               /* max number of snippets               */
    uint32_t        write;                  /* next available snippet index         */
    struct snippet* list;                   /* list of snippets                     */

    struct trie     trie;                   /* trie to search for them              */

    struct snippet_triggers   triggers;     /* trigger storage                      */
    struct snippet_expansions expansions;   /* expansion storage                    */

};

/* ─────  Snippet Set  ──────────────────────────────────────────────────────────── */

/* don't forget to use parse.h to parse a config into a snippet_set */
void snippet_triggers_free(struct snippet_triggers* triggers);
void snippet_expansions_free(struct snippet_expansions* expansions);
void snippet_set_free(struct snippet_set* snippets);

/* ─────  Snippet Accessors  ────────────────────────────────────────────────────── */

int snippet_set_insert(struct snippet_set*  snippets, struct snippet_data* data);

/* ─────  Useful Methods  ───────────────────────────────────────────────────────── */

int snippet_set_expand(const struct snippet_set* snippets, uint32_t snippet,
                        unsigned char* buffer, size_t length);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
