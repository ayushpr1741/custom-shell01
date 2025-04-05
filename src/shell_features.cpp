#include "shell.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <direct.h>
#include <cstdlib>

using namespace std;

vector<string> command_history;

void execute_command(const vector<string>& args) {
    if (args.empty()) return;

    command_history.push_back(args[0]);

    string cmd = args[0];

    if (cmd == "help") {
        cout << "Basic Commands:\n";
        cout << "  help               Show this help message\n";
        cout << "  exit               Exit the shell\n";
        cout << "  cd <dir>           Change directory\n";
        cout << "  cls                Clear the screen\n";
        cout << "  mkdir <dir>        Create a directory\n";
        cout << "  rmdir <dir>        Remove a directory\n";
        cout << "  echo <text>        Print text to the console\n";
        cout << "  history            Show previously run commands\n";

        cout << "\nAdvanced Commands:\n";
        cout << "  run <program>      Run a program (e.g., notepad.exe)\n";
        cout << "  time               Show current system time\n";
        cout << "  date               Show current system date\n";
        cout << "  open <file>        Open a file with its default program\n";
        cout << "  find <word>        Search for a word in a file (interactive)\n";
    } 
    else if (cmd == "cd") {
        if (args.size() < 2) cout << "Usage: cd <directory>\n";
        else if (_chdir(args[1].c_str()) != 0) perror("cd failed");
    } 
    else if (cmd == "cls") {
        system("cls");  // Windows-specific
    }
    else if (cmd == "mkdir") {
        if (args.size() < 2) cout << "Usage: mkdir <dir>\n";
        else if (_mkdir(args[1].c_str()) != 0) perror("mkdir failed");
    }
    else if (cmd == "rmdir") {
        if (args.size() < 2) cout << "Usage: rmdir <dir>\n";
        else if (_rmdir(args[1].c_str()) != 0) perror("rmdir failed");
    }
    else if (cmd == "echo") {
        for (size_t i = 1; i < args.size(); ++i) cout << args[i] << " ";
        cout << "\n";
    }
    else if (cmd == "history") {
        for (size_t i = 0; i < command_history.size(); ++i)
            cout << i + 1 << ": " << command_history[i] << "\n";
    }
    else if (cmd == "run") {
        if (args.size() < 2) cout << "Usage: run <program>\n";
        else system(args[1].c_str());
    }
    else if (cmd == "time") {
        SYSTEMTIME t;
        GetLocalTime(&t);
        printf("Time: %02d:%02d:%02d\n", t.wHour, t.wMinute, t.wSecond);
    }
    else if (cmd == "date") {
        SYSTEMTIME t;
        GetLocalTime(&t);
        printf("Date: %02d/%02d/%04d\n", t.wDay, t.wMonth, t.wYear);
    }
    else if (cmd == "open") {
        if (args.size() < 2) cout << "Usage: open <file>\n";
        else ShellExecuteA(NULL, "open", args[1].c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
    else if (cmd == "find") {
        if (args.size() < 2) {
            cout << "Usage: find <word>\n";
            return;
        }
        string filename, line;
        cout << "Enter file name: ";
        getline(cin, filename);
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Could not open file.\n";
            return;
        }
        int line_num = 1;
        while (getline(file, line)) {
            if (line.find(args[1]) != string::npos) {
                cout << "Line " << line_num << ": " << line << "\n";
            }
            line_num++;
        }
        file.close();
    }
    else {
        cout << "Unknown command: " << cmd << "\n";
    }
}
