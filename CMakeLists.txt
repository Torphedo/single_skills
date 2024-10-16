cmake_minimum_required(VERSION 3.16)
project(single_skills
	VERSION 1.0.0
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99) # Our code is C99
set(CMAKE_SHARED_LIBRARY_PREFIX "") # Remove "lib" prefix from the output filename
if (MSVC)
	# Statically link C runtime. Comment this out to get a tiny DLL.
	# On my machine the final DLL goes from ~150KiB to 21KiB.
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Compile MinHook
add_subdirectory(ext/minhook)

# Add pd_loader_core import library as a library. This allows us to get linked to
# pd_loader_core.dll at runtime without building it from source.
add_library(pd_loader_core SHARED IMPORTED)
set_target_properties(pd_loader_core PROPERTIES IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/ext/pd_loader_core/libpd_loader_core.dll.a")

# Compile our DLL
add_library(single_skills SHARED
	"src/dll_main.c"
	"src/simple_arena.c"
)

# Add the MinHook library to our DLL
target_link_libraries(single_skills PRIVATE minhook pd_loader_core)

# This lets us include MinHook.h without the whole relative path.
target_include_directories(single_skills PRIVATE "ext/minhook/include" "ext/pd_loader_core/include")
