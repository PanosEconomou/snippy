/* ╔══════════════════════════════════════════════════════════════════════════════╗ *
 * ║                                                                              ║ *
 * ║   ┏━┛┏━ ┛┏━┃┏━┃┃ ┃                                                           ║ *
 * ║   ━━┃┃ ┃┃┏━┛┏━┛━┏┛                                                           ║ *
 * ║   ━━┛┛ ┛┛┛  ┛   ┛                                                            ║ *
 * ║   ring.h | A circular buffer to quickly reference the keyboard stream.       ║ *
 * ║                                                                              ║ *
 * ╚══════════════════════════════════════════════════════════════════════════════╝ */

#ifndef SNIPPY_RING_H
#define SNIPPY_RING_H

/* -----  Headers  ──────────────────────────────────────────────────────────────── */

#include <stddef.h>                         /* size_t                               */

/* ─────  Constants  ────────────────────────────────────────────────────────────── */

#define RING_CAPACITY 256                   /* FTLOG please keep as power of 2.     */

/* ─────  Structures   ──────────────────────────────────────────────────────────── */

struct ring {
    char   data[RING_CAPACITY];             /* Data character buffer                */ 
    size_t head;                            /* Position of the last character       */
    size_t content;                         /* How much of the data is not garbage  */
};

/* ─────  Ring  ─────────────────────────────────────────────────────────────────── */

void ring_init (struct ring* ring);

/* ─────  Mutators  ─────────────────────────────────────────────────────────────── */


void ring_push (struct ring* ring, char thing);
void ring_pop  (struct ring* ring);
void ring_write(struct ring* ring, const char* data, size_t length);
int  ring_read (struct ring* ring, size_t length, char* buffer, size_t capacity);

#endif

/* ──────────────────────────────────────────────────────────────────────────────── */
