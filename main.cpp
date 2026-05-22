#include "main.h"

struct Error {
    const std::string message;
    const std::string filePath;
    const std::string function;
    const std::string source;
    const std::string line;
};

static bool isProgressBarActive = false;
static uint32_t filesSkipped = 0;

struct Arguments {
    bool showHelp = false;
    bool silentAssertions = false;
    bool forceOverwrite = false;
    bool ignoreDebugInfo = false;
    bool minimizeDiffs = false;
    bool unrestrictedAscii = false;
    std::string inputPath;
    std::string outputPath;
    std::string extensionFilter;
} arguments;

struct Directory {
    std::string path;
    std::vector<Directory> folders;
    std::vector<std::string> files;
};

static std::string string_to_lowercase(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] += 'a' - 'A';
        }
    }
    return result;
}

static void find_files_recursively(Directory& directory) {
    std::string searchPath = arguments.inputPath + directory.path;

    for (const auto& entry : fs::recursive_directory_iterator(searchPath)) {
        if (entry.is_directory()) {
            std::string relPath = fs::relative(entry.path(), searchPath).string();
            if (!relPath.empty() && relPath != ".") {
                // Normalize path separator
                for (size_t i = 0; i < relPath.size(); i++) {
                    if (relPath[i] == '\\') relPath[i] = '/';
                }
                if (!relPath.empty() && relPath.back() != '/') relPath += '/';

                // Check if this subdirectory already exists
                bool found = false;
                for (auto& folder : directory.folders) {
                    if (folder.path == directory.path + relPath) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    directory.folders.push_back(Directory{ .path = directory.path + relPath });
                }
            }
        } else {
            std::string ext = string_to_lowercase(entry.path().extension().string());
            if (!arguments.extensionFilter.size() ||
                arguments.extensionFilter == ext) {
                directory.files.push_back(entry.path().filename().string());
            }
        }
    }
}

static bool decompile_files_recursively(const Directory& directory) {
    std::string fullOutputPath = arguments.outputPath + directory.path;
    Platform::create_directory(fullOutputPath);

    for (size_t i = 0; i < directory.files.size(); i++) {
        std::string outputFile = directory.files[i];
        outputFile = Platform::remove_extension(outputFile);
        outputFile += ".lua";

        try {
            std::string inputFile = arguments.inputPath + directory.path + directory.files[i];
            std::string outputFileFull = fullOutputPath + outputFile;

            erase_progress_bar();
            print("--------------------");
            print("Input file: " + inputFile);
            print("Reading bytecode...");

            Bytecode bytecode(inputFile);
            bytecode();
            print("Building ast...");
            Ast ast(bytecode, arguments.ignoreDebugInfo, arguments.minimizeDiffs);
            ast();
            print("Writing lua source...");
            Lua lua(bytecode, ast, outputFileFull, arguments.forceOverwrite,
                    arguments.minimizeDiffs, arguments.unrestrictedAscii);
            lua();
            erase_progress_bar();
            print("Output file: " + outputFileFull);
        } catch (const Error& error) {
            erase_progress_bar();

            if (arguments.silentAssertions) {
                print("\nError running " + error.function +
                      "\nSource: " + error.source + ":" + error.line +
                      "\n\n" + error.message);
                filesSkipped++;
                continue;
            }

            print("\n[ERROR] " + error.function);
            print("Source: " + error.source + ":" + error.line);
            print("File: " + error.filePath);
            print(error.message);
            print("File skipped.");
            filesSkipped++;
        } catch (const std::exception& e) {
            print("\n[ERROR] Exception: " + std::string(e.what()));
            print("File skipped.");
            filesSkipped++;
        }
    }

    for (size_t i = 0; i < directory.folders.size(); i++) {
        if (!decompile_files_recursively(directory.folders[i])) {
            return false;
        }
    }

    return true;
}

