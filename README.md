# Linux System Programming Examples in C

A comprehensive collection of practical examples demonstrating core Linux system programming concepts using the C language.

## Overview

These examples cover essential system programming topics that are fundamental to Unix/Linux development:

1. **File I/O** - Low-level file operations
2. **Process Management** - Process creation and control
3. **Signal Handling** - Asynchronous event handling
4. **Inter-Process Communication (IPC)** - Pipes and data exchange
5. **System Information** - Process and system metadata
6. **Memory Management** - Dynamic allocation and memory mapping
7. **Directory Operations** - File system traversal and manipulation

## Prerequisites

- **Linux/Unix** system (Linux, macOS, BSD, etc.)
- **GCC** compiler: `gcc --version`
- **Make** utility: `make --version`
- Basic knowledge of C programming

## Building

### Compile All Examples
```bash
make all
```

### Compile Specific Example
```bash
make file_io
make process_management
make signal_handling
make pipes_ipc
make system_info
make memory_management
make directory_ops
```

### Clean Up
```bash
make clean
```

## Example Details

### 1. File I/O (`file_io.c`)

**Topics Covered:**
- `open()` - Opening files with flags (O_CREAT, O_RDONLY, O_WRONLY, O_APPEND)
- `write()` - Writing data to files
- `read()` - Reading data from files
- `lseek()` - Seeking to file positions
- `close()` - Closing file descriptors

**Key System Calls:**
```c
int fd = open("file.txt", O_CREAT | O_WRONLY, 0644);
ssize_t n = write(fd, buffer, size);
ssize_t n = read(fd, buffer, size);
off_t offset = lseek(fd, position, SEEK_SET);
close(fd);
```

**Run:**
```bash
./file_io
```

### 2. Process Management (`process_management.c`)

**Topics Covered:**
- `fork()` - Creating child processes
- `wait()` / `waitpid()` - Parent waiting for children
- `execlp()` / `execvp()` - Replacing process image
- Process IDs (PID, PPID)
- Multiple process handling

**Key System Calls:**
```c
pid_t pid = fork();           // Create child
wait(&status);                 // Wait for child
int code = WEXITSTATUS(status);// Get exit status
execlp("ls", "ls", "-la", NULL); // Execute command
```

**Run:**
```bash
./process_management
```

### 3. Signal Handling (`signal_handling.c`)

**Topics Covered:**
- `sigaction()` - Signal registration
- Signal handlers (SIGINT, SIGUSR1, SIGTERM)
- Signal masks with `sigemptyset()`
- Volatile variables for signal safety
- Graceful termination patterns

**Key System Calls:**
```c
struct sigaction sa;
sa.sa_handler = signal_handler;
sigemptyset(&sa.sa_mask);
sigaction(SIGINT, &sa, NULL);
```

**Run:**
```bash
./signal_handling
# In another terminal:
kill -USR1 <PID>
kill -TERM <PID>
# Or press Ctrl+C three times in the program
```

### 4. Pipes and IPC (`pipes_ipc.c`)

**Topics Covered:**
- Anonymous pipes with `pipe()`
- Parent-child communication
- Bidirectional pipes
- Pipeline simulation (chaining processes)
- `fdopen()` for file stream conversion

**Key System Calls:**
```c
int pipefd[2];
pipe(pipefd);              // Create pipe
close(pipefd[0]);          // Close unused end
write(pipefd[1], data, size);
read(pipefd[0], buffer, size);
```

**Run:**
```bash
./pipes_ipc
```

### 5. System Information (`system_info.c`)

**Topics Covered:**
- Process identification: `getpid()`, `getppid()`, `getpgid()`
- User/group information: `getuid()`, `getgid()`, `getpwuid()`, `getgrgid()`
- Environment variables: `getenv()`, `environ`
- Current directory: `getcwd()`
- File descriptors and system limits
- File statistics: `stat()`, `fstat()`
- Time information: `time()`, `ctime()`

**Key System Calls:**
```c
pid_t pid = getpid();
struct passwd *pw = getpwuid(getuid());
char *env = getenv("PATH");
char cwd[PATH_MAX];
getcwd(cwd, sizeof(cwd));
struct stat st;
stat(filename, &st);
```

**Run:**
```bash
./system_info
./system_info /path/to/some/file
```

### 6. Memory Management (`memory_management.c`)

**Topics Covered:**
- Dynamic allocation: `malloc()`, `realloc()`, `calloc()`
- Memory-mapped files: `mmap()`, `munmap()`, `msync()`
- Memory operations: `memcpy()`, `memset()`, `memmove()`, `memcmp()`
- Aligned allocation: `posix_memalign()`
- Stack vs Heap

