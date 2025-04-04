#ifndef SHELL_H
#define SHELL_H

#include <vector>
#include <string>
using namespace std;
// Command history storage
extern std::vector<std::string> command_history;

// Function declarations
void execute_command(const std::vector<std::string>& args);
void print_help();
string wide_to_narrow(const wchar_t* wide);

#endif
