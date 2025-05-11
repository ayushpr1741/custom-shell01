#include <iostream>
#include <vector>
#include <windows.h>
#include <string>
#include <sstream>  


using namespace std;

struct Job {
    int id;
    HANDLE hProcess;
    DWORD pid;
    string command;
    bool isRunning;
};

vector<Job> jobList;
int jobCounter = 1;

void addJob(HANDLE hProcess, DWORD pid, const string& command) {
    Job newJob = { jobCounter++, hProcess, pid, command, true };
    jobList.push_back(newJob);
    cout << "[" << newJob.id << "] " << pid << " started in background\n";
}

void listJobs() {
    cout << "Active Background Jobs:\n";
    for (auto& job : jobList) {
        DWORD exitCode;
        GetExitCodeProcess(job.hProcess, &exitCode);

        // If process is still active, mark as Running
        bool stillRunning = (exitCode == STILL_ACTIVE);
        string status = stillRunning ? "Running" : "Exited";

        cout << "[" << job.id << "] PID: " << job.pid
             << " Command: " << job.command
             << " Status: " << status << endl;
    }
}




void fg(int jobId) {
    for (auto &job : jobList) {
        if (job.id == jobId) {
            cout << "Bringing job [" << job.id << "] to foreground...\n";
            WaitForSingleObject(job.hProcess, INFINITE);  // Wait for completion
            // Optional: Add timeout here to break after certain time.
            CloseHandle(job.hProcess);
            job.isRunning = false;
            return;
        }
    }
    cout << "Error: Job ID not found.\n";
}


void killJob(int jobId) {
    for (auto it = jobList.begin(); it != jobList.end(); ++it) {
        if (it->id == jobId) {
            cout << "Killing job [" << it->id << "]...\n";
            TerminateProcess(it->hProcess, 0);
            CloseHandle(it->hProcess);
            jobList.erase(it);
            return;
        }
    }
    cout << "Error: Job ID not found.\n";
}

void launchBackgroundProcess(const string& command) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    string cmdLine = "cmd.exe /C " + command;

    // CreateProcess requires a writable char*
    char* cmd = _strdup(cmdLine.c_str());

    BOOL success = CreateProcess(
        NULL,         // No module name (use command line)
        cmd,          // Command line
        NULL,         // Process handle not inheritable
        NULL,         // Thread handle not inheritable
        FALSE,        // Set handle inheritance to FALSE
        CREATE_NO_WINDOW, // Don't create console window (set to 0 if you want one)
        NULL,         // Use parent's environment block
        NULL,         // Use parent's starting directory 
        &si,          // Pointer to STARTUPINFO structure
        &pi           // Pointer to PROCESS_INFORMATION structure
    );

    if (success) {
        addJob(pi.hProcess, pi.dwProcessId, command);
        // Don't close hProcess here — it's needed for job control
        CloseHandle(pi.hThread);
    } else {
        cerr << "Failed to launch process: " << command << endl;
    }

    free(cmd); // Free duplicated string
}

vector<string> split(const string &str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);  // Create a stringstream from the input string
    string token;

    // Use getline to split the string into tokens by the delimiter
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


// Piping function
void runPipedCommand(const string &command) {
    vector<string> commands = split(command, '|'); // Split the command by '|'

    if (commands.size() != 2) {
        cout << "Error: Invalid command syntax for piping.\n";
        return;
    }

    string cmd1 = commands[0];
    string cmd2 = commands[1];

    // Create a pipe
    HANDLE readPipe, writePipe;
    if (!CreatePipe(&readPipe, &writePipe, NULL, 0)) {
        cerr << "Error creating pipe.\n";
        return;
    }

    // Execute the first command
    STARTUPINFO si1 = {0};
    PROCESS_INFORMATION pi1;
    si1.dwFlags = STARTF_USESTDHANDLES;
    si1.hStdOutput = writePipe;

    if (!CreateProcess(NULL, (LPSTR)cmd1.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1)) {
        cerr << "Error executing command 1: " << cmd1 << "\n";
        return;
    }

    // Close write end of the pipe in the parent process
    CloseHandle(writePipe);

    // Execute the second command
    STARTUPINFO si2 = {0};
    PROCESS_INFORMATION pi2;
    si2.dwFlags = STARTF_USESTDHANDLES;
    si2.hStdInput = readPipe;

    if (!CreateProcess(NULL, (LPSTR)cmd2.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
        cerr << "Error executing command 2: " << cmd2 << "\n";
        return;
    }

    // Close the read end of the pipe in the parent process
    CloseHandle(readPipe);

    // Wait for both processes to finish
    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    // Clean up
    CloseHandle(pi1.hProcess);
    CloseHandle(pi2.hProcess);
}

// File I/O redirection function
#include <fstream>  // Required for file streams

void runWithRedirection(const string &command) {
    string cmd, filename;
    bool redirectOutput = false, redirectInput = false;

    size_t pos;
    if ((pos = command.find('>')) != string::npos) {
        cmd = command.substr(0, pos);
        filename = command.substr(pos + 1);
        redirectOutput = true;
    } else if ((pos = command.find('<')) != string::npos) {
        cmd = command.substr(0, pos);
        filename = command.substr(pos + 1);
        redirectInput = true;
    } else {
        cmd = command;
    }

    // Trim spaces
    cmd.erase(cmd.find_last_not_of(" \n\r\t")+1);
    filename.erase(0, filename.find_first_not_of(" \n\r\t"));

    HANDLE hFile = NULL;

    if (redirectOutput) {
        hFile = CreateFileA(
            filename.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
    } else if (redirectInput) {
        hFile = CreateFileA(
            filename.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (redirectOutput) si.hStdOutput = hFile;
    if (redirectInput) si.hStdInput = hFile;

    // Duplicate handles so child can use them
    SetHandleInformation(hFile, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    ZeroMemory(&pi, sizeof(pi));

    string fullCmd = "cmd.exe /C " + cmd;
    char* cmdline = _strdup(fullCmd.c_str());

    BOOL success = CreateProcessA(
        NULL,
        cmdline,
        NULL, NULL, TRUE,
        0, NULL, NULL,
        &si, &pi
    );

    if (!success) {
        cerr << "CreateProcess failed.\n";
    } else {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    CloseHandle(hFile);
    free(cmdline);
}


void runPingCommand(const string &host) {
    string command = "ping " + host;
    system(command.c_str());
}



int main() {
    while (true) {
        cout << "\nShell> ";
        string input;
        getline(cin, input);

        if (input == "exit") break;
        else if (input == "jobs") listJobs();
        else if (input.rfind("fg ", 0) == 0) fg(stoi(input.substr(3)));
        else if (input.rfind("kill ", 0) == 0) killJob(stoi(input.substr(5)));
        else if (input.rfind("run ", 0) == 0) {
            string cmd = input.substr(4);
            launchBackgroundProcess(cmd);
        }
        else if (input.find('|') != string::npos) {
            runPipedCommand(input);
        }
        else if (input.find('>') != string::npos || input.find('<') != string::npos) {
            runWithRedirection(input);
        }
        else if (input.rfind("ping ", 0) == 0) {
            string host = input.substr(5);
            runPingCommand(host);
        }
        else {
            // Fallback for other native Windows commands
            system(input.c_str());
        }
    }

    return 0;
}
