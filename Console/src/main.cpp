#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <map>
#include <fstream>
#include "VFSXmlParser.hpp"
#include "VFS.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <climits>
#endif


struct Config {
    std::string vfsPath;
    std::string scriptPath;
};

std::string getUsername() {
#if defined(_WIN32)
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetUserNameA(buffer, &size)) {
        return std::string(buffer, size > 0 ? size - 1 : 0);
    }

    char* envValue = nullptr;
    size_t len = 0;
    if (_dupenv_s(&envValue, &len, "USERNAME") == 0 && envValue != nullptr) {
        std::string result(envValue);
        free(envValue);
        return result;
    }
    return "user";
#else
    // POSIX
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    return "user";
#endif
}

std::string getHostname() {
#if defined(_WIN32)
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer, size);
    }
    return "localhost";
#else
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif
    char host[HOST_NAME_MAX + 1];
    if (gethostname(host, sizeof(host)) == 0) {
        host[HOST_NAME_MAX] = '\0';
        return std::string(host);
    }
    return "localhost";
#endif
}

bool parseArgs(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--vfs" || arg == "-v") {
            if (i + 1 < argc) {
                config.vfsPath = argv[++i];
            }
            else {
                std::cerr << "Error: Option " << arg << " requires a path argument.\n";
                return false;
            }
        }
        else if (arg == "--script" || arg == "-s") {
            if (i + 1 < argc) {
                config.scriptPath = argv[++i];
            }
            else {
                std::cerr << "Error: Option " << arg << " requires a file path argument.\n";
                return false;
            }
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " --vfs <vfs_path> [--script <script_path>]\n";
            std::exit(0);
        }
        else {
            std::cerr << "Error: Unknown argument " << arg << "\n";
            return false;
        }
    }

    if (config.vfsPath.empty()) {
        std::cerr << "Error: Missing required parameter --vfs <path>\n";
        return false;
    }

    return true;
}

void dumpConfig(const Config& config) {
    std::cout << "vfs = " << config.vfsPath << "\n";
    std::cout << "script = " << (config.scriptPath.empty() ? "<none>" : config.scriptPath) << "\n";
}


std::vector<std::string> parseInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool executeCommand(const std::vector<std::string>& tokens, const Config& config, const VFS& vfs) {
    if (tokens.empty()) return true;

    const std::string& cmd = tokens[0];

    if (cmd == "exit") {
        if (tokens.size() > 1) {
            std::cerr << "exit: too many arguments\n";
            return true;
        }
        return false;
    }
    else if (cmd == "conf-dump") {
        dumpConfig(config);
    }
    else if (cmd == "vfs-dump") {
        std::cout << "=== VFS Tree in Memory ===\n";
        vfs.printTree(vfs.root);
        std::cout << "==========================\n";
    }
    else if (cmd == "ls" || cmd == "cd") {
        std::cout << "[STUB] Executing command: " << cmd << "\n";
        std::cout << "Arguments (" << tokens.size() - 1 << "): ";
        for (size_t i = 1; i < tokens.size(); ++i) {
            std::cout << "\"" << tokens[i] << "\" ";
        }
        std::cout << "\n";
    }
    else {
        std::cerr << cmd << ": command not found\n";
    }

    return true;
}

int main(int argc, char* argv[]) {
    Config config;
    if (!parseArgs(argc, argv, config)) {
        std::cerr << "Usage: " << argv[0] << " --vfs <vfs_path> [--script <script_path>]\n";
        return 1;
    }

    VFS vfs;
    if (!VFSXmlParser::loadVFS(config.vfsPath, vfs)) {
        std::cerr << "Error: Failed to initialize VFS from " << config.vfsPath << "\n";
        return 1;
    }

    std::cout << "=== Debug: Loaded Configuration ===\n";
    dumpConfig(config);
    std::cout << "===================================\n\n";

    const std::string prompt = getUsername() + "@" + getHostname() + ":~$ ";

    if (!config.scriptPath.empty()) {
        std::ifstream scriptFile(config.scriptPath);
        if (!scriptFile.is_open()) {
            std::cerr << "Error: Could not open script file: " << config.scriptPath << "\n";
        }
        else {
            std::string line;
            while (std::getline(scriptFile, line)) {
                std::cout << prompt << line << "\n";

                auto tokens = parseInput(line);
                if (!executeCommand(tokens, config, vfs)) {
                    return 0;
                }
            }
        }
    }

    std::string line;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        auto tokens = parseInput(line);
        if (!executeCommand(tokens, config, vfs)) {
            break;
        }
    }

    return 0;
}