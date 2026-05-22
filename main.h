/*
 * Portable cross-platform LuaJIT Decompiler v2
 * Requires: C++20
 */

#pragma once

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

#define DEBUG_INFO __func__, __FILE__, __LINE__

constexpr const char* PROGRAM_NAME = "LuaJIT Decompiler v2";
constexpr uint64_t DOUBLE_SIGN = 0x8000000000000000ULL;
constexpr uint64_t DOUBLE_EXPONENT = 0x7FF0000000000000ULL;
constexpr uint64_t DOUBLE_FRACTION = 0x000FFFFFFFFFFFFFULL;
constexpr uint64_t DOUBLE_SPECIAL = DOUBLE_EXPONENT;
constexpr uint64_t DOUBLE_NEGATIVE_ZERO = DOUBLE_SIGN;

// Platform abstraction layer
namespace Platform {

// File handling
inline FILE* open_file(const std::string& path, const char* mode) {
#ifdef _WIN32
    return fopen(path.c_str(), mode);
#else
    return fopen(path.c_str(), mode);
#endif
}

inline int64_t get_file_size(FILE* file) {
    long pos = ftell(file);
    fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
    fseek(file, pos, SEEK_SET);
    return size;
}

inline size_t read_file(FILE* file, void* buffer, size_t size) {
    return fread(buffer, 1, size, file);
}

inline void close_file(FILE* file) {
    if (file) fclose(file);
}

// Directory operations
inline bool is_directory(const std::string& path) {
    return fs::is_directory(path);
}

inline bool path_exists(const std::string& path) {
    return fs::exists(path);
}

// Console output
inline void print(const std::string& message) {
    std::cout << message << '\n';
}

inline void print_no_newline(const std::string& message) {
    std::cout << message;
}

inline void flush_output() {
    std::cout.flush();
}

// Path manipulation
inline std::string get_filename(const std::string& path) {
    return fs::path(path).filename().string();
}

inline std::string get_extension(const std::string& path) {
    return fs::path(path).extension().string();
}

inline std::string get_directory(const std::string& path) {
    return fs::path(path).parent_path().string();
}

inline std::string join_path(const std::string& a, const std::string& b) {
    return (fs::path(a) / b).string();
}

inline std::string remove_extension(const std::string& path) {
    return fs::path(path).stem().string();
}

inline std::string get_executable_directory() {
    return fs::current_path().string();
}

// Create directory
inline bool create_directory(const std::string& path) {
    return fs::create_directories(path);
}

// Input
inline bool kbhit() {
#ifdef _WIN32
    return _kbhit() != 0;
#else
    // For Unix, use termios-based non-blocking input
    // Simplified: return false (will block)
    return false;
#endif
}

// Sleep (cross-platform)
inline void sleep_ms(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

} // namespace Platform

// Compatibility layer for old Windows API calls
namespace WinAPI_Compat {

// File handle wrapper
struct FileHandle {
    FILE* file = nullptr;
    int64_t size = 0;
    int64_t pos = 0;

    bool open(const std::string& path) {
        file = Platform::open_file(path, "rb");
        if (!file) return false;
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        pos = 0;
        return true;
    }

    void close() {
        if (file) {
            Platform::close_file(file);
            file = nullptr;
        }
    }

    size_t read(void* buffer, size_t size) {
        size_t read_size = Platform::read_file(file, buffer, size);
        pos += read_size;
        return read_size;
    }

    bool is_valid() const { return file != nullptr; }
    int64_t get_size() const { return size; }
    int64_t remaining() const { return size - pos; }
};

// Directory enumeration
struct DirectoryIterator {
    fs::recursive_directory_iterator iter;
    fs::recursive_directory_iterator end;
    bool initialized = false;

    DirectoryIterator(const std::string& path) : iter(path), end() {}

    bool has_next() const { return iter != end; }

    void next() { ++iter; }

    bool is_directory() const { return fs::is_directory(iter->path()); }

    std::string filename() const { return iter->path().filename().string(); }

    std::string path() const { return iter->path().string(); }
};

} // namespace WinAPI_Compat

// Backward compatibility - use these in existing code
namespace compat {
    // Simple assert
    inline void assert(bool condition, const std::string& message,
                       const std::string& filePath, const std::string& function,
                       const std::string& source, uint32_t line) {
        if (!condition) {
            throw std::runtime_error("Assertion failed in " + function + " at " + source + ":" +
                                     std::to_string(line) + "\n" + message + "\nFile: " + filePath);
        }
    }
}

void print(const std::string& message);
void print_progress_bar(double progress = 0, double total = 100);
void erase_progress_bar();
void assert(bool condition, const std::string& message, const std::string& filePath,
            const std::string& function, const std::string& source, uint32_t line);
std::string byte_to_string(uint8_t byte);

class Bytecode;
class Ast;
class Lua;

#include "bytecode/bytecode.h"
#include "ast/ast.h"
#include "lua/lua.h"