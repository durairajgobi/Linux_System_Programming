# Socket Programming - Quick Reference & Common Patterns

## Table of Contents
1. Quick Reference Tables
2. Common Patterns
3. Debugging Tips
4. Performance Tuning
5. Security Considerations
6. Recipes & Examples

---

## 1. Quick Reference Tables

### Socket Function Summary

| Function | Purpose | Parameters | Returns |
|----------|---------|-----------|---------|
| `socket()` | Create socket | domain, type, protocol | fd or -1 |
| `bind()` | Bind to address | fd, addr, addrlen | 0 or -1 |
| `listen()` | Mark as listening | fd, backlog | 0 or -1 |
| `accept()` | Accept connection | fd, addr, addrlen | new fd or -1 |
| `connect()` | Connect to server | fd, addr, addrlen | 0 or -1 |
| `send()` | Send data | fd, buf, len, flags | bytes or -1 |
| `recv()` | Receive data | fd, buf, len, flags | bytes or -1 |
| `sendto()` | Send UDP packet | fd, buf, len, flags, addr, addrlen | bytes or -1 |
| `recvfrom()` | Receive UDP packet | fd, buf, len, flags, addr, addrlen | bytes or -1 |
| `close()` | Close socket | fd | 0 or -1 |
| `shutdown()` | Disable I/O | fd, how | 0 or -1 |
| `setsockopt()` | Set option | fd, level, optname, val, len | 0 or -1 |
| `getsockopt()` | Get option | fd, level, optname, val, len | 0 or -1 |
| `select()` | Monitor sockets | nfds, read, write, except, timeout | count or -1 |
| `poll()` | Monitor sockets | fds, nfds, timeout | count or -1 |
| `epoll_wait()` | Monitor sockets (Linux) | epfd, events, maxevents, timeout | count or -1 |

### Common Socket Options

```c
// Level: SOL_SOCKET
SO_REUSEADDR    int     Enable address reuse (useful after restart)
SO_REUSEPORT    int     Enable port reuse (multiple processes)
SO_KEEPALIVE    int     Enable TCP keep-alive
SO_SNDBUF       int     Send buffer size (bytes)
SO_RCVBUF       int     Receive buffer size (bytes)
SO_SNDTIMEO     timeval Send timeout
SO_RCVTIMEO     timeval Receive timeout
SO_LINGER       linger  Linger on close

// Level: IPPROTO_TCP
TCP_NODELAY     int     Disable Nagle algorithm
TCP_MAXSEG      int     Maximum segment size
TCP_CORK        int     Coalesce packets (Linux)

// Level: IPPROTO_IP
IP_MULTICAST_IF int     Multicast interface
```

### Error Codes

```c
ECONNREFUSED    111     Connection refused (server not listening)
ECONNRESET      104     Connection reset by peer
ETIMEDOUT       110     Connection timed out
EHOSTUNREACH    113     No route to host
ENETUNREACH     101     Network unreachable
EADDRINUSE      98      Address already in use
EACCES          13      Permission denied
EAGAIN          11      Resource temporarily unavailable
EWOULDBLOCK     11      Operation would block (non-blocking)
EPIPE           32      Broken pipe
EBADF           9       Bad file descriptor
```

---

## 2. Common Patterns

### Pattern 1: Simple Echo Server

**Goal:** Accept connections and echo back all received data

**C Implementation:**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // Create listening socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) { perror("socket"); exit(1); }
    
    // Enable reuse
    int enable = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    // Bind
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind"); exit(1);
    }
    
    // Listen
    if (listen(listen_fd, 5) == -1) {
        perror("listen"); exit(1);
    }
    
    printf("Server listening on port 8080\n");
    
    // Accept loop
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) { perror("accept"); continue; }
        
        // Echo loop
        char buf[1024];
        int n;
        while ((n = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
            send(client_fd, buf, n, 0);
        }
        
        close(client_fd);
    }
    
    close(listen_fd);
    return 0;
}
```

**C++ Implementation:**
```cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