**Key System Calls:**
```c
void *ptr = malloc(size);
ptr = realloc(ptr, new_size);
void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
msync(addr, size, MS_SYNC);
munmap(addr, size);
memcpy(dst, src, size);
memset(ptr, 0, size);
```

**Run:**
```bash
./memory_management
```

### 7. Directory Operations (`directory_ops.c`)

**Topics Covered:**
- Directory listing: `opendir()`, `readdir()`, `closedir()`
- File information in directories
- Directory creation: `mkdir()`
- Directory navigation: `chdir()`, `getcwd()`
- Tree walking: `nftw()`
- Symbolic links: `symlink()`, `readlink()`
- Path operations: `basename()`, `dirname()`
- Temporary files: `mkstemp()`

**Key System Calls:**
```c
DIR *dir = opendir(path);
struct dirent *entry = readdir(dir);
closedir(dir);
mkdir(path, mode);
chdir(path);
nftw(path, callback, max_fds, flags);
symlink(target, linkname);
readlink(linkname, buffer, size);
int fd = mkstemp(template);
```

**Run:**
```bash
./directory_ops
```

## Common Patterns

### Error Handling
```c
int fd = open("file.txt", O_RDONLY);
if (fd == -1) {
    perror("open");  // Prints descriptive error message
    exit(1);
}
```

### Process Creation Pattern
```c
pid_t pid = fork();
if (pid < 0) {
    perror("fork");
    exit(1);
} else if (pid == 0) {
    // Child process
    exit(0);
} else {
    // Parent process
    wait(NULL);
}
```

### Safe Signal Handling
```c
volatile sig_atomic_t flag = 0;

void handler(int sig) {
    flag = 1;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    
    while (!flag) {
        // Safe operations
    }
}
```

## Important Concepts

### File Descriptors
- `STDIN_FILENO` (0) - Standard input
- `STDOUT_FILENO` (1) - Standard output
- `STDERR_FILENO` (2) - Standard error
- Higher numbers for open files

### Process States
- Running: Process is executing
- Sleeping: Waiting for I/O or event
- Stopped: Suspended by signal
- Zombie: Terminated but parent hasn't waited

### Memory Layout
```
High Address  ┌─────────────┐
              │   Stack     │ (grows down)
              │             │
              │             │
              ├─────────────┤
              │   Heap      │ (grows up)
              │             │
              ├─────────────┤
              │   Data      │ (initialized & uninitialized)
              ├─────────────┤
              │   Text      │ (code)
Low Address   └─────────────┘
```

## Tips and Best Practices

1. **Always check return values** - Most system calls return -1 on error
2. **Use `perror()` or `strerror(errno)`** - Get meaningful error messages
3. **Close file descriptors** - Avoid resource leaks
4. **Handle signals safely** - Use only async-signal-safe functions
5. **Test edge cases** - Empty files, permission errors, etc.
6. **Use compiler warnings** - Compile with `-Wall -Wextra`
7. **Learn system call man pages** - `man 2 fork`, `man 3 malloc`, etc.

## Further Reading

### Man Pages
```bash
man 2 syscalls        # Overview of system calls
man 2 fork            # Process creation
man 2 open            # File operations
man 2 signal          # Signal handling
man 3 malloc          # Memory allocation
man 3 pthread_create  # Multithreading (not covered here)
```

### Books
- **The C Programming Language** by Kernighan & Ritchie
- **Advanced Programming in the UNIX Environment** by Stevens & Rago
- **UNIX Systems Programming** by Kay Robbins & Steve Robbins

### Online Resources
- Linux man-pages online: https://man7.org/linux/man-pages/
- POSIX specification: https://pubs.opengroup.org/onlinepubs/9699919799/
- Linux Kernel Source: https://kernel.org/

## Troubleshooting

### "Permission denied" on mkdir
```bash
# Check directory permissions
ls -ld test_directory
# Fix permissions if needed
chmod 755 test_directory
```

### "Too many open files"
```bash
# Increase file descriptor limit
ulimit -n 2048
```

### Segmentation fault with pointers
- Always initialize pointers
- Check for NULL before dereferencing
- Use tools like `valgrind` for memory debugging
```bash
valgrind ./program_name
```

## License

These examples are provided as educational material and are in the public domain.

## Questions & Support

Refer to man pages or visit online Linux programming documentation for specific questions.
