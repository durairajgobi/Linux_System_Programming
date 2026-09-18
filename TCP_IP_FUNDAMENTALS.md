# Socket Programming & TCP/IP Concepts - Complete Guide

## Table of Contents
1. TCP/IP Model Overview
2. Socket Fundamentals
3. C Socket Programming
4. C++ Socket Programming
5. Client-Server Architecture
6. Advanced Concepts
7. Best Practices

---

## 1. TCP/IP Model Overview

### OSI vs TCP/IP Model

```
TCP/IP Stack (4 Layers):
┌─────────────────────────────────┐
│  Application Layer              │  (HTTP, HTTPS, FTP, DNS, SMTP, SSH)
├─────────────────────────────────┤
│  Transport Layer                │  (TCP, UDP, SCTP)
├─────────────────────────────────┤
│  Internet Layer                 │  (IP, ICMP, IGMP)
├─────────────────────────────────┤
│  Link Layer                     │  (Ethernet, WiFi, PPP)
└─────────────────────────────────┘
```

### Transport Protocols

#### TCP (Transmission Control Protocol)
- **Connection-oriented**: Establishes connection before data transfer
- **Reliable**: Guarantees data delivery in order
- **Flow control**: Manages data transmission rate
- **Congestion control**: Adapts to network conditions
- **Header size**: 20-60 bytes
- **Use cases**: Web (HTTP), Email (SMTP, POP3), File Transfer (FTP), Secure Shell (SSH)

#### UDP (User Datagram Protocol)
- **Connectionless**: No connection establishment
- **Unreliable**: No delivery guarantees
- **Low latency**: Minimal overhead
- **Header size**: 8 bytes
- **Use cases**: DNS, Video streaming, Online gaming, VoIP, IoT

### IP Addressing

```
IPv4 Address Format: 192.168.1.1 (32-bit)
┌──────────┬──────────┬──────────┬──────────┐
│ 11000000 │ 10101000 │ 00000001 │ 00000001 │
│   192    │   168    │    1     │    1     │
└──────────┴──────────┴──────────┴──────────┘

Common Private Ranges:
- 10.0.0.0 to 10.255.255.255         (Class A)
- 172.16.0.0 to 172.31.255.255       (Class B)
- 192.168.0.0 to 192.168.255.255     (Class C)
- 127.0.0.1                          (Loopback)
```

### Port Numbers

```
Port Range: 0-65535 (16-bit)

Well-Known Ports (0-1023):
- 21: FTP (File Transfer Protocol)
- 22: SSH (Secure Shell)
- 23: Telnet
- 25: SMTP (Simple Mail Transfer Protocol)
- 53: DNS (Domain Name System)
- 80: HTTP (Hypertext Transfer Protocol)
- 110: POP3 (Post Office Protocol v3)
- 143: IMAP (Internet Message Access Protocol)
- 443: HTTPS (HTTP Secure)
- 3306: MySQL
- 5432: PostgreSQL

Registered Ports (1024-49151):
- 1433: SQL Server
- 3389: RDP (Remote Desktop Protocol)
- 5000-5900: Application-specific

Dynamic/Private Ports (49152-65535):
- Assigned dynamically by operating system
```

---

## 2. Socket Fundamentals

### What is a Socket?

A **socket** is an endpoint for network communication—a combination of IP address, protocol type (TCP/UDP), and port number.

```
Socket = (IP Address : Port, Protocol Type)
Example: (192.168.1.100:8080, TCP)
```

### Socket Types

#### SOCK_STREAM (TCP)
```
Characteristics:
✓ Connection-oriented
✓ Reliable ordered delivery
✓ Bidirectional communication
✓ Flow control
✓ Error checking

Use: Web servers, Email, File transfer, Remote login
```

#### SOCK_DGRAM (UDP)
```
Characteristics:
✓ Connectionless
✓ Unreliable delivery
✓ Lower latency
✓ Lower overhead
✓ Datagram-based

Use: DNS, Video/Audio streaming, Gaming, IoT
```

#### SOCK_RAW
```
Characteristics:
✓ Direct IP protocol access
✓ Custom protocol implementation
✓ System privileges required

Use: ICMP (ping), custom protocols, network diagnostics
```

### TCP Connection States

```
TCP State Machine:

Client Side:                    Server Side:
─────────────                   ───────────

CLOSED                          LISTEN
  │                               │
  │ connect()                      │ accept()
  ↓                               ↓
SYN_SENT ─────────────→──────── SYN_RECEIVED
  │                               │
  │ SYN-ACK ←──────────────────── │
  │                               │
  ↓                               │
ESTABLISHED ←───────────────────→ ESTABLISHED
  │                               │
  │ close()                        │
  ↓                               ↓
FIN_WAIT_1 ──────────────→────── CLOSE_WAIT
  │                               │
  │ FIN ←──────────────────────── │ close()
  │                               ↓
  ↓                          LAST_ACK
TIME_WAIT ←───────────────────────┘
  │                               
  ↓ (After timeout)               
CLOSED                            
```

### 3-Way Handshake (TCP Connection Establishment)

