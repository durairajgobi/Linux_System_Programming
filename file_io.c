#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Example 1: Create and write to a file
    int fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    
    const char *data = "Hello, Linux System Programming!\n";
    ssize_t bytes_written = write(fd, data, strlen(data));
    printf("Bytes written: %ld\n", bytes_written);
    close(fd);
    
    // Example 2: Read from a file
    fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    
    char buffer[256];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Read from file: %s", buffer);
    }
    close(fd);
    
    // Example 3: Seek and read at specific position
    fd = open("test.txt", O_RDONLY);
    lseek(fd, 7, SEEK_SET);  // Seek to position 7
    bytes_read = read(fd, buffer, 5);
    buffer[bytes_read] = '\0';
    printf("Read from position 7: %s\n", buffer);
    close(fd);
    
    // Example 4: Append to file
    fd = open("test.txt", O_APPEND | O_WRONLY);
    const char *append_data = "Appended line.\n";
    write(fd, append_data, strlen(append_data));
    close(fd);
    
    return 0;
}
