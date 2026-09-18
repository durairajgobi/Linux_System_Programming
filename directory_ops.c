#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>
#include <libgen.h>

// Callback function for nftw (file tree walk)
int process_file(const char *fpath, const struct stat *sb, 
                 int typeflag, struct FTW *ftwbuf) {
    printf("%*s%s", ftwbuf->level * 2, "", basename((char *)fpath));
    
    if (S_ISDIR(sb->st_mode)) {
        printf("/\n");
    } else {
        printf(" (%ld bytes)\n", sb->st_size);
    }
    
    return 0;  // Continue traversal
}

int main(int argc, char *argv[]) {
    // Example 1: List directory contents
    printf("=== Listing Directory Contents ===\n");
    
    const char *dir_to_open = ".";
    DIR *dir = opendir(dir_to_open);
    
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }
    
    struct dirent *entry;
    int file_count = 0, dir_count = 0;
    
    printf("Contents of %s:\n", dir_to_open);
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden entries (except . and ..)
        if (entry->d_name[0] == '.' && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            continue;
        }
        
        printf("  %s", entry->d_name);
        
        if (entry->d_type == DT_DIR) {
            printf(" (directory)\n");
            dir_count++;
        } else if (entry->d_type == DT_REG) {
            printf(" (file)\n");
            file_count++;
        } else {
            printf(" (other)\n");
        }
    }
    closedir(dir);
    
    printf("Total: %d files, %d directories\n\n", file_count, dir_count);
    
    // Example 2: Detailed file information in directory
    printf("=== Detailed File Information ===\n");
    
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        struct stat st;
        if (stat(entry->d_name, &st) == 0) {
            printf("File: %s\n", entry->d_name);
            printf("  Size: %ld bytes\n", st.st_size);
            printf("  Permissions: %o\n", st.st_mode & 0777);
            printf("  Links: %ld\n", st.st_nlink);
            printf("  Type: %s\n", 
                   S_ISDIR(st.st_mode) ? "Directory" :
                   S_ISREG(st.st_mode) ? "Regular file" :
                   S_ISLNK(st.st_mode) ? "Symbolic link" :
                   "Other");
        }
    }
    closedir(dir);
    
    // Example 3: Create directory structure
    printf("\n=== Creating Directories ===\n");
    
    const char *newdir = "test_directory";
    if (mkdir(newdir, 0755) == 0) {
        printf("Created directory: %s\n", newdir);
    } else {
        perror("mkdir");
    }
    
    // Create nested directories
    char nested[256];
    sprintf(nested, "%s/subdir1/subdir2", newdir);
    char path[256];
    strcpy(path, "");
    
    for (char *p = nested; *p; p++) {
        if (*p == '/') {
            strncat(path, "/", 1);
            char temp[256];
            strcpy(temp, path);
            mkdir(temp, 0755);
        } else {
            strncat(path, p, 1);
        }
    }
    mkdir(nested, 0755);
    printf("Created nested directory: %s\n", nested);
    
    // Example 4: Recursive directory traversal with nftw
    printf("\n=== Recursive Directory Traversal ===\n");
    
    if (nftw(newdir, process_file, 20, 0) == -1) {
        perror("nftw");
    }
    
    // Example 5: Change directory
    printf("\n=== Changing Directories ===\n");
    
    char original_dir[256];
    if (getcwd(original_dir, sizeof(original_dir))) {
        printf("Original directory: %s\n", original_dir);
    }
    
    if (chdir(newdir) == 0) {
        char current_dir[256];
        if (getcwd(current_dir, sizeof(current_dir))) {
            printf("Changed to: %s\n", current_dir);
        }
        chdir(original_dir);  // Change back
    } else {
        perror("chdir");
    }
    
    // Example 6: Working with symbolic links (if supported)
    printf("\n=== Symbolic Links ===\n");
    
    const char *target = "test_directory";
    const char *linkname = "test_link";
    
    if (symlink(target, linkname) == 0) {
        printf("Created symbolic link: %s -> %s\n", linkname, target);
        
        // Read symlink
        char link_target[256];
        ssize_t len = readlink(linkname, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf("Symlink points to: %s\n", link_target);
        }
    } else {
        perror("symlink");
    }
    
    // Example 7: Path operations
    printf("\n=== Path Operations ===\n");
    
    const char *filepath = "test_directory/file.txt";
    char path_copy[256];
    strcpy(path_copy, filepath);
    printf("Full path: %s\n", filepath);
    printf("Directory: %s\n", dirname(path_copy));
    strcpy(path_copy, filepath);
    printf("Filename: %s\n", basename(path_copy));
    
    // Example 8: Temporary files
    printf("\n=== Temporary Files ===\n");
    
    char temp_template[] = "/tmp/tmpfileXXXXXX";
    int temp_fd = mkstemp(temp_template);
    if (temp_fd != -1) {
        printf("Created temporary file: %s\n", temp_template);
        write(temp_fd, "Temporary data", 14);
        close(temp_fd);
        unlink(temp_template);
        printf("Deleted temporary file\n");
    } else {
        perror("mkstemp");
    }
    
    // Cleanup
    printf("\n=== Cleanup ===\n");
    unlink(linkname);
    rmdir("test_directory/subdir1/subdir2");
    rmdir("test_directory/subdir1");
    rmdir("test_directory");
    printf("Cleaned up test directories\n");
    
    return 0;
}
