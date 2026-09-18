#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

/**
 * ============================================================================
 * C Socket Programming Examples
 * Production-ready implementations with error handling
 * ============================================================================
 */

// ============================================================================
// 1. SIMPLE TCP SERVER
// ============================================================================

/**
 * Simple Echo Server - Single threaded, one client at a time
 * 
 * Compile: gcc -o echo_server echo_server.c
 * Run:     ./echo_server
 * Test:    telnet localhost 8080
 */

void simple_echo_server(void) {
    int listen_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[4096];
    int n;
    
    // Create socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return;
    }
    
    // Enable address reuse (allows immediate restart)
    int enable = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        close(listen_fd);
        return;
    }
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on all interfaces
    
    // Bind socket
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        return;
    }
    
    printf("Server listening on port 8080...\n");
    
    // Listen for connections
    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        return;
    }
    
    // Accept and serve connections
    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        
        // Get client information
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        printf("New connection from %s:%d\n", client_ip, client_port);
        
        // Echo loop
        while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
            printf("Received %d bytes: %.*s\n", n, n, buffer);
            if (send(client_fd, buffer, n, 0) == -1) {
                perror("send");
                break;
            }
        }
        
        if (n == 0) {
            printf("Connection closed by client\n");
        } else if (n == -1) {
            perror("recv");
        }
        
        close(client_fd);
    }
    
    close(listen_fd);
}

// ============================================================================
// 2. SIMPLE TCP CLIENT
// ============================================================================

/**
 * Simple Client - Connects and sends data
 * 
 * Compile: gcc -o tcp_client tcp_client.c
 * Run:     ./tcp_client <host> <port> <message>
 */

int simple_tcp_client(const char *host, const char *port_str, const char *message) {
    int sockfd;
    struct sockaddr_in server_addr;
    int port = atoi(port_str);
    char buffer[4096];
    int n;
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }
    
    // Set connection timeout (5 seconds)
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert address
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(sockfd);
        return -1;
    }
    
    // Connect to server
    printf("Connecting to %s:%d...\n", host, port);
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    
    printf("Connected!\n");
    
    // Send message
    printf("Sending: %s\n", message);
    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("send");
        close(sockfd);
        return -1;
    }
    
    // Receive response
    n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n == -1) {
        perror("recv");
        close(sockfd);
        return -1;
    }
    
    if (n == 0) {
        printf("Connection closed by server\n");
    } else {
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);
    }
    
    close(sockfd);
    return 0;
}

// ============================================================================
// 3. MULTITHREADED TCP SERVER
// ============================================================================

/**
 * Multithreaded Echo Server - Handles multiple clients concurrently
 * Each client connection handled in separate thread
 */

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} client_info_t;

void *handle_client(void *arg) {
    client_info_t *info = (client_info_t *)arg;
    int client_fd = info->client_fd;
    char client_ip[INET_ADDRSTRLEN];
    int client_port = ntohs(info->client_addr.sin_port);
    char buffer[4096];
    int n;
    
    inet_ntop(AF_INET, &info->client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("[Thread %ld] Client connected: %s:%d\n", pthread_self(), client_ip, client_port);
    
    // Echo loop
    while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        printf("[Thread %ld] Received %d bytes\n", pthread_self(), n);
        if (send(client_fd, buffer, n, 0) == -1) {
            perror("send");
            break;
        }
    }
    
    printf("[Thread %ld] Client disconnected\n", pthread_self());
    close(client_fd);
    free(info);
    
    pthread_exit(NULL);
}

