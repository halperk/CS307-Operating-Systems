// Huseyin Alper Karadeniz (28029) - CS 307 PA1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

    // Prints the details of the main SHELL process 
    printf("I'm SHELL process, with PID: %d - Main command is: man free | grep -A 14 -e --human > output.txt\n", getpid());

    // Initializes file descriptior and pipe
    int fd[2];
    pipe(fd);

    // Creates a child process for MAN command
    int cpid1 = fork();

    if (cpid1 < 0) {

        // Checks if fork operation for MAN command failed
        exit(1);

    } else if (cpid1 == 0) {

        // Enters MAN process, which is a child process of SHELL
        printf("I'm MAN process, with PID: %d - My command is: man free\n", getpid());

        // Duplicates fd[1] to STDOUT_FILENO
        dup2(fd[1], STDOUT_FILENO);

        // Closes both read and write ends
        close(fd[0]); close(fd[1]);

        // Executes MAN command
        char* args[] = {"man", "free", NULL};
        execvp(args[0], args);

    } else {
        
        // SHELL process continues to execute
        // Waits for child process of MAN to finish
        waitpid(cpid1, NULL, 0);

        // Creates a child process for GREP command
        int cpid2 = fork();

        if (cpid2 < 0) {

            // Checks if fork operation for GREP command failed
            exit(1);

        } else if (cpid2 == 0) {

            // Enters GREP process, which is a child process of SHELL
            printf("I'm GREP process, with PID: %d - My command is: grep -A 14 -e --human > output.txt\n", getpid());

            // Duplicates fd[0] to STDIN_FILENO
            dup2(fd[0], STDIN_FILENO);

            // Closes both write and read ends
            close(fd[1]); close(fd[0]);

            // Creates a file to write the output of the GREP command
            int new_fd = open("output.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            
            // Duplicates new_fd to STDIN_FILENO
            dup2(new_fd, STDOUT_FILENO);

            // Closes new_fd
            close(new_fd);

            // Executes GREP command
            char* args[] = {"grep", "-A 14", "-e", "--human", NULL};
            execvp(args[0], args);

        } else {
            
            // SHELL process continues to execute
            // Closes both write and read ends
            close(fd[1]); close(fd[0]);

            // Waits for child process of GREP to finish
            waitpid(cpid2, NULL, 0);

            // Informs the user about all processes finished
            printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid());
            
        }
    }

    return 0;
}
