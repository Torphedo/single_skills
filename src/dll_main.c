#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <MinHook.h> // Also includes Windows.h
#include <pd_loader_core.h>

#include "structures.h"
#include "simple_arena.h"
#include "path.h"

// Once we finish loading in all the modded skill data,
// the plugin's job is done, and we can unload it.
static bool ready_to_shut_down = false;

// Enable this to dump the modded gsdata to a valid file. For some reason,
// the text offsets work differently at runtime than they do for a static
// gsdata file. So by enabling this you'll get a valid file, but broken
// text in-game.
const bool dump_to_file = false;

typedef void (*LOAD_SKILLS)(void* unknown_ptr, int* unknown_int_ptr, int unknown_int);
LOAD_SKILLS address_load_skills = NULL; // Address of the function that loads skills
LOAD_SKILLS original_load_skills = NULL; // After we hook the function, calling this will run the original code.

skill_storage* storage = NULL;


void hook_load_skills(void* unknown_ptr, int* unknown_int_ptr, int unknown_int) {
    // Call the original code. We don't know or care what these parameters are.
    printf("single_skills: Loading skill data...\n");
    original_load_skills(unknown_ptr, unknown_int_ptr, unknown_int);

    char path[MAX_PATH] = {0};
    uint32_t patched_skill_count = 0;
    uint32_t patched_text_count = 0;

    // Loop through all files in the "skills" folder, and write them to the skill array if they are the correct size.
    char** skills_file_list = PHYSFS_enumerateFiles("/skills/");
    for (char** i = skills_file_list; *i != NULL; i++) {
        sprintf(path, "/skills/%s", *i);
        void* skill_file = PHYSFS_openRead(path);
        if (skill_file != NULL) {
            if (PHYSFS_fileLength(skill_file) == sizeof(skill)) {
                skill buffer = {0};

                PHYSFS_readBytes(skill_file, &buffer, sizeof(skill));

                // The 2 bytes at 0x8 store the skill ID. Need to label this in the struct.
                uint16_t current_skill_id = *(uint16_t*) &buffer.data[0x8];
                storage->skill_array[current_skill_id] = buffer;

                PHYSFS_close(skill_file);
                printf("single_skills: Loaded %s.\n", *i);
                patched_skill_count++;
            }
        }
    }
    PHYSFS_freeList(skills_file_list);

    printf("single_skills: Patched in %d modded skill(s).\n", patched_skill_count);

    /* All the following code handles rebuilding the skill text structures in memory. These
     * consist of a header, followed by a large array of offsets ("offset table") used to
     * determine length, followed by all the strings.
     * At least in the current implementation, this is kind of expensive. To optimize it,
     * we skip the entire process if there are no skill text files in the text folder.
     */

    uint32_t skill_text_file_count = 0;
    char** text_file_list = PHYSFS_enumerateFiles("/skills/text/");
    for (char** i = text_file_list; *i != NULL; i++) {
        if (path_has_extension(*i, ".name.txt") || path_has_extension(*i, ".desc.txt")){
            skill_text_file_count++;
        }
    }
    PHYSFS_freeList(text_file_list);

    if (skill_text_file_count == 0) {
        printf("single_skills: No skill text files, skipping a full text rebuild.\n");
    }
    else {
        skill_text_header *header = (skill_text_header *) ((uint8_t *) storage + 0x34004);
        skill_text_offset *offsets = (skill_text_offset *) ((uint8_t *) header + sizeof(skill_text_header));
        simple_arena *arena = simple_arena_create(0x10000);
        char **string_array = simple_arena_alloc(arena, sizeof(char *) * header->offset_count * 2);
        uint32_t skill_id = 0; // This is updated every other loop

        // Copy all strings to a new block of memory
        for (uint32_t i = 0; i < header->offset_count * 2; i += 2) {
            uint32_t name_size = offsets[skill_id].desc_offset - offsets[skill_id].name_offset;
            uint32_t desc_size = 0;
            if (skill_id + 1 == header->offset_count) {
                break; // Stops a crash, needs a proper fix later.
                desc_size = header->text_size - header->offset_table_size;
            } else {
                desc_size = offsets[skill_id + 1].name_offset + sizeof(skill_text_offset);
            }
            desc_size -= offsets[skill_id].desc_offset;
            string_array[i] = simple_arena_alloc(arena, name_size);
            string_array[i + 1] = simple_arena_alloc(arena, desc_size);

            memcpy(string_array[i], (char *) &offsets[skill_id] + offsets[skill_id].name_offset, name_size);
            memcpy(string_array[i + 1], (char *) &offsets[skill_id] + offsets[skill_id].desc_offset, desc_size);

            skill_id += 1;
        }

        // Delete the offset table and text
        memset(offsets, 0, header->text_size - sizeof(*header));
        skill_id = 0;
        char *text_base = (char *) offsets + header->offset_table_size - sizeof(*header);
        char *text_pos = text_base;

        // Rebuild offset table and copy back strings (from files if present)
        for (uint32_t i = 0; i < header->offset_count * 2; i += 2) {
            if (skill_id + 1 == header->offset_count) {
                break;
            }
            offsets[skill_id].index = skill_id + 1;

            sprintf(path, "/skills/text/%d.name.txt", skill_id);
            void *name_txt = PHYSFS_openRead(path);
            sprintf(path, "/skills/text/%d.desc.txt", skill_id);
            void *desc_txt = PHYSFS_openRead(path);
            memset(path, 0, MAX_PATH); // Clear string so it doesn't leak into the name

            uint32_t name_size = 0;
            uint32_t desc_size = 0;

            if (name_txt != NULL) {
                patched_text_count++;
                PHYSFS_readBytes(name_txt, path, PHYSFS_fileLength(name_txt));
                PHYSFS_close(name_txt);
                name_size = strlen(path) + 1;
                memcpy(text_pos, path, name_size);
                memset(path, 0, MAX_PATH); // Clear string so it doesn't leak into the description
            } else {
                name_size = strlen(string_array[i]) + 1;
                memcpy(text_pos, string_array[i], name_size);
            }

            if (dump_to_file) {
                offsets[skill_id].name_offset = (text_pos - text_base);
            }
            else {
                offsets[skill_id].name_offset = (text_pos - (char *) &offsets[skill_id]);
            }
            text_pos += name_size;

            if (desc_txt != NULL) {
                patched_text_count++;
                PHYSFS_readBytes(desc_txt, path, PHYSFS_fileLength(desc_txt));
                PHYSFS_close(desc_txt);
                desc_size = strlen(path) + 1;
                memcpy(text_pos, path, desc_size);
                memset(path, 0, MAX_PATH); // Clear path so it doesn't leak into the next loop
            } else {
                desc_size = strlen(string_array[i]) + 1;
                memcpy(text_pos, string_array[i], desc_size);
            }

            if (dump_to_file) {
                offsets[skill_id].desc_offset = (text_pos - text_base);
            }
            else {
                offsets[skill_id].desc_offset = (text_pos - (char *) &offsets[skill_id]);
            }
            text_pos += desc_size;

            skill_id++;
        }

        header->text_size = (uintptr_t)text_pos - (uintptr_t)header;

        simple_arena_free(arena);
    }
    printf("single_skills: Patched in %d skill text file(s).\n", patched_text_count);

    if (dump_to_file) {
        void *gsdata_out = PHYSFS_openWrite("gsdata_en.dat");
        PHYSFS_writeBytes(gsdata_out, storage, storage->filesize);
        PHYSFS_close(gsdata_out);
    }

    ready_to_shut_down = true;
}

