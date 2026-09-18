#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    // Example 1: Dynamic memory allocation
    printf("=== Dynamic Memory Allocation ===\n");
    
    // malloc: allocate uninitialized memory
    int *arr = (int *)malloc(5 * sizeof(int));
    if (arr == NULL) {
        perror("malloc");
        exit(1);
    }
    
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 10;
    }
    printf("Array allocated with malloc: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    
    // realloc: resize allocated memory
    int *new_arr = (int *)realloc(arr, 10 * sizeof(int));
    if (new_arr == NULL) {
        perror("realloc");
        free(arr);
        exit(1);
    }
    arr = new_arr;
    for (int i = 5; i < 10; i++) {
        arr[i] = i * 10;
    }
    printf("Array resized with realloc (10 elements)\n");
    free(arr);
    
    // calloc: allocate and initialize to zero
    char *buffer = (char *)calloc(100, sizeof(char));
    if (buffer == NULL) {
        perror("calloc");
        exit(1);
    }
    strcpy(buffer, "Hello, allocated memory!");
    printf("String from calloc: %s\n", buffer);
    free(buffer);
    
    // Example 2: Stack vs Heap
    printf("\n=== Stack vs Heap ===\n");
    int stack_var = 42;
    int *heap_ptr = (int *)malloc(sizeof(int));
    *heap_ptr = 100;
    
    printf("Stack variable address: %p (value: %d)\n", (void *)&stack_var, stack_var);
    printf("Heap variable address: %p (value: %d)\n", (void *)heap_ptr, *heap_ptr);
    free(heap_ptr);
    
    // Example 3: Memory-mapped files
    printf("\n=== Memory-Mapped Files ===\n");
    
    // Create a test file
    const char *filename = "test_mmap.txt";
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    
    // Write some data and extend file
    const char *data = "Memory mapped file example - This file is mapped to memory";
    write(fd, data, strlen(data));
    
    // Map file into memory
    void *addr = mmap(NULL, strlen(data), PROT_READ | PROT_WRITE, 
                      MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }
    
    printf("Original content: %s\n", (char *)addr);
    
    // Modify mapped memory (changes reflected in file)
    char *mapped = (char *)addr;
    // Replace "example" with "EXAMPLE"
    memcpy(mapped + 20, "EXAMPLE", 7);
    
    printf("Modified content: %s\n", (char *)addr);
    
    // Sync changes to disk
    if (msync(addr, strlen(data), MS_SYNC) == -1) {
        perror("msync");
    }
    
    munmap(addr, strlen(data));
    close(fd);
    
    // Verify changes persisted
    fd = open(filename, O_RDONLY);
    char verify_buffer[256];
    read(fd, verify_buffer, sizeof(verify_buffer));
    printf("File content after mapping: %s\n", verify_buffer);
    close(fd);
    
    // Example 4: Memory operations (efficient copying/setting)
    printf("\n=== Memory Operations ===\n");
    
    char src[50] = "Source string";
    char dst[50];
    
    // memcpy: copy memory
    memcpy(dst, src, strlen(src) + 1);
    printf("After memcpy: %s\n", dst);
    
    // memset: fill memory with value
    char fill_buf[20];
    memset(fill_buf, '*', 20);
    fill_buf[19] = '\0';
    printf("After memset with '*': %s\n", fill_buf);
    
    // memmove: safer copy (handles overlaps)
    char overlap[30] = "HelloWorld";
    memmove(overlap + 5, overlap, strlen(overlap) + 1);
    printf("After memmove: %s\n", overlap);
    
    // memcmp: compare memory
    if (memcmp("abc", "abc", 3) == 0) {
        printf("memcmp: strings are equal\n");
    }
    
    // Example 5: Aligned memory allocation
    printf("\n=== Aligned Memory Allocation ===\n");
    
    void *aligned_ptr;
    int alignment = 64;  // Often used for SIMD or cache-line alignment
    int result = posix_memalign(&aligned_ptr, alignment, 1024);
    
    if (result == 0) {
        printf("Allocated 1024 bytes aligned to %d bytes\n", alignment);
        printf("Address: %p\n", aligned_ptr);
        printf("Address mod %d = %zu (should be 0)\n", 
               alignment, (size_t)aligned_ptr % alignment);
        free(aligned_ptr);
    } else {
        printf("posix_memalign failed\n");
    }
    
    return 0;
}
