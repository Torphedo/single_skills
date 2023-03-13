#pragma once

#include <stdint.h>

typedef struct {
    uint8_t data[0x90];
}skill;

typedef struct {
    uint32_t filesize;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
    uint32_t skill_limiter; // The number of skills available to use. Default is 376 (0x176)
    uint8_t unknown5[136]; // This data hasn't been studied yet
    skill skill_array[751];
}skill_storage;