static char* parse_arguments(int argc, char** argv) {
    if (argc < 2) return nullptr;
    arguments.inputPath = argv[1];
    bool isInputPathSet = true;

    if (!arguments.inputPath.empty() && arguments.inputPath[0] == '-') {
        arguments.inputPath.clear();
        isInputPathSet = false;
    }

    std::string argument;

    for (size_t i = isInputPathSet ? 2 : 1; i < static_cast<size_t>(argc); i++) {
        argument = argv[i];

        if (argument.size() >= 2 && argument[0] == '-') {
            if (argument[1] == '-') {
                argument = argument.c_str() + 2;

                if (argument == "extension") {
                    if (i <= static_cast<size_t>(argc) - 2) {
                        i++;
                        arguments.extensionFilter = argv[i];
                        continue;
                    }
                } else if (argument == "force_overwrite") {
                    arguments.forceOverwrite = true;
                    continue;
                } else if (argument == "help") {
                    arguments.showHelp = true;
                    continue;
                } else if (argument == "ignore_debug_info") {
                    arguments.ignoreDebugInfo = true;
                    continue;
                } else if (argument == "minimize_diffs") {
                    arguments.minimizeDiffs = true;
                    continue;
                } else if (argument == "output") {
                    if (i <= static_cast<size_t>(argc) - 2) {
                        i++;
                        arguments.outputPath = argv[i];
                        continue;
                    }
                } else if (argument == "silent_assertions") {
                    arguments.silentAssertions = true;
                    continue;
                } else if (argument == "unrestricted_ascii") {
                    arguments.unrestrictedAscii = true;
                    continue;
                }
            } else if (argument.size() == 2) {
                switch (argument[1]) {
                case 'e':
                    if (i > static_cast<size_t>(argc) - 2) break;
                    i++;
                    arguments.extensionFilter = argv[i];
                    continue;
                case 'f':
                    arguments.forceOverwrite = true;
                    continue;
                case '?':
                case 'h':
                    arguments.showHelp = true;
                    continue;
                case 'i':
                    arguments.ignoreDebugInfo = true;
                    continue;
                case 'm':
                    arguments.minimizeDiffs = true;
                    continue;
                case 'o':
                    if (i > static_cast<size_t>(argc) - 2) break;
                    i++;
                    arguments.outputPath = argv[i];
                    continue;
                case 's':
                    arguments.silentAssertions = true;
                    continue;
                case 'u':
                    arguments.unrestrictedAscii = true;
                    continue;
                }
            }
        }

        return argv[i];
    }

    return nullptr;
}

static void wait_for_exit() {
    print("Press Enter to exit...");
    std::string dummy;
    std::getline(std::cin, dummy);
}