class EchoServer {
    int listen_fd;
    
public:
    EchoServer(int port) {
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1) throw std::runtime_error("socket failed");
        
        int enable = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
            throw std::runtime_error("bind failed");
        
        if (listen(listen_fd, 5) == -1)
            throw std::runtime_error("listen failed");
        
        std::cout << "Server listening on port " << port << std::endl;
    }
    
    ~EchoServer() { close(listen_fd); }
    
    void run() {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd == -1) continue;
            
            char buf[1024];
            int n;
            while ((n = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
                send(client_fd, buf, n, 0);
            }
            
            close(client_fd);
        }
    }
};

int main() {
    try {
        EchoServer server(8080);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
```

### Pattern 2: Multithreaded Server

**Goal:** Handle multiple clients concurrently

**C with pthreads:**
```c
#include <pthread.h>

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} client_info_t;

void* handle_client(void* arg) {
    client_info_t* info = (client_info_t*)arg;
    int client_fd = info->client_fd;
    
    char buf[1024];
    int n;
    while ((n = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        send(client_fd, buf, n, 0);
    }
    
    close(client_fd);
    free(info);
    return NULL;
}

// In main accept loop:
while (1) {
    client_info_t* info = malloc(sizeof(client_info_t));
    info->client_fd = accept(listen_fd, (struct sockaddr*)&info->client_addr, &addr_len);
    
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, info);
    pthread_detach(thread_id);
}
```

**C++ with std::thread:**
```cpp
void handle_client(int client_fd) {
    char buf[1024];
    int n;
    while ((n = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        send(client_fd, buf, n, 0);
    }
    close(client_fd);
}

// In main accept loop:
std::thread t(handle_client, client_fd);
t.detach();
```

### Pattern 3: Connection Timeout

**Goal:** Don't wait indefinitely for connections or responses

**C Implementation:**
```c
struct timeval tv;
tv.tv_sec = 5;      // 5 seconds
tv.tv_usec = 0;
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

char buf[1024];
int n = recv(sockfd, buf, sizeof(buf), 0);
if (n == -1 && errno == EAGAIN) {
    printf("Receive timeout\n");
}
```

**C++ Implementation:**
```cpp
socket.set_recv_timeout(5);  // 5 seconds

try {
    std::string data = socket.recv_message();
} catch (const SocketException& e) {
    if (std::string(e.what()).find("timeout") != std::string::npos) {
        std::cout << "Receive timeout" << std::endl;
    }
}
```

### Pattern 4: Graceful Shutdown

**Goal:** Close connection cleanly without losing data

**C Implementation:**
```c
// Server initiates close
shutdown(client_fd, SHUT_WR);  // Stop writing, can still read

// Read remaining data
char buf[1024];
while (recv(client_fd, buf, sizeof(buf), 0) > 0);

// Now close
close(client_fd);
```

**C++ Implementation:**
```cpp
socket.shutdown(SHUT_WR);

try {
    while (socket.recv(buf, sizeof(buf)) > 0);
} catch (const SocketException&) {
    // Expected on EOF
}
```

### Pattern 5: Connection Pool

**Goal:** Reuse connections for efficiency

```cpp
class ConnectionPool {
private:
    std::vector<std::shared_ptr<Socket>> pool;
    std::queue<int> available;
    std::mutex mutex;
    size_t max_size;
    
public:
    ConnectionPool(size_t size) : max_size(size) {
        for (size_t i = 0; i < size; i++) {
            pool.push_back(std::make_shared<Socket>());
            available.push(i);
        }
    }
    
    std::shared_ptr<Socket> acquire() {
        std::lock_guard<std::mutex> lock(mutex);
        if (available.empty()) return nullptr;
        
        int index = available.front();
        available.pop();
        return pool[index];
    }
    
    void release(int index) {
        std::lock_guard<std::mutex> lock(mutex);
        available.push(index);
    }
};
```

---

## 3. Debugging Tips

### Using tcpdump

```bash
# Capture TCP traffic on port 8080
sudo tcpdump -i any -n tcp port 8080

# Capture and display data
sudo tcpdump -i any -n -A tcp port 8080

# Save to file
sudo tcpdump -i any tcp port 8080 -w capture.pcap

# Read from file
tcpdump -r capture.pcap

# Filter specific host
sudo tcpdump -i any host 192.168.1.100 and port 8080
```

### Using strace

```bash
# Trace system calls
strace -e trace=network ./my_server

# Follow threads
strace -f -e trace=network ./my_server

# Show time between calls
strace -e trace=network -r ./my_server

# Save to file
strace -e trace=network -o trace.log ./my_server
```

### Using netstat

```bash
# Show listening sockets
netstat -tuln | grep 8080

# Show all TCP connections
netstat -tcp

# Monitor in real-time
watch -n 1 netstat -tuln
```

### Using ss (modern replacement)

```bash
# Show listening sockets
ss -tuln | grep 8080

# Show established connections
ss -tun

# Show statistics
ss -s
```

### Debugging with gdb

```bash
# Run with gdb
gdb ./my_server

# Set breakpoint
(gdb) break main
(gdb) break handle_client

# Run
(gdb) run

# Step through
(gdb) step
(gdb) next

# Print variable
(gdb) print client_fd

# Watch variable
(gdb) watch errno
```

---

## 4. Performance Tuning

### Buffer Optimization

```c
// Increase buffers for high throughput
int buf_size = 2 * 1024 * 1024;  // 2MB
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));

