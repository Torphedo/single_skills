#pragma once

// Simple arena allocator we use the make a string array with a single memory allocation

#include <stdint.h>

typedef struct {
    uint8_t* base; // Pointer to the start of the allocation; this can be passed to free()
    uintptr_t pos;
}simple_arena;


// Create a new arena of the specified size and return a pointer to it
simple_arena* simple_arena_create(uint32_t size);

// Free all the memory in the specified arena
void simple_arena_free(simple_arena* arena);

// Returns a pointer to a new allocation and increments the arena position
void* simple_arena_alloc(simple_arena* arena, uint32_t size);

