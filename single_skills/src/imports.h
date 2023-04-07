#pragma once
#include <stdlib.h>
#include <stdint.h>

// This will have function pointers for all the functions we need from other plugins or the core loader.

void* (*plugin_get_proc_address)(const char* filename, const char* function_name) = NULL;
void (*plugin_cleanup)(void* plugin_handle) = NULL;

void* (*PHYSFS_openRead)(const char* filename) = NULL;
int64_t (*PHYSFS_fileLength)(void* handle) = NULL;
int64_t (*PHYSFS_readBytes)(void* handle, void* buffer, uint64_t len) = NULL;
int (*PHYSFS_close)(void* handle) = NULL;
char** (*PHYSFS_enumerateFiles)(const char* dir) = NULL;
void (*PHYSFS_freeList)(void* listVar) = NULL;

// Get function pointers for all the functions we need
void import_functions() {
    // Load functions we need from the core loader
    void* loader_core = GetModuleHandleA("pd_loader_core.dll");
    plugin_get_proc_address = (void*)GetProcAddress((void*)loader_core,"plugin_get_proc_address");
    plugin_cleanup = (void*)GetProcAddress((void*)loader_core, "plugin_cleanup");
    PHYSFS_close = (void*)GetProcAddress((void*)loader_core, "PHYSFS_close");
    PHYSFS_fileLength = (void*)GetProcAddress((void*)loader_core, "PHYSFS_fileLength");
    PHYSFS_readBytes = (void*)GetProcAddress((void*)loader_core, "PHYSFS_readBytes");
    PHYSFS_openRead = (void*)GetProcAddress((void*)loader_core, "PHYSFS_openRead");
    PHYSFS_enumerateFiles = (void*)GetProcAddress((void*)loader_core, "PHYSFS_enumerateFiles");
    PHYSFS_freeList = (void*)GetProcAddress((void*)loader_core, "PHYSFS_freeList");
}