// Application buffer (align to page size: 4096)
char buffer[65536];  // Multiple of 4096
```

### TCP Tuning (Linux)

```bash
# Increase backlog for high-traffic servers
sysctl -w net.core.somaxconn=4096

# Increase max file descriptors
ulimit -n 1000000

# TCP buffer tuning
sysctl -w net.ipv4.tcp_rmem="4096 131072 6291456"
sysctl -w net.ipv4.tcp_wmem="4096 131072 6291456"

# Enable TCP fast open
sysctl -w net.ipv4.tcp_fastopen=3

# Tune TIME_WAIT timeout
sysctl -w net.ipv4.tcp_fin_timeout=30
```

### Multiplexing Performance

```
select()  vs  poll()  vs  epoll()
──────────────────────────────────
O(n)          O(n)        O(1)         Time complexity
1024          65536       Unlimited    Max file descriptors (typical)
Portable      Portable    Linux only   Portability
```

**Choose based on:**
- Small number of sockets (<1000): select() or poll()
- Large number of sockets (>10000): epoll() on Linux, kqueue() on BSD

---

## 5. Security Considerations

### Preventing Buffer Overflow

```c
// WRONG: No bounds checking
char buffer[256];
recv(sockfd, buffer, 1024, 0);  // Can overflow!

// CORRECT: Bounds checked
char buffer[256];
recv(sockfd, buffer, sizeof(buffer) - 1, 0);  // Safe
buffer[sizeof(buffer) - 1] = '\0';
```

### Validating Input

```cpp
// Validate before processing
std::string input = socket.recv_message();

// Check length
if (input.length() > MAX_MESSAGE_SIZE) {
    throw SocketException("Message too long");
}

// Check format
if (!std::all_of(input.begin(), input.end(), ::isalnum)) {
    throw SocketException("Invalid characters");
}
```

### Using TLS/SSL

```cpp
// C++ with OpenSSL
try {
    SecureSocket sock;
    sock.enable_tls();
    sock.connect("example.com", 443);
    sock.send_secure("GET / HTTP/1.1\r\n");
    std::string response = sock.recv_secure();
} catch (const SocketException& e) {
    std::cerr << "Secure connection failed: " << e.what() << std::endl;
}
```

### Rate Limiting

```cpp
class RateLimiter {
private:
    std::chrono::high_resolution_clock::time_point last_request;
    std::chrono::milliseconds min_interval;
    
public:
    RateLimiter(int requests_per_second) 
        : min_interval(1000 / requests_per_second) {}
    