int main(int argc, char* argv[]) {
    print(std::string(PROGRAM_NAME) + "\nCompiled on " + __DATE__);

    if (parse_arguments(argc, argv)) {
        print("Invalid argument: " + std::string(parse_arguments(argc, argv)));
        print("Use -h or --help to show usage and options.");
        return EXIT_FAILURE;
    }

    if (arguments.showHelp) {
        print(
            "Usage: luajit-decompiler-v2 INPUT_PATH [options]\n"
            "\n"
            "Available options:\n"
            "  -h, -?, --help\t\tShow this message\n"
            "  -o, --output OUTPUT_PATH\tOverride default output directory\n"
            "  -e, --extension EXTENSION\tOnly decompile files with the specified extension\n"
            "  -s, --silent_assertions\tDisable assertion error pop-up window\n"
            "\t\t\t\t  and auto skip files that fail to decompile\n"
            "  -f, --force_overwrite\t\tAlways overwrite existing files\n"
            "  -i, --ignore_debug_info\tIgnore bytecode debug info\n"
            "  -m, --minimize_diffs\t\tOptimize output formatting to help minimize diffs\n"
            "  -u, --unrestricted_ascii\tDisable default UTF-8 encoding and string restrictions"
        );
        return EXIT_SUCCESS;
    }

    if (!arguments.inputPath.empty() && arguments.inputPath[0] == '-') {
        arguments.inputPath.clear();
    }

    if (!arguments.inputPath.size()) {
        print("No input path specified!");
        print("Usage: luajit-decompiler-v2 INPUT_PATH [options]");
        return EXIT_FAILURE;
    }

    // Normalize path separators
    std::replace(arguments.inputPath.begin(), arguments.inputPath.end(), '\\', '/');

    if (!Platform::path_exists(arguments.inputPath)) {
        print("Failed to open input path: " + arguments.inputPath);
        return EXIT_FAILURE;
    }

    // Set default output path if not specified
    if (!arguments.outputPath.size()) {
        arguments.outputPath = Platform::get_executable_directory();
        arguments.outputPath = Platform::join_path(arguments.outputPath, "output");
    } else {
        std::replace(arguments.outputPath.begin(), arguments.outputPath.end(), '\\', '/');

        if (!Platform::path_exists(arguments.outputPath)) {
            print("Failed to open output path: " + arguments.outputPath);
            return EXIT_FAILURE;
        }

        if (!Platform::is_directory(arguments.outputPath)) {
            print("Output path is not a folder!");
            return EXIT_FAILURE;
        }

        if (arguments.outputPath.back() != '/') {
            arguments.outputPath += '/';
        }
    }

    if (!arguments.extensionFilter.empty()) {
        if (arguments.extensionFilter.front() != '.') {
            arguments.extensionFilter.insert(arguments.extensionFilter.begin(), '.');
        }
        arguments.extensionFilter = string_to_lowercase(arguments.extensionFilter);
    }

    Directory root;

    if (Platform::is_directory(arguments.inputPath)) {
        if (arguments.inputPath.back() != '/') {
            arguments.inputPath += '/';
        }
        find_files_recursively(root);

        if (!root.files.size() && !root.folders.size()) {
            print("No files " +
                  (arguments.extensionFilter.size() ? "with extension " + arguments.extensionFilter + " " : "") +
                  "found in path: " + arguments.inputPath);
            return EXIT_FAILURE;
        }
    } else {
        root.files.push_back(Platform::get_filename(arguments.inputPath));
        arguments.inputPath = Platform::get_directory(arguments.inputPath) + "/";
    }

    try {
        if (!decompile_files_recursively(root)) {
            print("--------------------");
            print("Aborted!");
            return EXIT_FAILURE;
        }
    } catch (...) {
        throw;
    }

    print("--------------------");
    if (filesSkipped) {
        print("Failed to decompile " + std::to_string(filesSkipped) + " file(s).");
    }
    print("Done!");

    return EXIT_SUCCESS;
}

void print(const std::string& message) {
    std::cout << message << '\n';
}

void print_progress_bar(double progress, double total) {
    static char PROGRESS_BAR[24] = "[====================]";

    uint8_t threshold = static_cast<uint8_t>(std::round(20.0 / total * progress));

    for (int i = 19; i >= 0; i--) {
        PROGRESS_BAR[i + 2] = i < threshold ? '=' : ' ';
    }

    std::cout << PROGRESS_BAR;
    Platform::flush_output();
    isProgressBarActive = true;
}

void erase_progress_bar() {
    if (!isProgressBarActive) return;
    std::cout << "\r                      \r";
    Platform::flush_output();
    isProgressBarActive = false;
}

void assert(bool assertion, const std::string& message, const std::string& filePath,
            const std::string& function, const std::string& source, uint32_t line) {
    if (!assertion) throw Error{
        .message = message,
        .filePath = filePath,
        .function = function,
        .source = source,
        .line = std::to_string(line)
    };
}

std::string byte_to_string(uint8_t byte) {
    char string[] = "0x00";
    uint8_t digit;

    for (int i = 1; i >= 0; i--) {
        digit = (byte >> i * 4) & 0xF;
        string[3 - i] = digit >= 0xA ? 'A' + digit - 0xA : '0' + digit;
    }

    return string;
}