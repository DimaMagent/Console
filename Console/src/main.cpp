#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <climits>
#endif

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


std::vector<std::string> parseInput(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    const std::string username = getUsername();
    const std::string hostname = getHostname();
    const std::string prompt = username + "@" + hostname + ":~$ ";

    std::string line;
    while (true) {
        std::cout << prompt;

        // Обработка EOF
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        auto tokens = parseInput(line);
        if (tokens.empty()) {
            continue;
        }

        const std::string& cmd = tokens[0];

        if (cmd == "exit") {
            if (tokens.size() > 1) {
                std::cerr << "exit: too many arguments\n";
                continue;
            }
            break;
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
    }

    return 0;
}