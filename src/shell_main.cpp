#include "shell.h"  // Must be included
#include <iostream>
#include <string>
#include <vector>
#include <direct.h>
#include <windows.h>
using namespace std;

// Add this external declaration
extern vector<string> command_history;
extern void init_signals();
extern void execute_command(const vector<string>& args);

int main() {
    string input;
    vector<string> args;

    init_signals(); //Initialize signal handlers

    cout << "Custom Shell (type 'help' for commands)\n";

    while (true) {
        char cwd[MAX_PATH];
        if (_getcwd(cwd, MAX_PATH)) {
            cout << "\n" << cwd << "> ";
        } else {
            cout << "\n> ";
        }

        getline(cin, input);

        if (input.empty()) continue;
        if (input == "exit") break;

        //Simple space based splitting
        args.clear();
        size_t pos = 0;
        while ((pos = input.find(' ')) != string::npos) {
            args.push_back(input.substr(0, pos));
            input.erase(0, pos + 1);
        }
        args.push_back(input);

        //DEBUG:Print parsed command
        cout << "DEBUG: Executing '" << args[0] << "' with " 
             << args.size()-1 << " arguments\n";

        execute_command(args);  
    }
    return 0;
}
