// Huseyin Alper Karadeniz (28029) - CS 307 PA2

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <pthread.h>
#include <mutex>
#include <thread>
#include <cstring>

using namespace std;

void printThread (int *fd, mutex &systemMutex);

int main() {
    
    ifstream in_file;
    in_file.open("commands.txt");
    
    vector<vector<string>> commands;
    vector<vector<char*>> commandsArgs;

    string line;
    while (getline(in_file, line)) { // reads and saves all the commands in the commands.txt
        stringstream ss(line);
        
        string cmd_name, input, option, redirection = "-", filename, background_job = "n";
        
        ss >> cmd_name; // command field
        
        string word;
        while (ss >> word) {
            if (word[0] == '-') { // option field
                option = word;
            } else if (word[0] == '<' || word[0] == '>') { // redirection and filename fields
                redirection = word;
                ss >> filename;
            } else if (word[0] == '&') { // background job field
                background_job = "y";
            } else { // input field
                input = word;
            }
        }
        
        vector<string> command;
        command.push_back(cmd_name);
        command.push_back(input);
        command.push_back(option);
        command.push_back(redirection);
        command.push_back(filename);
        command.push_back(background_job);
        commands.push_back(command);
        
        // creates arguments that will be pushed to execvp then
        vector<char*> argsVector;
        for (int i = 0; i < 3; i++) {
            if (command[i] != "") {
                argsVector.push_back(command[i].data());
            }
        }
        argsVector.push_back(NULL);
        commandsArgs.push_back(argsVector);
    }

    in_file.close();
    
    // creates parse.txt file
    ofstream out_file;

    out_file.open("parse.txt");
    
    for (int i = 0; i < commands.size(); i++) {
        out_file << "----------" << endl;
        out_file << "Command: " << commands[i][0] << endl;
        out_file << "Inputs: " << commands[i][1] << endl;
        out_file << "Options: " << commands[i][2] << endl;
        out_file << "Redirection: " << commands[i][3] << endl;
        out_file << "Background Job: " << commands[i][5] << endl;
        out_file << "----------" << endl;
    }
    
    out_file.close();
    
    vector<thread> threads; // initialization of threads
    vector<int> backgroundOperations;
    mutex systemMutex;
    
    for (int i = 0; i < commands.size(); i++) {
        string command = commands[i][0];
        string redirection = commands[i][3];
        string filename = commands[i][4];
        string background_job = commands[i][5];
        
        if (command == "wait") { // wait command
            
            // waits for all the background processes to be completed
            for (int i = 0; i < backgroundOperations.size(); i++) {
                waitpid(backgroundOperations[i], NULL, 0);
            }
            backgroundOperations.clear();
            
            // joins all threads at that point
            for (int i = 0; i < threads.size(); i++) {
                threads[i].join();
            }
            threads.clear();
            
        } else if (redirection == ">") { // has output redirectioning part
            
            int pid = fork();
            
            if (pid == 0) { // child process
                vector<char*> argsVector;
                for (int j = 0; j < 3; j++) {
                    if (commands[i][j] != "") {
                        argsVector.push_back(commands[i][j].data());
                    }
                }
                argsVector.push_back(NULL);
                char** args = argsVector.data();
                close(STDOUT_FILENO);
                open(filename.data(), O_WRONLY | O_CREAT, 0666);
                execvp(args[0], args); // executes the command
                
            } else if (pid > 0) { // shell process
                if (background_job == "n") { // waits for child process
                    waitpid(pid, NULL, 0);
                } else { // adds the child process to the background
                    backgroundOperations.push_back(pid);
                }
            }
            
        } else { // other commands
            
            int* fd = new int[2];
            pipe(fd);
            
            int pid = fork();
            
            if (pid == 0) { // child process
                if (redirection == "-") { // has no redirectioning part
                    vector<char*> argsVector;
                    for (int j = 0; j < 3; j++) {
                        if (commands[i][j] != "") {
                            argsVector.push_back(commands[i][j].data());
                        }
                    }
                    argsVector.push_back(NULL);
                    char** args = argsVector.data();
                    dup2(fd[1], STDOUT_FILENO); // duplicates to the write end
                    execvp(args[0], args); // executes the command
                } else { // has redirectioning part
                    vector<char*> argsVector;
                    for (int j = 0; j < 3; j++) {
                        if (commands[i][j] != "") {
                            argsVector.push_back(commands[i][j].data());
                        }
                    }
                    argsVector.push_back(NULL);
                    char** args = argsVector.data();
                    close(STDIN_FILENO);
                    open(filename.data(), O_RDONLY);
                    dup2(fd[1], STDOUT_FILENO);
                    execvp(args[0], args); // executes the command
                }
            } else if (pid > 0) { // shell process
                if (background_job == "n") {
                    close(fd[1]);
                    thread newThread(printThread, fd, std::ref(systemMutex));
                    newThread.join();
                } else {
                    close(fd[1]);
                    threads.push_back(thread(printThread, fd, std::ref(systemMutex)));
                }
            }
        }
        
        if (i == commands.size()-1) { // last command before termination
            
            // waits for all the background processes to be completed
            for (int i = 0; i < backgroundOperations.size(); i++) {
                waitpid(backgroundOperations[i], NULL, 0);
            }
            backgroundOperations.clear();
            
            // joins all threads at that point
            for (int i = 0; i < threads.size(); i++) {
                threads[i].join();
            }
            threads.clear();
        }
    }

    return 0;
}

void printThread (int *fd, mutex &systemMutex) {
    
    FILE *stream = fdopen(fd[0], "r");
    std::thread::id threadID = std::this_thread::get_id();
    char l[2];
    
    systemMutex.lock();
    cout << "---- " << threadID << endl;
    while (fgets(&l[0], 2, stream)) {cout << l;}
    cout << "---- " << threadID << endl;
    fflush(stdout);
    systemMutex.unlock();
    
}