void multithreaded_echo_server(void) {
    int listen_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread_id;
    
    // Create socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return;
    }
    
    // Enable address reuse
    int enable = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Bind and listen
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 ||
        listen(listen_fd, 5) == -1) {
        perror("bind/listen");
        close(listen_fd);
        return;
    }
    
    printf("Multithreaded server listening on port 8080...\n");
    
    // Accept and handle connections
    while (1) {
        client_info_t *info = malloc(sizeof(client_info_t));
        if (!info) {
            fprintf(stderr, "Memory allocation failed\n");
            continue;
        }
        
        client_len = sizeof(client_addr);
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd == -1) {
            perror("accept");
            free(info);
            continue;
        }
        
        info->client_fd = client_fd;
        info->client_addr = client_addr;
        
        // Create thread to handle client
        if (pthread_create(&thread_id, NULL, handle_client, info) != 0) {
            perror("pthread_create");
            close(client_fd);
            free(info);
        } else {
            // Detach thread (automatic cleanup)
            pthread_detach(thread_id);
        }
    }
    
    close(listen_fd);
}

// ============================================================================
// 4. UDP SERVER & CLIENT
// ============================================================================

/**
 * Simple UDP Echo Server
 */
void udp_echo_server(void) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[4096];
    int n;
    char client_ip[INET_ADDRSTRLEN];
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return;
    }
    
    // Enable address reuse
    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8081);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(sockfd);
        return;
    }
    
    printf("UDP Echo Server listening on port 8081...\n");
    
    // Receive and echo datagrams
    while (1) {
        client_len = sizeof(client_addr);
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                    (struct sockaddr *)&client_addr, &client_len);
        
        if (n == -1) {
            perror("recvfrom");
            continue;
        }
        
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Received from %s:%d: %d bytes\n", client_ip, 
               ntohs(client_addr.sin_port), n);
        
        // Send back to client
        if (sendto(sockfd, buffer, n, 0,
                  (struct sockaddr *)&client_addr, client_len) == -1) {
            perror("sendto");
        }
    }
    
    close(sockfd);
}

/**
 * Simple UDP Client
 */
int udp_client(const char *host, const char *port_str, const char *message) {
    int sockfd;
    struct sockaddr_in server_addr;
    int port = atoi(port_str);
    char buffer[4096];
    int n;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }
    
    // Set receive timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    // Prepare server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(sockfd);
        return -1;
    }
    
    // Send message
    printf("Sending UDP message to %s:%d\n", host, port);
    if (sendto(sockfd, message, strlen(message), 0,
              (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(sockfd);
        return -1;
    }
    
    // Receive response
    n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (n == -1) {
        perror("recvfrom");
        close(sockfd);
        return -1;
    }
    
    buffer[n] = '\0';
    printf("Received: %s\n", buffer);
    
    close(sockfd);
    return 0;
}

// ============================================================================
// 5. NONBLOCKING SOCKET EXAMPLE
// ============================================================================

#include <fcntl.h>

/**
 * Set socket to non-blocking mode
 */
int set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }
    
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }
    
    return 0;
}

/**
 * Non-blocking client with timeout
 */
int nonblocking_connect(const char *host, int port, int timeout_sec) {
    int sockfd;
    struct sockaddr_in addr;
    int flags;
    fd_set write_set;
    struct timeval timeout;
    int error;
    socklen_t error_len;
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }
    
    // Set non-blocking
    if (set_nonblocking(sockfd) == -1) {
        close(sockfd);
        return -1;
    }
    
    // Prepare address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    
    // Attempt connection
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (errno != EINPROGRESS) {
            perror("connect");
            close(sockfd);
            return -1;
        }
    }
    
    // Wait for connection with timeout
    FD_ZERO(&write_set);
    FD_SET(sockfd, &write_set);
    
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;
    
    int select_result = select(sockfd + 1, NULL, &write_set, NULL, &timeout);
    
    if (select_result == -1) {
        perror("select");
        close(sockfd);
        return -1;
    } else if (select_result == 0) {
        fprintf(stderr, "Connection timeout\n");
        close(sockfd);
        return -1;
    }
    
    // Check if connection was successful
    error_len = sizeof(error);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &error_len) == -1) {
        perror("getsockopt SO_ERROR");
        close(sockfd);
        return -1;
    }
    
    if (error != 0) {
        fprintf(stderr, "Connection failed: %s\n", strerror(error));
        close(sockfd);
        return -1;
    }
    
    printf("Connected successfully using non-blocking socket\n");
    return sockfd;
}

