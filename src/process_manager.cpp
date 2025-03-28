#include <iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

struct Job {
    int id;          // Job ID
    pid_t pid;       // Process ID
    string command;  // Command string
    bool isRunning;  // Job status (running/stopped)
};

// Global job list
vector<Job> jobList;
int jobCounter = 1; // Keeps track of job numbers

// Function to add a new job to jobList
void addJob(pid_t pid, const string& command) {
    Job newJob = {jobCounter++, pid, command, true}; // New job entry
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
int main()
{
    
}
