#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <direct.h>

using namespace std;

// Add this for history functionality
vector<string> command_history;

void execute_command(const vector<string>& args) {
    if (args.empty()) return;

    // Add command to history (place at start of function)
    command_history.push_back(args[0]);

    // Built-in commands
    if (args[0] == "cd") {
        if (args.size() < 2) {
            cerr << "cd: missing argument\n";
        } else if (_chdir(args[1].c_str()) != 0) {
            cerr << "cd: failed\n";
        }
    }
    else if (args[0] == "pwd") {
        char cwd[1024];
        if (_getcwd(cwd, sizeof(cwd))) {
            cout << cwd << endl;
        } else {
            cerr << "pwd: failed\n";
        }
    }
    // Add these new commands HERE (before the 'else')
    else if (args[0] == "clear") {
        system("cls");  // Clear screen
    }
    else if (args[0] == "history") {
        // Show last 10 commands
        cout << "Command history:\n";
        int start = (command_history.size() > 10) ? command_history.size() - 10 : 0;
        for (size_t i = start; i < command_history.size(); i++) {
            cout << "  " << i+1 << ": " << command_history[i] << endl;
        }
    }
    // External commands
    else {
        string cmd;
        for (const auto& arg : args) cmd += arg + " ";
        
        // Special handling for GUI programs
        if (cmd.find(".exe") != string::npos || 
            args[0] == "notepad" || args[0] == "calc") {
            system(("start \"\" " + cmd).c_str());
        } else {
            system(cmd.c_str());
        }
    }
}