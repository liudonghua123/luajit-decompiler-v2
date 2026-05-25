## LuaJIT Decompiler v2

*LuaJIT Decompiler v2* is a replacement tool for the old and now mostly defunct python decompiler.
The project fixes all of the bugs and quirks the python decompiler had while also offering
full support for gotos and stripped bytecode including locals and upvalues.

## Features

- Full LuaJIT bytecode 1 and 2 support
- Goto statement support
- Stripped bytecode support (locals and upvalues)
- Cross-platform (Windows, Linux, macOS)
- WASI / wasm32-wasi target support
- Batch decompilation (directories)

## Building

### Prerequisites

- CMake 3.10 or higher
- C++20 compatible compiler (MSVC, GCC, Clang)

### Build Instructions

**Windows (Visual Studio):**
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

**Linux/macOS:**
```bash
cmake -B build -G "Unix Makefiles"
cmake --build build --config Release
```

**WASI (wasm32-wasi):**
```bash
cmake -B build -DBUILD_WASI=ON -G "Unix Makefiles"
cmake --build build --config Release
```

The executable will be at `build/bin/Release/luajit-decompiler-v2.exe` (Windows) or `build/bin/luajit-decompiler-v2` (Unix/WASI).

## Usage

```bash
# Basic usage - decompile a single file
luajit-decompiler-v2 input.lua

# Specify output directory
luajit-decompiler-v2 input.lua --output output/

# Batch decompile all files in a directory
luajit-decompiler-v2 ./bytecode_files/

# Only decompile .luac files
luajit-decompiler-v2 ./bytecode_files/ --extension .luac

# Read bytecode from stdin and write decompiled Lua to stdout
cat input.luac | luajit-decompiler-v2 -

# Explicit stdout output
cat input.luac | luajit-decompiler-v2 - --output -

# Silent mode (skip files that fail to decompile)
luajit-decompiler-v2 input.lua --silent_assertions

# Force overwrite existing files
luajit-decompiler-v2 input.lua --force_overwrite
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-h`, `--help` | Show help message |
| `-o`, `--output PATH` | Output directory, or `-` to write to stdout |
| `-e`, `--extension EXT` | Only decompile files with specified extension |
| `-s`, `--silent_assertions` | Auto-skip files that fail to decompile |
| `-f`, `--force_overwrite` | Always overwrite existing files |
| `-i`, `--ignore_debug_info` | Ignore bytecode debug info |
| `-m`, `--minimize_diffs` | Optimize output formatting to minimize diffs |
| `-u`, `--unrestricted_ascii` | Disable UTF-8 encoding restrictions |

## GitHub Actions

The project includes CI workflows that automatically build and test on:
- Windows (latest)
- Ubuntu (latest)
- macOS (latest)

## TODO

- [ ] Bytecode big endian support
- [ ] Improved decompilation logic for conditional assignments

---

This project uses an boolean expression decompilation algorithm that is based on this paper:
[www.cse.iitd.ac.in/~sak/reports/isec2016-paper.pdf](https://www.cse.iitd.ac.in/~sak/reports/isec2016-paper.pdf)