#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Cleanup and virtual filesystem functions for Phantom Dust plugins */

/* Example of how to call these functions:
 * plugin_api api = *remote_api;
 * api.PHYSFS_openRead("/skills/text/47.name.txt");
 *
 * For detailed documentation of PHYSFS_* functions, visit http://www.icculus.org/physfs/docs/html/physfs_8h.html
 */

typedef struct {
    void (*plugin_cleanup)(void* dll_handle);
    bool (*PHYSFS_exists)(const char* path);
    void* (*PHYSFS_openRead)(const char* path);
    void* (*PHYSFS_openWrite)(const char* path);
    void* (*PHYSFS_openAppend)(const char* path);
    int64_t (*PHYSFS_readBytes)(void* handle, void* buffer, uint64_t size);
    int64_t (*PHYSFS_writeBytes)(void* handle, const void* buffer, uint64_t size);
    int64_t (*PHYSFS_fileLength)(void* handle);
    int64_t (*PHYSFS_tell)(void* handle);
    int32_t (*PHYSFS_seek)(void* handle, uint64_t pos);
    int (*PHYSFS_close)(void* handle);
    char** (*PHYSFS_enumerateFiles)(const char* directory);
    void (*PHYSFS_freeList)(void* list);
}plugin_api;
