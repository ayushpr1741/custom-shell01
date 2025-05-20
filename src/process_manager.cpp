#include <iostream>
#include <vector>
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

const string shellPassword = "1234";
const string notesFile = "shell_notes.txt";

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
        bool stillRunning = (exitCode == STILL_ACTIVE);
        string status = stillRunning ? "Running" : "Exited";
        cout << "[" << job.id << "] PID: " << job.pid << " Command: " << job.command << " Status: " << status << endl;
    }
}

void fg(int jobId) {
    for (auto it = jobList.begin(); it != jobList.end(); ++it) {
        if (it->id == jobId) {
            cout << "Bringing job [" << it->id << "] to foreground...\n";
            WaitForSingleObject(it->hProcess, INFINITE);
            CloseHandle(it->hProcess);
            jobList.erase(it);
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
    char* cmd = _strdup(cmdLine.c_str());

    BOOL success = CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (success) {
        addJob(pi.hProcess, pi.dwProcessId, command);
        CloseHandle(pi.hThread);
    } else {
        cerr << "Failed to launch process: " << command << endl;
    }
    free(cmd);
}

vector<string> split(const string &str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void runPipedCommand(const string &command) {
    vector<string> commands = split(command, '|');
    if (commands.size() != 2) {
        cout << "Error: Only single pipe supported.\n";
        return;
    }

    string cmd1 = "cmd.exe /C " + commands[0];
    string cmd2 = "cmd.exe /C " + commands[1];

    HANDLE readPipe, writePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        cerr << "Error creating pipe.\n";
        return;
    }

    STARTUPINFOA si1 = {};
    PROCESS_INFORMATION pi1 = {};
    si1.cb = sizeof(si1);
    si1.dwFlags = STARTF_USESTDHANDLES;
    si1.hStdOutput = writePipe;
    si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    char* cmdline1 = _strdup(cmd1.c_str());
    if (!CreateProcessA(NULL, cmdline1, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1)) {
        cerr << "Error launching first command.\n";
        CloseHandle(writePipe);
        CloseHandle(readPipe);
        free(cmdline1);
        return;
    }
    free(cmdline1);
    CloseHandle(writePipe);

    STARTUPINFOA si2 = {};
    PROCESS_INFORMATION pi2 = {};
    si2.cb = sizeof(si2);
    si2.dwFlags = STARTF_USESTDHANDLES;
    si2.hStdInput = readPipe;
    si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    char* cmdline2 = _strdup(cmd2.c_str());
    if (!CreateProcessA(NULL, cmdline2, NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2)) {
        cerr << "Error launching second command.\n";
        CloseHandle(readPipe);
        free(cmdline2);
        return;
    }
    free(cmdline2);
    CloseHandle(readPipe);

    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);
}

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
    cmd.erase(cmd.find_last_not_of(" \n\r\t")+1);
    filename.erase(0, filename.find_first_not_of(" \n\r\t"));

    HANDLE hFile = NULL;
    if (redirectOutput) {
        hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    } else if (redirectInput) {
        hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (redirectOutput) si.hStdOutput = hFile;
    if (redirectInput) si.hStdInput = hFile;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    SetHandleInformation(hFile, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    string fullCmd = "cmd.exe /C " + cmd;
    char* cmdline = _strdup(fullCmd.c_str());
    BOOL success = CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    if (!success) cerr << "CreateProcess failed.\n";
    else {
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

void scheduleCommand(const string& command, int delaySeconds) {
    cout << "Scheduling command: \"" << command << "\" to run after " << delaySeconds << " seconds.\n";
    Sleep(delaySeconds * 1000);
    system(command.c_str());
}

void addNote(const string& note) {
    ofstream out(notesFile, ios::app);
    if (!out) {
        cerr << "Unable to open notes file.\n";
        return;
    }
    out << note << endl;
    cout << "Note added.\n";
}

void viewNotes() {
    ifstream in(notesFile);
    string line;
    cout << "Shell Notes:\n";
    while (getline(in, line)) {
        cout << "- " << line << endl;
    }
}

bool authenticateShell() {
    string pass;
    cout << "Enter shell password: ";
    getline(cin, pass);
    return pass == shellPassword;
}

void lockShell() {
    string pass;
    cout << "Shell locked. Enter password to unlock:\n";
    do {
        cout << "Password: ";
        getline(cin, pass);
        if (pass != shellPassword) cout << "Incorrect password. Try again.\n";
    } while (pass != shellPassword);
    cout << "Shell unlocked.\n";
}

int main() {
    if (!authenticateShell()) {
        cout << "Incorrect password. Exiting shell.\n";
        return 1;
    }

    while (true) {
        cout << "\nShell> ";
        string input;
        getline(cin, input);

        if (input == "exit") break;
        else if (input == "jobs") listJobs();
        else if (input.rfind("fg ", 0) == 0) fg(stoi(input.substr(3)));
        else if (input.rfind("kill ", 0) == 0) killJob(stoi(input.substr(5)));
        else if (input.rfind("run ", 0) == 0) launchBackgroundProcess(input.substr(4));
        else if (input.find('|') != string::npos) runPipedCommand(input);
        else if (input.find('>') != string::npos || input.find('<') != string::npos) runWithRedirection(input);
        else if (input.rfind("ping ", 0) == 0) runPingCommand(input.substr(5));
        else if (input.rfind("schedule ", 0) == 0) {
            size_t atPos = input.find(" at ");
            if (atPos != string::npos) {
                string cmd = input.substr(9, atPos - 9);
                int delay = stoi(input.substr(atPos + 4));
                scheduleCommand(cmd, delay);
            } else cout << "Usage: schedule <command> at <seconds>\n";
        }
        else if (input.rfind("note add ", 0) == 0) addNote(input.substr(9));
        else if (input == "note view") viewNotes();
        else if (input == "lock") lockShell();
        else system(input.c_str());
    }

    return 0;
}
