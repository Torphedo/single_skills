#pragma once

#include <stdint.h>

typedef struct {
    uint8_t data[0x90];
}skill;

typedef struct {
    uint32_t filesize;
    uint32_t unknown1;
    uint32_t skill_count;
    uint32_t unknown3;
    uint32_t unknown4;
    uint32_t skill_limiter; // The number of skills available to use. Default is 376 (0x176)
    uint8_t unknown5[136]; // This data hasn't been studied yet
    skill skill_array[753]; // 753 is the default size of the "skill_count" member.
}skill_storage;

// This starts at 0x34004
typedef struct {
    uint32_t text_size; // Size of the header, offset table, and all skill text
    uint32_t offset_count;
    uint32_t offset_table_size;
    uint32_t version_num; // Seems to be the game version number?
    uint32_t skill_limiter; // Seems to be a duplicate of the skill limiter?
}skill_text_header;

typedef struct {
    uint32_t index;
    uint32_t name_offset;
    uint32_t desc_offset;
}skill_text_offset;
