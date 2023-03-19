add_library(single_skills SHARED
	"src/dll_main.c"
	"src/simple_arena.c"
)

# Add the MinHook library to our DLL
target_link_libraries(single_skills PRIVATE minhook)

# This lets us include MinHook.h without the whole relative path.
target_include_directories(single_skills PRIVATE "../ext/minhook/include")
