#include "shell.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <sstream>
#include <csignal>
using namespace std;

extern vector<string> command_history;

void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
            cout << "\nInterrupt received (Ctrl+C)\n";
            break;
    }
}


void execute_piped_command(const string& command) {
    string temp = command;
    size_t pipe_pos = temp.find('|');
    if (pipe_pos == string::npos) return;

    string first_cmd = temp.substr(0, pipe_pos);
    string second_cmd = temp.substr(pipe_pos + 1);

    string temp_file = "temp_output.txt";
    first_cmd += " > " + temp_file;
    system(first_cmd.c_str());

    second_cmd += " < " + temp_file;
    system(second_cmd.c_str());

    remove(temp_file.c_str());
}

void handle_redirection_and_execute(const string& input) {
    string command = input;
    string in_file, out_file;
    bool has_input = false, has_output = false;

    size_t in_pos = command.find('<');
    size_t out_pos = command.find('>');

    if (in_pos != string::npos) {
        has_input = true;
        in_file = command.substr(in_pos + 1);
        command = command.substr(0, in_pos);
    }

    if (out_pos != string::npos) {
        has_output = true;
        out_file = command.substr(out_pos + 1);
        command = command.substr(0, out_pos);
    }

    stringstream cmd_stream;
    if (has_input) {
        cmd_stream << command << " < " << in_file;
    }
    if (has_output) {
        cmd_stream << command << " > " << out_file;
    }
    if (!has_input && !has_output) {
        cmd_stream << command;
    }

    system(cmd_stream.str().c_str());
}

void execute_command(const vector<string>& args) {
    if (args.empty()) return;

    string full_command;
    for (const auto& arg : args) full_command += arg + " ";
    command_history.push_back(full_command);

    if (full_command.find('|') != string::npos) {
        execute_piped_command(full_command);
        return;
    }

    if (full_command.find('<') != string::npos || full_command.find('>') != string::npos) {
        handle_redirection_and_execute(full_command);
        return;
    }

    if (args.back() == "&") {
        cerr << "Background execution not supported on this build.\n";
        return;
    }

    //Custom command: delep<filename> (delete file if not in use)
    if (args[0] == "delep") {
        if (args.size() < 2) {
            cerr << "Usage: delep <file>\n";
        } else {
            if (remove(args[1].c_str()) == 0) {
                cout << "Deleted file: " << args[1] << endl;
            } else {
                cerr << "Error: Could not delete file (may be in use).\n";
            }
        }
        return;
    }

    //Custom command:sb (show banner)
    if (args[0] == "sb") {
        cout << "\n==========================\n";
        cout << " Welcome to Custom Shell \n";
        cout << "==========================\n\n";
        return;
    }

    //If command not handled, fall back to system execution
    system(full_command.c_str());
}

// Register signal handlers
void init_signals() {
    signal(SIGINT, signal_handler);
}


