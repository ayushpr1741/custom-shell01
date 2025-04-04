#include <iostream>
#include <string>
#include <vector>  // Add this line
#include "shell.h" // Create this file if missing

// Forward declaration (add this)
void execute_command(const std::vector<std::string>& args);

int main() {
    std::string input;
    std::vector<std::string> args;

    while (true) {
        std::cout << "my-shell> ";
        std::getline(std::cin, input);

        if (input == "exit") break;

        args.clear();
        size_t start = 0, end = 0;
        while ((end = input.find(' ', start)) != std::string::npos) {
            args.push_back(input.substr(start, end - start));
            start = end + 1;
        }
        args.push_back(input.substr(start));

        execute_command(args);
    }
    return 0;
}