```
Client                              Server
  │                                   │
  │──────────SYN (seq=x)──────────→   │
  │         [SYN_SENT]           [LISTEN]
  │                                   │
  │←─────SYN-ACK (seq=y, ack=x+1)──   │
  │         [SYN_RCVD]        [ESTABLISHED]
  │                                   │
  │──────ACK (seq=x+1, ack=y+1)──→    │
  │      [ESTABLISHED]                │
  │                                   │

After 3-way handshake: Connection established, data transfer can begin
```

### TCP Connection Termination (4-Way Handshake)

```
Client                              Server
  │                                   │
  │──────────FIN (seq=x)──────────→   │
  │         [FIN_WAIT_1]         [CLOSE_WAIT]
  │                                   │
  │←──────ACK (ack=x+1)──────────     │
  │         [FIN_WAIT_2]              │
  │                                   │
  │← ─────FIN (seq=y)──────────────   │
  │      [FIN_WAIT_2]           [LAST_ACK]
  │                                   │
  │──────ACK (ack=y+1)──────────→     │
  │       [TIME_WAIT]              [CLOSED]
  │                                   │
  ↓ (After timeout)
 CLOSED
```

### Socket Address Structures

#### IPv4 (sockaddr_in)
```c
struct sockaddr_in {
    short sin_family;              // AF_INET
    unsigned short sin_port;       // Port (network byte order)
    struct in_addr sin_addr;       // IP address (network byte order)
    char sin_zero[8];              // Padding (set to 0)
};

struct in_addr {
    unsigned long s_addr;          // 32-bit IPv4 address
};
```

#### IPv6 (sockaddr_in6)
```c
struct sockaddr_in6 {
    sa_family_t sin6_family;       // AF_INET6
    in_port_t sin6_port;           // Port (network byte order)
    uint32_t sin6_flowinfo;        // Flow info
    struct in6_addr sin6_addr;     // 128-bit IPv6 address
    uint32_t sin6_scope_id;        // Scope ID
};
```

#### Generic (sockaddr)
```c
struct sockaddr {
    unsigned short sa_family;      // Address family (AF_INET, AF_INET6)
    char sa_data[14];              // Protocol-specific address
};
```

---

## 3. Core Socket Functions

### Connection-Oriented (TCP) Flow

#### Server Side
```
socket()      → Create socket
bind()        → Bind to local address
listen()      → Mark socket as listening
accept()      → Accept incoming connection
recv()        → Receive data
send()        → Send data
close()       → Close connection
```

#### Client Side
```
socket()      → Create socket
connect()     → Connect to server
send()        → Send data
recv()        → Receive data
close()       → Close connection
```

### Connectionless (UDP) Flow

#### Server Side
```
socket()      → Create socket
bind()        → Bind to local address
recvfrom()    → Receive data from client
sendto()      → Send data to client
close()       → Close socket
```

#### Client Side
```
socket()      → Create socket
sendto()      → Send data to server
recvfrom()    → Receive data from server
close()       → Close socket
```

### Key Socket Functions

#### socket()
```c
int socket(int domain, int type, int protocol);

domain:   AF_INET (IPv4), AF_INET6 (IPv6), AF_UNIX (Local)
type:     SOCK_STREAM (TCP), SOCK_DGRAM (UDP), SOCK_RAW
protocol: 0 (automatic), IPPROTO_TCP, IPPROTO_UDP

Returns: File descriptor (>=0) on success, -1 on error
```

#### bind()
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

sockfd:   Socket file descriptor
addr:     Address structure (sockaddr_in, sockaddr_in6)
addrlen:  Length of address structure

Returns: 0 on success, -1 on error
```

#### listen()
```c
int listen(int sockfd, int backlog);

sockfd:   Socket file descriptor
backlog:  Maximum pending connections (typically 5-128)

Returns: 0 on success, -1 on error
```

#### accept()
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

sockfd:   Listening socket file descriptor
addr:     Client address (filled by function)
addrlen:  Address length (input/output parameter)

Returns: New socket file descriptor on success, -1 on error
```

#### connect()
```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

sockfd:   Socket file descriptor
addr:     Server address structure
addrlen:  Length of address structure

Returns: 0 on success, -1 on error
```

#### send() / recv()
```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

flags:
  0               Default behavior
  MSG_DONTWAIT    Non-blocking
  MSG_DONTROUTE   Don't route packet
  MSG_NOSIGNAL    Don't send SIGPIPE (TCP)

Returns: Number of bytes sent/received, -1 on error, 0 on EOF (recv)
```

#### sendto() / recvfrom()
```c
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

Returns: Number of bytes sent/received, -1 on error
```

#### close()
```c
int close(int sockfd);

Returns: 0 on success, -1 on error
```

#### shutdown()
```c
int shutdown(int sockfd, int how);

how:
  SHUT_RD   (0) No more receiving
  SHUT_WR   (1) No more sending
  SHUT_RDWR (2) No more sending or receiving

Returns: 0 on success, -1 on error
```

---

## 4. Byte Order Conversions

Network byte order is **Big-Endian** (most significant byte first).

### Host to Network Conversions

