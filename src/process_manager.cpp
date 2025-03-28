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

int main() {
    
}