    bool allow() {
        auto now = std::chrono::high_resolution_clock::now();
        if (now - last_request >= min_interval) {
            last_request = now;
            return true;
        }
        return false;
    }
};
```

---

## 6. Recipes & Examples

### Recipe 1: HTTP GET Request

```cpp
std::string http_get(const std::string& host, const std::string& path) {
    TCPClient client;
    client.connect(host, 80);
    
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";
    
    client.send_message(request);
    
    std::string response;
    while (true) {
        try {
            response += client.recv_message();
        } catch (const SocketException&) {
            break;
        }
    }
    
    return response;
}
```

### Recipe 2: Port Scanner

```cpp
bool is_port_open(const std::string& host, int port, int timeout_sec = 1) {
    TCPClient client;
    try {
        client.connect(host, port);
        return true;
    } catch (const SocketException&) {
        return false;
    }
}

void scan_ports(const std::string& host) {
    for (int port = 1; port <= 65535; port++) {
        if (is_port_open(host, port)) {
            std::cout << "Port " << port << " is open" << std::endl;
        }
    }
}
```

### Recipe 3: Broadcast Message

```cpp
void broadcast_message(const std::string& message, int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    int broadcast_enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, 
              sizeof(broadcast_enable));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
    sendto(sockfd, message.c_str(), message.length(), 0,
          (struct sockaddr*)&addr, sizeof(addr));
    
    close(sockfd);
}
```

### Recipe 4: DNS Lookup

```cpp
std::string dns_lookup(const std::string& hostname) {
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname.c_str(), NULL, &hints, &res) != 0) {
        throw std::runtime_error("DNS lookup failed");
    }
    
    struct sockaddr_in* addr_in = (struct sockaddr_in*)res->ai_addr;
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, INET_ADDRSTRLEN);
    
    freeaddrinfo(res);
    return std::string(ip_str);
}
```

### Recipe 5: Throughput Test

```cpp
void throughput_test_client(const std::string& host, int port, size_t data_size) {
    TCPClient client;
    client.connect(host, port);
    
    std::vector<char> buffer(data_size, 0);
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        try {
            client.send_message(std::string(buffer.begin(), buffer.end()));
        } catch (const SocketException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            break;
        }
    }
    
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    
    size_t total_bytes = data_size * 1000;
    double throughput = (double)total_bytes / (ms / 1000.0) / (1024 * 1024);
    
    std::cout << "Throughput: " << std::fixed << std::setprecision(2)
             << throughput << " MB/s" << std::endl;
}
```

---

## Compilation & Testing

### Compile C Examples
```bash
gcc -o socket_examples socket_examples.c -pthread -Wall -Wextra -lm
./socket_examples simple_server
./socket_examples simple_client 127.0.0.1 8080 "Hello"
```

### Compile C++ Examples
```bash
g++ -std=c++11 -o socket_examples_cpp socket_examples_cpp.cpp -pthread -Wall -Wextra
./socket_examples_cpp tcp_server 8080
./socket_examples_cpp tcp_client 127.0.0.1 8080 "Hello"
```

### Testing Tools
```bash
# Test with netcat
nc -l 8080
nc localhost 8080

# Test with telnet
telnet localhost 8080

# Benchmark with iperf3
iperf3 -s -p 8080        # Server
iperf3 -c localhost -p 8080  # Client
```

---

## Summary Checklist

- [ ] Always check return values of socket functions
- [ ] Set `SO_REUSEADDR` to prevent "Address already in use"
- [ ] Set timeouts to prevent indefinite blocking
- [ ] Use non-blocking sockets for interactive applications
- [ ] Implement graceful shutdown with `shutdown()` before `close()`
- [ ] Disable Nagle algorithm (`TCP_NODELAY`) for low-latency apps
- [ ] Validate and bounds-check all input data
- [ ] Use TLS/SSL for sensitive communications
- [ ] Monitor connections with appropriate logging
- [ ] Test with various network conditions (latency, packet loss)
- [ ] Profile for performance bottlenecks
- [ ] Document protocol specifications clearly
- [ ] Use connection pooling for high-throughput scenarios
- [ ] Implement proper error recovery and retry logic
- [ ] Keep socket code separate from business logic

This quick reference should cover most common socket programming scenarios in both C and C++!
