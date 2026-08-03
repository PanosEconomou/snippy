/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   trie.c | inexing current snippets using a trie                             ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include "trie.h"
#include "snippet.h"
#include "ring.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* ─────  Trie  ─────────────────────────────────────────────────────────────────── */

/* Initializes the memory the trie requires */
int trie_init(struct trie* trie, uint32_t capacity)
{
    *trie = (struct trie){0};

    if (!capacity) return -EINVAL;
    trie->nodes = calloc(capacity, sizeof(struct node));

    if (!trie->nodes) return -ENOMEM;

    trie->capacity          = capacity;
    trie->root              = 0;
    trie->write             = 1;
    trie->nodes[trie->root] = (struct node){
        .character  = 0,
        .child      = TRIE_NONE,
        .sibling    = TRIE_NONE,
        .snippet    = SNIPPET_NONE
    };

    return 0;
}

void trie_free(struct trie* trie)
{
    free(trie->nodes);
    *trie = (struct trie){0};
}


/* ─────  Node  ─────────────────────────────────────────────────────────────────── */

static uint32_t node_alloc(struct trie* trie)
{
    if (trie->write >= trie->capacity) return TRIE_NONE;

    return trie->write++;
}

/* ─────  Traversal Methods  ────────────────────────────────────────────────────── */

static uint32_t trie_find_last_sibling(struct trie* trie, uint32_t node)
{
    while (trie->nodes[node].sibling != TRIE_NONE) {
        node = trie->nodes[node].sibling;
    }

    return node;
}

/* NOTE: returns TRIE_NONE if there are no children */
static uint32_t trie_find_last_child(struct trie* trie, uint32_t parent)
{
    uint32_t child = trie->nodes[parent].child;
    if (child == TRIE_NONE) return TRIE_NONE;

    return trie_find_last_sibling(trie, child);
}

static uint32_t trie_find_child(const struct trie* trie, uint32_t parent, 
                                unsigned char character)
{
    uint32_t current_child = trie->nodes[parent].child;
    if (current_child == TRIE_NONE) return TRIE_NONE;

    do {
        if (trie->nodes[current_child].character == character) {
            return current_child;
        }

        current_child = trie->nodes[current_child].sibling;

    } while (current_child != TRIE_NONE);

    return TRIE_NONE;
}

static uint32_t trie_add_child(struct trie* trie, uint32_t parent, 
                                unsigned char character)
{
    uint32_t child = trie_find_child(trie, parent, character);
    if (child != TRIE_NONE) return child;

    uint32_t new_node = node_alloc(trie);
    if (new_node == TRIE_NONE) return TRIE_NONE;

    trie->nodes[new_node] = (struct node){
        .character = character,
        .child     = TRIE_NONE,
        .sibling   = TRIE_NONE,
        .snippet   = SNIPPET_NONE,
    };

    child = trie_find_last_child(trie, parent);

    if (child == TRIE_NONE) trie->nodes[parent].child = new_node;
    else trie->nodes[child].sibling = new_node;

    return new_node;
}

/* ─────  Mutation Methods  ─────────────────────────────────────────────────────── */

enum trie_status trie_insert(struct trie* trie, uint32_t snippet, 
                             const unsigned char* trigger, size_t length)
{
    uint32_t last_node = trie->root;
    for (size_t i = 0; i < length; i++) {

        last_node = trie_add_child(trie, last_node, trigger[length - 1 - i]);
        if (last_node == TRIE_NONE) return TRIE_NO_MEMORY;

    }

    enum trie_status code = TRIE_INSERTED;

    /* NOTE: if two snippets terminate at the same node the last one is registered */
    if (trie->nodes[last_node].snippet != SNIPPET_NONE) code = TRIE_OVERWROTE;

    trie->nodes[last_node].snippet = snippet;

    return code;

}

/* NOTE: right now it selects the longest snippet it can find */
uint32_t trie_parse_ring(const struct trie* trie, const struct ring* ring)
{
    uint32_t best_match = SNIPPET_NONE;
    uint32_t last_node  = trie->root;

    for (size_t i = 0; i < ring->content; i++) {

        last_node = trie_find_child(trie, last_node, ring_peek_back(ring, i));

        if (last_node == TRIE_NONE) break;

        if (trie->nodes[last_node].snippet != SNIPPET_NONE) {
            best_match = trie->nodes[last_node].snippet;
        }
    }

    return best_match;
}

/* ──────────────────────────────────────────────────────────────────────────────── */
