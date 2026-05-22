cmake_minimum_required(VERSION 3.10)
project(luajit-decompiler-v2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories for source files
include_directories(${CMAKE_SOURCE_DIR})

# Find packages
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    main.cpp
    bytecode/bytecode.cpp
    bytecode/prototype.cpp
    ast/ast.cpp
    lua/lua.cpp
)

# Headers for IDE
set(HEADERS
    main.h
    bytecode/bytecode.h
    bytecode/prototype.h
    bytecode/constants.h
    bytecode/instructions.h
    ast/ast.h
    ast/function.h
    ast/building_blocks.h
    ast/conditionBuilder.h
    lua/lua.h
)

# Create executable
add_executable(luajit-decompiler-v2 ${SOURCES} ${HEADERS})

# Platform-specific settings
if(WIN32)
    target_compile_definitions(luajit-decompiler-v2 PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(luajit-decompiler-v2 PRIVATE Threads::Threads shlwapi)
else()
    target_link_libraries(luajit-decompiler-v2 PRIVATE Threads::Threads)
endif()

# Require unsigned char
target_compile_options(luajit-decompiler-v2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/J>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-funsigned-char>
)

# Install target
install(TARGETS luajit-decompiler-v2 DESTINATION bin)