#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    // Example 1: Simple pipe communication between parent and child
    printf("=== Pipe Communication Example ===\n");
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Child process: read from pipe
        close(pipefd[1]);  // Close write end
        
        char buffer[256];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Child received: %s\n", buffer);
        }
        close(pipefd[0]);
        exit(0);
    } else {
        // Parent process: write to pipe
        close(pipefd[0]);  // Close read end
        
        const char *message = "Hello from parent!";
        write(pipefd[1], message, strlen(message));
        close(pipefd[1]);
        
        wait(NULL);  // Wait for child
    }
    
    // Example 2: Bidirectional pipes between parent and child
    printf("\n=== Bidirectional Pipes Example ===\n");
    
    int pipe_p2c[2];  // Parent to child
    int pipe_c2p[2];  // Child to parent
    
    if (pipe(pipe_p2c) == -1 || pipe(pipe_c2p) == -1) {
        perror("pipe");
        exit(1);
    }
    
    pid = fork();
    
    if (pid == 0) {
        // Child process
        close(pipe_p2c[1]);
        close(pipe_c2p[0]);
        
        char buffer[256];
        ssize_t n = read(pipe_p2c[0], buffer, sizeof(buffer) - 1);
        buffer[n] = '\0';
        printf("Child received: %s\n", buffer);
        
        const char *response = "Message received, thank you!";
        write(pipe_c2p[1], response, strlen(response));
        
        close(pipe_p2c[0]);
        close(pipe_c2p[1]);
        exit(0);
    } else {
        // Parent process
        close(pipe_p2c[0]);
        close(pipe_c2p[1]);
        
        const char *msg = "Parent calling child";
        write(pipe_p2c[1], msg, strlen(msg));
        
        char buffer[256];
        ssize_t n = read(pipe_c2p[0], buffer, sizeof(buffer) - 1);
        buffer[n] = '\0';
        printf("Parent received: %s\n", buffer);
        
        close(pipe_p2c[1]);
        close(pipe_c2p[0]);
        wait(NULL);
    }
    
    // Example 3: Pipeline simulation (like: cat file | grep pattern)
    printf("\n=== Pipeline Simulation (processes chained with pipes) ===\n");
    
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    
    // Create first child: generates numbers
    pid = fork();
    if (pid == 0) {
        close(pipe1[0]);
        for (int i = 1; i <= 5; i++) {
            dprintf(pipe1[1], "%d\n", i);
        }
        close(pipe1[1]);
        exit(0);
    }
    
    // Create second child: squares numbers
    pid = fork();
    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        
        char line[256];
        FILE *fp = fdopen(pipe1[0], "r");
        FILE *wp = fdopen(pipe2[1], "w");
        
        while (fgets(line, sizeof(line), fp)) {
            int num = atoi(line);
            fprintf(wp, "Square of %d = %d\n", num, num * num);
        }
        fclose(fp);
        fclose(wp);
        exit(0);
    }
    
    // Parent: read from second pipe
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[1]);
    
    char line[256];
    FILE *fp = fdopen(pipe2[0], "r");
    while (fgets(line, sizeof(line), fp)) {
        printf("Result: %s", line);
    }
    fclose(fp);
    
    // Wait for all children
    wait(NULL);
    wait(NULL);
    
    return 0;
}