```c
// 32-bit integer (IP address)
uint32_t htonl(uint32_t hostlong);    // Host to Network Long
uint32_t ntohl(uint32_t netlong);     // Network to Host Long

// 16-bit integer (Port number)
uint16_t htons(uint16_t hostshort);   // Host to Network Short
uint16_t ntohs(uint16_t netshort);    // Network to Host Short
```

### Address Conversion

```c
// IPv4 address string to binary
int inet_pton(int af, const char *src, void *dst);
// af: AF_INET or AF_INET6
// Returns: 1 on success, 0 on invalid input, -1 on error

// Binary IPv4 address to string
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
// Returns: Pointer to dst on success, NULL on error

// Legacy (IPv4 only)
unsigned long inet_addr(const char *cp);           // String to binary
char *inet_ntoa(struct in_addr in);                // Binary to string
```

---

## 5. Error Handling

### errno Values

```c
#include <errno.h>
#include <string.h>

int main() {
    if (socket(...) == -1) {
        // Print human-readable error message
        perror("socket");           // Prints: socket: Connection refused
        
        // Or use strerror
        fprintf(stderr, "Error: %s\n", strerror(errno));
        
        // Check specific error
        if (errno == EADDRINUSE) {
            printf("Address already in use\n");
        }
    }
}

Common errno values:
- ECONNREFUSED: Connection refused
- ECONNRESET: Connection reset by peer
- ETIMEDOUT: Connection timed out
- EHOSTUNREACH: No route to host
- ENETUNREACH: Network unreachable
- EADDRINUSE: Address already in use
- EACCES: Permission denied
- EAGAIN: Resource temporarily unavailable
- EWOULDBLOCK: Operation would block
```

### Socket Options

```c
// Get socket option
int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen);

// Set socket option
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen);

level: SOL_SOCKET, IPPROTO_TCP, IPPROTO_IP
optname: SO_REUSEADDR, SO_REUSEPORT, SO_KEEPALIVE, SO_LINGER, etc.
```

### Common Socket Options

```c
// Allow socket address reuse (useful after server restart)
int enable = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

// Enable TCP keep-alive
setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));

// Set receive buffer size
int buf_size = 65536;
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size));

// Set send buffer size
setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));

// Set socket timeout (recv/send)
struct timeval tv;
tv.tv_sec = 5;      // 5 seconds
tv.tv_usec = 0;
setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

// Set TCP_NODELAY (disable Nagle algorithm for low-latency)
setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
```

---

## 6. Multiplexing & Asynchronous I/O

### select()
```c
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);

// Macro operations
FD_ZERO(&set);              // Initialize set
FD_SET(fd, &set);           // Add fd to set
FD_CLR(fd, &set);           // Remove fd from set
FD_ISSET(fd, &set);         // Check if fd in set
```

### poll()
```c
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

struct pollfd {
    int fd;                 // File descriptor
    short events;           // Requested events (POLLIN, POLLOUT, etc.)
    short revents;          // Returned events
};
```

### epoll() (Linux)
```c
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents,
               int timeout);
```

---

## 7. Performance Considerations

### Nagle's Algorithm
- Groups small packets into larger ones
- Reduces network overhead
- May increase latency
- Disable with TCP_NODELAY for interactive apps

### Buffer Sizing
```
Send buffer: SO_SNDBUF (default ~128KB)
Receive buffer: SO_RCVBUF (default ~128KB)
Application buffer: 4KB-64KB typical
```

### Connection Pooling
- Reuse existing connections
- Reduce connection establishment overhead
- Typical pool size: 10-100 connections

### Bandwidth Calculation
```
Bandwidth = (Data bytes transmitted) / (Time elapsed)

Throughput test:
1. Send N bytes
2. Measure time
3. Calculate: throughput = N / time (bytes/sec)
```

---

## Best Practices

1. **Always check return values** - Every socket function can fail
2. **Use SO_REUSEADDR** - Allows immediate socket reuse after close
3. **Set socket timeouts** - Prevent indefinite blocking
4. **Graceful shutdown** - Use shutdown() before close()
5. **Resource cleanup** - Close sockets even on error
6. **Non-blocking I/O** - Use for responsive applications
7. **Multiplexing** - Use select/poll/epoll for many connections
8. **Error handling** - Distinguish between recoverable and fatal errors
9. **Logging** - Log connection events for debugging
10. **Security** - Use TLS/SSL for sensitive data

---

## Quick Reference

### IPv4 TCP Server Template
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(sockfd, 5);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        
        char buf[1024];
        int n = recv(client_sock, buf, sizeof(buf), 0);
        send(client_sock, buf, n, 0);
        
        close(client_sock);
    }
    
    close(sockfd);
    return 0;
}
```

### IPv4 TCP Client Template
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    
    const char *msg = "Hello Server";
    send(sockfd, msg, strlen(msg), 0);
    
    char buf[1024];
    int n = recv(sockfd, buf, sizeof(buf), 0);
    printf("Received: %.*s\n", n, buf);
    
    close(sockfd);
    return 0;
}
```

This comprehensive guide provides the foundation for understanding and implementing socket programming in both C and C++.
