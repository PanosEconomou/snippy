/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   trie.h | inexing current snippets using a trie                             ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_TRIE_H
#define SNIPPY_TRIE_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include "ring.h"                           /* struct ring                          */

#include <stdint.h>                         /* uint32_t                             */

/* ─────  Constants  ────────────────────────────────────────────────────────────── */

#define TRIE_NONE    0xFFFFFFFFu            /* max for uint32_t, to act as NULL     */
#define SNIPPET_NONE 0xFFFFFFFFu            /* max for uint32_t, to act as NULL     */

/* ─────  Status Codes  ─────────────────────────────────────────────────────────── */

enum trie_status {
    TRIE_INSERTED,                      /* successful insertion in the trie         */
    TRIE_OVERWROTE,                     /* trie had to overwrite existing to insert */
    TRIE_NO_MEMORY,                     /* run out of memory to store new ndoes     */
};

/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct node {
    unsigned char character;                /* the character that leads here        */
    uint32_t      child;                    /* index to first child                 */
    uint32_t      sibling;                  /* index to next sibling                */
    uint32_t      snippet;                  /* index to snippet, or TRIE_NONE       */
};

struct trie {
    uint32_t        root;                   /* index to the trie root               */
    uint32_t        capacity;               /* how many nodes can you hold          */
    uint32_t        write;                  /* how many nodes have you used         */

    struct node*    nodes;                  /* node array                           */
};

/* ─────  Trie  ─────────────────────────────────────────────────────────────────── */

int  trie_init(struct trie* trie, uint32_t capacity);
void trie_free(struct trie* trie);


/* ─────  Mutation Methods  ─────────────────────────────────────────────────────── */

enum trie_status trie_insert(struct trie*          trie, 
                             uint32_t              snippet, 
                             const unsigned char*  trigger, 
                             size_t                length);

uint32_t trie_parse_ring(const struct trie* trie, const struct ring* ring);

#endif 

/* ──────────────────────────────────────────────────────────────────────────────── */
