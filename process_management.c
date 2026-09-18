#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    printf("Parent process ID: %d\n", getpid());
    
    // Example 1: Fork a child process
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child process ID: %d\n", getpid());
        printf("Child's parent process ID: %d\n", getppid());
        sleep(2);
        printf("Child process exiting\n");
        exit(42);  // Exit with status code 42
    } else {
        // Parent process
        printf("Parent created child with PID: %d\n", pid);
        
        // Example 2: Wait for child process
        int status;
        pid_t child_pid = wait(&status);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Child %d exited with status: %d\n", child_pid, exit_code);
        }
    }
    
    // Example 3: Execute a program
    printf("\n--- Executing external program ---\n");
    pid = fork();
    if (pid == 0) {
        // Child: execute ls command
        execlp("ls", "ls", "-la", "/tmp", NULL);
        // If execlp succeeds, this line is never reached
        perror("execlp failed");
        exit(1);
    } else {
        // Parent: wait for child
        waitpid(pid, NULL, 0);
    }
    
    // Example 4: Multiple children (fork bomb - small scale)
    printf("\n--- Creating multiple children ---\n");
    for (int i = 0; i < 3; i++) {
        pid_t child = fork();
        if (child == 0) {
            printf("Child #%d (PID: %d)\n", i, getpid());
            sleep(1);
            exit(0);
        }
    }
    
    // Parent waits for all children
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }
    
    printf("All children completed\n");
    
    return 0;
}