// ============================================================================
// 6. SELECT-BASED MULTIPLEXING SERVER
// ============================================================================

/**
 * Server handling multiple clients using select()
 * Demonstrates I/O multiplexing for concurrent clients
 */
void select_based_server(void) {
    int listen_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    fd_set read_set, write_set;
    int max_fd;
    char buffer[4096];
    int n;
    
    // Array to track client file descriptors
    int clients[FD_SETSIZE];
    int num_clients = 0;
    
    memset(clients, -1, sizeof(clients));
    
    // Create listening socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return;
    }
    
    int enable = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    // Set to non-blocking
    set_nonblocking(listen_fd);
    
    // Prepare address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Bind and listen
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 ||
        listen(listen_fd, 5) == -1) {
        perror("bind/listen");
        close(listen_fd);
        return;
    }
    
    printf("Select-based server listening on port 8080...\n");
    printf("Handles up to %d clients\n", FD_SETSIZE);
    
    // Main loop
    while (1) {
        FD_ZERO(&read_set);
        FD_SET(listen_fd, &read_set);
        
        max_fd = listen_fd;
        
        // Add connected clients to read set
        for (int i = 0; i < FD_SETSIZE && i < num_clients; i++) {
            if (clients[i] != -1) {
                FD_SET(clients[i], &read_set);
                if (clients[i] > max_fd) {
                    max_fd = clients[i];
                }
            }
        }
        
        // Wait for activity
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_set, NULL, NULL, &timeout);
        
        if (activity == -1) {
            perror("select");
            break;
        }
        
        if (activity == 0) {
            printf("Select timeout\n");
            continue;
        }
        
        // Check listening socket
        if (FD_ISSET(listen_fd, &read_set)) {
            client_len = sizeof(client_addr);
            client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            
            if (client_fd != -1 && num_clients < FD_SETSIZE) {
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                printf("New client: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
                
                clients[num_clients++] = client_fd;
            }
        }
        
        // Check connected clients
        for (int i = 0; i < num_clients; i++) {
            if (clients[i] != -1 && FD_ISSET(clients[i], &read_set)) {
                n = recv(clients[i], buffer, sizeof(buffer), 0);
                
                if (n <= 0) {
                    printf("Client disconnected\n");
                    close(clients[i]);
                    // Remove from array
                    memmove(&clients[i], &clients[i+1], (num_clients - i - 1) * sizeof(int));
                    num_clients--;
                    i--;
                } else {
                    printf("Received %d bytes from client %d\n", n, i);
                    send(clients[i], buffer, n, 0);
                }
            }
        }
    }
    
    close(listen_fd);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i] != -1) {
            close(clients[i]);
        }
    }
}

// ============================================================================
// 7. BANDWIDTH TEST
// ============================================================================

/**
 * Measures throughput between server and client
 */
void bandwidth_test_server(int port) {
    int listen_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    unsigned char buffer[65536];
    int n;
    
    // Create socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return;
    }
    
    // Increase buffer sizes for bandwidth test
    int buf_size = 1024 * 1024;  // 1MB
    setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
    setsockopt(listen_fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
    
    int enable = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    // Bind and listen
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 ||
        listen(listen_fd, 1) == -1) {
        perror("bind/listen");
        close(listen_fd);
        return;
    }
    
    printf("Bandwidth test server on port %d\n", port);
    printf("Waiting for client...\n");
    
    client_len = sizeof(client_addr);
    client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (client_fd == -1) {
        perror("accept");
        close(listen_fd);
        return;
    }
    
    printf("Client connected. Receiving data...\n");
    
    unsigned long long bytes_received = 0;
    time_t start_time = time(NULL);
    
    // Receive data for 10 seconds
    while (time(NULL) - start_time < 10) {
        n = recv(client_fd, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        bytes_received += n;
    }
    
    time_t elapsed = time(NULL) - start_time;
    double throughput = (double)bytes_received / (1024 * 1024) / elapsed;  // MB/s
    
    printf("Throughput: %.2f MB/s (%llu bytes in %ld seconds)\n",
           throughput, bytes_received, elapsed);
    
    close(client_fd);
    close(listen_fd);
}

