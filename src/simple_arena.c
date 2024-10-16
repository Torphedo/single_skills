#include <malloc.h>

#include "simple_arena.h"

simple_arena* simple_arena_create(uint32_t size) {
    simple_arena* arena = calloc(1, sizeof(*arena));
    arena->base = calloc(1, size);
    if (arena->base == NULL) {
        free(arena);
        return NULL;
    }
    else {
        return arena;
    }
}

void simple_arena_free(simple_arena* arena) {
    free(arena->base);
    arena->base = NULL;
    arena->pos = 0;
    free(arena);
}

void* simple_arena_alloc(simple_arena* arena, uint32_t size) {
    uint8_t* return_ptr = &arena->base[arena->pos];
    arena->pos += size;
    return return_ptr;
}
