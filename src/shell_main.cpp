#include "shell.h"
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <direct.h>
#include <sstream>

using namespace std;

extern void init_signals();
extern void execute_command(const vector<string>& args);
extern string get_input_with_features();

int main() {
    string input;
    vector<string> args;

    init_signals();

    cout << "Custom Shell (type 'help' for commands)\n";

    while (true) {
        char cwd[MAX_PATH];
        if (_getcwd(cwd, MAX_PATH)) {
            cout << "\n" << cwd << " ";
        }

        input = get_input_with_features();
        if (input.empty()) continue;
        if (input == "exit") break;

        args.clear();
        stringstream ss(input);
        string token;
        while (ss >> token) args.push_back(token);

        execute_command(args);
    }

    return 0;
}
