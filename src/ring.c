/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   ring.c | A circular buffer to quickly reference the keyboard stream.       ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#include "ring.h"

#include <stdlib.h>

/* ─────  Ring  ─────────────────────────────────────────────────────────────────── */

void ring_init(struct ring* ring)
{
    if (!ring) return;

    *ring = (struct ring){0};

    /* NOTE: the first push will pushi it to the start of the buffer */
    ring->head = RING_CAPACITY - 1;
}

/* ─────  Mutators  ─────────────────────────────────────────────────────────────── */


void ring_push(struct ring* ring, unsigned char thing)
{
    /* NOTE: the bitwize & operation acts like % */ 
    ring->head    = (ring->head + 1) & (RING_CAPACITY - 1);
    ring->content = ring->content < RING_CAPACITY ? (ring->content + 1) : RING_CAPACITY; 

    ring->data[ring->head] = thing;
}

void ring_pop(struct ring* ring)
{
    /* NOTE: the bitwize & operation acts like %, and when size_t -1
     *       becomes super large & correctly rounds it using two's complement */ 
    ring->head    = (ring->head - 1) & (RING_CAPACITY - 1);
    ring->content =  ring->content > 0? ring->content - 1 : 0; 
}

void ring_write(struct ring* ring, const unsigned char* data, size_t length) 
{
    for (size_t i = 0; i < length; i++) ring_push(ring, data[i]);
}

unsigned char ring_peek_back(const struct ring* ring, size_t spots)
{
    return ring->data[(ring->head - spots) & (RING_CAPACITY - 1)];
}

int ring_read(struct ring* ring, size_t length, unsigned char* buffer, size_t capacity)
{
    if (length > capacity)      length = capacity;
    if (length > ring->content) length = ring->content;

    for (size_t i = 0; i < length; i++) buffer[i] = ring_peek_back(ring, length - 1 - i);

    return (int)length;
}

/* ──────────────────────────────────────────────────────────────────────────────── */
