CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = -lm

# All programs
PROGRAMS = file_io process_management signal_handling pipes_ipc system_info memory_management directory_ops

# Default target
all: $(PROGRAMS)

# Individual targets
file_io: file_io.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

process_management: process_management.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

signal_handling: signal_handling.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

pipes_ipc: pipes_ipc.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

system_info: system_info.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

memory_management: memory_management.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

directory_ops: directory_ops.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean up generated files
clean:
	rm -f $(PROGRAMS)
	rm -f *.o core
	rm -f test.txt test_mmap.txt test_link
	rm -rf test_directory

# Help target
help:
	@echo "Linux System Programming Examples - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all               - Build all examples"
	@echo "  file_io           - File I/O operations"
	@echo "  process_management- Process management and forking"
	@echo "  signal_handling   - Signal handling"
	@echo "  pipes_ipc         - Pipes and IPC"
	@echo "  system_info       - System information"
	@echo "  memory_management - Memory allocation and MMaps"
	@echo "  directory_ops     - Directory operations"
	@echo "  clean             - Remove all generated files"
	@echo "  help              - Show this help message"
	@echo ""
	@echo "Example usage:"
	@echo "  make               # Build all examples"
	@echo "  make file_io       # Build specific example"
	@echo "  ./file_io          # Run example"
	@echo "  make clean         # Clean up"

.PHONY: all clean help
