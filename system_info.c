#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>

extern char **environ;  // Array of environment variables

int main(int argc, char *argv[]) {
    // Example 1: Process information
    printf("=== Process Information ===\n");
    printf("Process ID (PID): %d\n", getpid());
    printf("Parent Process ID (PPID): %d\n", getppid());
    printf("Process Group ID: %d\n", getpgid(0));
    printf("Session ID: %d\n", getsid(0));
    
    // Example 2: User and group information
    printf("\n=== User and Group Information ===\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID: %d\n", getgid());
    printf("Effective GID: %d\n", getegid());
    
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        printf("Username: %s\n", pw->pw_name);
        printf("User home: %s\n", pw->pw_dir);
        printf("User shell: %s\n", pw->pw_shell);
    }
    
    struct group *gr = getgrgid(getgid());
    if (gr) {
        printf("Group name: %s\n", gr->gr_name);
    }
    
    // Example 3: Environment variables
    printf("\n=== Environment Variables ===\n");
    printf("PATH: %s\n", getenv("PATH") ? getenv("PATH") : "Not set");
    printf("HOME: %s\n", getenv("HOME") ? getenv("HOME") : "Not set");
    printf("USER: %s\n", getenv("USER") ? getenv("USER") : "Not set");
    
    printf("\nAll environment variables (first 10):\n");
    int count = 0;
    for (char **env = environ; *env && count < 10; env++, count++) {
        printf("  %s\n", *env);
    }
    
    // Example 4: Working directory
    printf("\n=== Directory Information ===\n");
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("Current working directory: %s\n", cwd);
    }
    
    // Example 5: File descriptors
    printf("\n=== File Descriptors ===\n");
    printf("Standard input (stdin) FD: %d\n", STDIN_FILENO);
    printf("Standard output (stdout) FD: %d\n", STDOUT_FILENO);
    printf("Standard error (stderr) FD: %d\n", STDERR_FILENO);
    
    // Example 6: Limits
    printf("\n=== System Limits ===\n");
    printf("Maximum path length: %d\n", PATH_MAX);
    printf("Maximum number of processes: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    printf("Page size: %ld bytes\n", sysconf(_SC_PAGE_SIZE));
    printf("Number of processors online: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    
    // Example 7: Time information
    printf("\n=== Time Information ===\n");
    time_t now = time(NULL);
    printf("Current time (seconds since epoch): %ld\n", now);
    printf("Current time: %s", ctime(&now));
    
    // Example 8: File statistics
    printf("\n=== File Statistics (this program) ===\n");
    struct stat st;
    if (stat(argv[0], &st) == 0) {
        printf("File size: %ld bytes\n", st.st_size);
        printf("Permissions: %o (octal)\n", st.st_mode & 0777);
        printf("Owner UID: %d\n", st.st_uid);
        printf("Owner GID: %d\n", st.st_gid);
        printf("Last modified: %s", ctime(&st.st_mtime));
        printf("Number of hard links: %ld\n", st.st_nlink);
    }
    
    return 0;
}