void __stdcall plugin_thread(void* plugin_handle) {
    const uint8_t* pduwp = (uint8_t*)GetModuleHandle("PDUWP.exe");
    storage = (skill_storage*) (pduwp + 0x4C5240);

    address_load_skills = (LOAD_SKILLS) (pduwp + 0x17E0A0); // Address of the function that loads skills

    if (MH_Initialize() != MH_OK) {
        printf("single_skills: Failed to start MinHook.\n");
    }
    if (MH_CreateHook(address_load_skills, &hook_load_skills, (void**) &original_load_skills) != MH_OK) {
        printf("single_skills: Failed to create hook.\n");
    }
    if (MH_EnableHook(address_load_skills) != MH_OK) {
        printf("single_skills: Failed to enable hook.\n");
    }


    // Wait until we load the modded skill data to exit
	while (!ready_to_shut_down) {
		Sleep(100);
	}
    // Sleep another millisecond to make sure we don't accidentally remove the hook before it fully returns
    Sleep(1);
    MH_RemoveHook(address_load_skills);
    MH_Uninitialize();
}

__declspec(dllexport) int32_t __stdcall DllMain(void* plugin_handle, uint32_t reason, void* reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        // Disable DLL notifications for new threads starting up, because we have no need to run special code here.
        DisableThreadLibraryCalls(plugin_handle);

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) plugin_thread, plugin_handle, 0, NULL);
    }
    return TRUE;
}
