// Purpose: The code_checker program: walks the repository,
// runs the rules on every checked file and prints violations.
// Introduced in: Stage 0b (rebuild)
// Architecture: It walks the root folder (not recursively)
// and src/, tests/ and tools/ (recursively), skipping .git.
// checkedKind from checker decides which files are checked;
// --list-cpp prints only the sorted C++ file paths.

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "checker.h"
#include "text_cleaner.h"

namespace {

// Turns CRLF line endings into LF, as files are read.
std::string normalizeNewlines(const std::string& text) {
    std::string result;
    int size = static_cast<int>(text.size());
    for (int i = 0; i < size; i = i + 1) {
        if (text[i] == '\r' && i + 1 < size && text[i + 1] == '\n') {
            continue;
        }
        result.push_back(text[i]);
    }
    return result;
}

// Reads one file in binary; an unreadable file reads as empty.
std::string readFile(const std::filesystem::path& file) {
    std::ifstream stream(file, std::ios::binary);
    if (!stream) {
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(stream)),
                        std::istreambuf_iterator<char>());
    return normalizeNewlines(content);
}

// Adds the files directly in the root folder (not recursive).
void addRootFiles(const std::filesystem::path& root,
                  std::vector<std::string>& paths) {
    std::error_code error;
    std::filesystem::directory_iterator it(root, error);
    if (error) {
        return;
    }
    for (const std::filesystem::directory_entry& entry : it) {
        std::error_code fileError;
        if (entry.is_regular_file(fileError)) {
            paths.push_back(entry.path().filename().generic_string());
        }
    }
}

// Adds the files under one folder, recursively, skipping .git.
void addTreeFiles(const std::filesystem::path& root, const std::string& name,
                  std::vector<std::string>& paths) {
    std::error_code error;
    std::filesystem::path base = root / name;
    std::filesystem::recursive_directory_iterator it(base, error);
    if (error) {
        return;
    }
    while (it != std::filesystem::recursive_directory_iterator()) {
        std::error_code entryError;
        if (it->is_directory(entryError) && it->path().filename() == ".git") {
            it.disable_recursion_pending();
        } else if (it->is_regular_file(entryError)) {
            std::error_code relError;
            std::filesystem::path rel =
                std::filesystem::relative(it->path(), root, relError);
            if (!relError) {
                paths.push_back(rel.generic_string());
            }
        }
        it.increment(error);
        if (error) {
            break;
        }
    }
}

// Collects every file the walk visits, sorted by path.
std::vector<std::string> collectPaths(const std::filesystem::path& root) {
    std::vector<std::string> paths;
    addRootFiles(root, paths);
    addTreeFiles(root, "src", paths);
    addTreeFiles(root, "tests", paths);
    addTreeFiles(root, "tools", paths);
    std::sort(paths.begin(), paths.end());
    return paths;
}

// Prints the sorted C++ file paths, one per line, nothing else.
int listCppFiles(const std::vector<std::string>& paths) {
    for (const std::string& path : paths) {
        if (aih::checkedKind(path) == aih::FileKind::Cpp) {
            std::cout << path << "\n";
        }
    }
    return 0;
}

// Checks every checked file and prints violations and summary.
int runChecks(const std::filesystem::path& root,
              const std::vector<std::string>& paths) {
    int files = 0;
    int total = 0;
    for (const std::string& path : paths) {
        aih::FileKind kind = aih::checkedKind(path);
        if (kind == aih::FileKind::None) {
            continue;
        }
        files = files + 1;
        std::string text = readFile(root / std::filesystem::path(path));
        std::vector<aih::Violation> violations =
            aih::checkText(path, text, kind);
        for (const aih::Violation& violation : violations) {
            std::cout << aih::formatViolation(path, violation) << "\n";
        }
        total = total + static_cast<int>(violations.size());
    }
    std::cout << "code checks: " << files << " files, " << total
              << " violations\n";
    if (total == 0) {
        return 0;
    }
    return 1;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string rootText = ".";
    bool listCpp = false;
    if (argc > 1 && std::string(argv[1]) == "--list-cpp") {
        listCpp = true;
        if (argc > 2) {
            rootText = argv[2];
        }
    } else if (argc > 1) {
        rootText = argv[1];
    }
    std::filesystem::path root(rootText);
    std::vector<std::string> paths = collectPaths(root);
    if (listCpp) {
        return listCppFiles(paths);
    }
    return runChecks(root, paths);
}