/**
 * Bandwidth test client
 */
int bandwidth_test_client(const char *host, int port, int duration_sec) {
    int sockfd;
    struct sockaddr_in addr;
    unsigned char buffer[65536];
    int buf_size = 1024 * 1024;  // 1MB
    
    memset(buffer, 0, sizeof(buffer));
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }
    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    
    printf("Connecting to %s:%d...\n", host, port);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    
    printf("Connected. Sending data for %d seconds...\n", duration_sec);
    
    unsigned long long bytes_sent = 0;
    time_t start_time = time(NULL);
    
    while (time(NULL) - start_time < duration_sec) {
        int n = send(sockfd, buffer, sizeof(buffer), 0);
        if (n == -1) {
            perror("send");
            break;
        }
        bytes_sent += n;
    }
    
    time_t elapsed = time(NULL) - start_time;
    double throughput = (double)bytes_sent / (1024 * 1024) / elapsed;
    
    printf("Sent: %.2f MB/s (%llu bytes in %ld seconds)\n",
           throughput, bytes_sent, elapsed);
    
    close(sockfd);
    return 0;
}

// ============================================================================
// MAIN - Demonstrate all examples
// ============================================================================

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <example>\n", argv[0]);
        printf("Examples:\n");
        printf("  1. simple_server     - Simple echo server\n");
        printf("  2. simple_client     - Simple client\n");
        printf("  3. multi_server      - Multithreaded server\n");
        printf("  4. udp_server        - UDP echo server\n");
        printf("  5. udp_client        - UDP client\n");
        printf("  6. select_server     - Select-based multiplexing server\n");
        printf("  7. bandwidth_server  - Bandwidth test server\n");
        printf("  8. bandwidth_client  - Bandwidth test client\n");
        return 1;
    }
    
    const char *example = argv[1];
    
    if (strcmp(example, "simple_server") == 0) {
        simple_echo_server();
    } else if (strcmp(example, "simple_client") == 0) {
        if (argc < 5) {
            printf("Usage: %s simple_client <host> <port> <message>\n", argv[0]);
            return 1;
        }
        simple_tcp_client(argv[2], argv[3], argv[4]);
    } else if (strcmp(example, "multi_server") == 0) {
        multithreaded_echo_server();
    } else if (strcmp(example, "udp_server") == 0) {
        udp_echo_server();
    } else if (strcmp(example, "udp_client") == 0) {
        if (argc < 5) {
            printf("Usage: %s udp_client <host> <port> <message>\n", argv[0]);
            return 1;
        }
        udp_client(argv[2], argv[3], argv[4]);
    } else if (strcmp(example, "select_server") == 0) {
        select_based_server();
    } else if (strcmp(example, "bandwidth_server") == 0) {
        int port = argc > 2 ? atoi(argv[2]) : 9999;
        bandwidth_test_server(port);
    } else if (strcmp(example, "bandwidth_client") == 0) {
        if (argc < 4) {
            printf("Usage: %s bandwidth_client <host> <port> [duration]\n", argv[0]);
            return 1;
        }
        int duration = argc > 4 ? atoi(argv[4]) : 10;
        bandwidth_test_client(argv[2], atoi(argv[3]), duration);
    } else {
        printf("Unknown example: %s\n", example);
        return 1;
    }
    
    return 0;
}

// Compilation:
// gcc -o socket_examples socket_examples.c -pthread -Wall -Wextra
//
// Examples:
// ./socket_examples simple_server
// ./socket_examples simple_client 127.0.0.1 8080 "Hello"
// ./socket_examples multi_server
// ./socket_examples udp_server
// ./socket_examples select_server
// ./socket_examples bandwidth_server 9999
// ./socket_examples bandwidth_client 127.0.0.1 9999 10
