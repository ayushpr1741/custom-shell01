#include <iostream>
#include <vector>
#include <windows.h>  // Windows API for process management
#include <string>

using namespace std;

struct Job {
    int id;          // Job ID
    HANDLE hProcess; // Windows process handle
    DWORD pid;       // Process ID
    string command;  // Command string
    bool isRunning;  // Job status (running/stopped)
};

// Global job list
vector<Job> jobList;
int jobCounter = 1; // Keeps track of job numbers

// Function to add a new job to jobList
void addJob(HANDLE hProcess, DWORD pid, const string& command) {
    Job newJob = {jobCounter++, hProcess, pid, command, true};
    jobList.push_back(newJob);
    cout << "[" << newJob.id << "] " << pid << " started in background\n";
}

// Function to list all background jobs
void listJobs() {
    cout << "Active Background Jobs:\n";
    for (const auto& job : jobList) {
        cout << "[" << job.id << "] PID: " << job.pid
             << " Command: " << job.command
             << " Status: " << (job.isRunning ? "Running" : "Stopped") << endl;
    }
}

void fg(int jobId) {
    for (auto &job : jobList) {
        if (job.id == jobId) {
            cout << "Bringing job [" << job.id << "] to foreground...\n";
            WaitForSingleObject(job.hProcess, INFINITE); 
            CloseHandle(job.hProcess);
            job.isRunning = false;
            return;
        }
    }
    cout << "Error: Job ID not found.\n";
}

void bg(int jobId) {
    for (auto &job : jobList) {
        if (job.id == jobId) {
            cout << "Resuming job [" << job.id << "] in background...\n";
            ResumeThread(job.hProcess);
            job.isRunning = true;
            return;
        }
    }
    cout << "Error: Job ID not found.\n";
}

void killJob(int jobId) {
    for (auto &job : jobList) {
        if (job.id == jobId) {
            cout << "Killing job [" << job.id << "]...\n";
            TerminateProcess(job.hProcess, 0);
            CloseHandle(job.hProcess);
            job.isRunning = false;
            return;
        }
    }
    cout << "Error: Job ID not found.\n";
}

int main() {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    
    if (CreateProcess(NULL, (LPSTR)"notepad.exe", NULL, NULL, FALSE,
                      CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        addJob(pi.hProcess, pi.dwProcessId, "notepad.exe");

        
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread); 
    } else {
        cout << "Failed to start notepad.exe\n";
        return 1;
    }

    Sleep(2000); 
    listJobs(); 
    Sleep(1000);
    fg(1); 
    Sleep(1000);

    if (CreateProcess(NULL, (LPSTR)"notepad.exe", NULL, NULL, FALSE,
                      0, NULL, NULL, &si, &pi)) {
        addJob(pi.hProcess, pi.dwProcessId, "notepad.exe");
    }

    Sleep(2000);
    listJobs(); 
    killJob(2); 

    return 0;
}
