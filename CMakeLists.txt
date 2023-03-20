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

# Compile MinHook first using the CMakeLists.txt in the "ext/minhook" folder
add_subdirectory(ext/minhook)

# Compile our DLL using the CMakeLists.txt in the "single_skills" folder
add_subdirectory(single_skills)
