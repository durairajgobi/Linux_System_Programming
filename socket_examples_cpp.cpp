#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <exception>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

/**
 * ============================================================================
 * C++ Socket Programming Examples
 * Object-oriented design with RAII principles
 * Production-ready with comprehensive error handling
 * ============================================================================
 */

// ============================================================================
// 1. SOCKET EXCEPTION CLASS
// ============================================================================

class SocketException : public std::exception {
private:
    std::string message;
    
public:
    explicit SocketException(const std::string& msg) : message(msg) {}
    
    SocketException(const std::string& context, int error_code) {
        message = context + ": " + strerror(error_code);
    }
    
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// ============================================================================
// 2. SOCKET WRAPPER CLASS (RAII)
// ============================================================================

/**
 * Basic socket wrapper providing RAII semantics
 * Automatically closes socket on destruction
 */
class Socket {
private:
    int fd;
    
    // Prevent copying
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
public:
    explicit Socket(int socket_fd = -1) : fd(socket_fd) {}
    
    // Move constructor and assignment
    Socket(Socket&& other) noexcept : fd(other.fd) {
        other.fd = -1;
    }
    
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close_internal();
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }
    
    ~Socket() {
        close_internal();
    }
    
    // Create a new socket
    void create(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0) {
        close_internal();
        fd = socket(domain, type, protocol);
        if (fd == -1) {
            throw SocketException("socket", errno);
        }
    }
    
    // Bind socket to address
    void bind(const struct sockaddr* addr, socklen_t addrlen) {
        if (::bind(fd, addr, addrlen) == -1) {
            throw SocketException("bind", errno);
        }
    }
    
    // Listen for connections
    void listen(int backlog = 5) {
        if (::listen(fd, backlog) == -1) {
            throw SocketException("listen", errno);
        }
    }
    
    // Accept incoming connection
    Socket accept(struct sockaddr* addr = nullptr, socklen_t* addrlen = nullptr) {
        int client_fd = ::accept(fd, addr, addrlen);
        if (client_fd == -1) {
            throw SocketException("accept", errno);
        }
        return Socket(client_fd);
    }
    
    // Connect to remote address
    void connect(const struct sockaddr* addr, socklen_t addrlen) {
        if (::connect(fd, addr, addrlen) == -1) {
            throw SocketException("connect", errno);
        }
    }
    
    // Send data
    ssize_t send(const void* buf, size_t len, int flags = 0) {
        ssize_t n = ::send(fd, buf, len, flags);
        if (n == -1) {
            throw SocketException("send", errno);
        }
        return n;
    }
    
    // Receive data
    ssize_t recv(void* buf, size_t len, int flags = 0) {
        ssize_t n = ::recv(fd, buf, len, flags);
        if (n == -1) {
            throw SocketException("recv", errno);
        }
        return n;
    }
    
    // Send to (UDP)
    ssize_t sendto(const void* buf, size_t len,
                  const struct sockaddr* addr, socklen_t addrlen,
                  int flags = 0) {
        ssize_t n = ::sendto(fd, buf, len, flags, addr, addrlen);
        if (n == -1) {
            throw SocketException("sendto", errno);
        }
        return n;
    }
    
    // Receive from (UDP)
    ssize_t recvfrom(void* buf, size_t len,
                    struct sockaddr* addr, socklen_t* addrlen,
                    int flags = 0) {
        ssize_t n = ::recvfrom(fd, buf, len, flags, addr, addrlen);
        if (n == -1) {
            throw SocketException("recvfrom", errno);
        }
        return n;
    }
    
    // Shutdown socket
    void shutdown(int how = SHUT_RDWR) {
        if (fd != -1 && ::shutdown(fd, how) == -1) {
            // Ignore error in shutdown
        }
    }
    
    // Set socket option
    void setsockopt(int level, int optname, const void* optval, socklen_t optlen) {
        if (::setsockopt(fd, level, optname, optval, optlen) == -1) {
            throw SocketException("setsockopt", errno);
        }
    }
    
    // Set address reuse
    void set_reuse_addr(bool enable = true) {
        int opt = enable ? 1 : 0;
        setsockopt(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    
    // Set TCP nodelay
    void set_tcp_nodelay(bool enable = true) {
        int opt = enable ? 1 : 0;
        setsockopt(IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    }
    
    // Set receive timeout
    void set_recv_timeout(int timeout_sec) {
        struct timeval tv;
        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;
        setsockopt(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
    
    // Set send buffer size
    void set_send_buffer_size(int size) {
        setsockopt(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
    }
    
    // Set receive buffer size
    void set_recv_buffer_size(int size) {
        setsockopt(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    }
    
    // Set non-blocking mode
    void set_nonblocking(bool enable = true) {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) {
            throw SocketException("fcntl F_GETFL", errno);
        }
        
        if (enable) {
            flags |= O_NONBLOCK;
        } else {
            flags &= ~O_NONBLOCK;
        }
        
        if (fcntl(fd, F_SETFL, flags) == -1) {
            throw SocketException("fcntl F_SETFL", errno);
        }
    }
    
    // Get underlying file descriptor
    int get_fd() const { return fd; }
    
    // Check if socket is valid
    bool is_valid() const { return fd != -1; }
    
private:
    void close_internal() {
        if (fd != -1) {
            ::close(fd);
            fd = -1;
        }
    }
};

// ============================================================================
// 3. TCP SERVER CLASS
// ============================================================================

/**
 * Simple TCP Server with client handling
 */
class TCPServer {
private:
    Socket listen_socket;
    int port;
    std::vector<std::thread> threads;
    std::mutex thread_mutex;
    bool running = false;
    
public:
    explicit TCPServer(int p) : port(p) {}
    
    virtual ~TCPServer() {
        stop();
    }
    
    void start() {
        std::cout << "Starting TCP server on port " << port << std::endl;
        
        listen_socket.create(AF_INET, SOCK_STREAM, 0);
        listen_socket.set_reuse_addr(true);
        
        // Prepare address
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        listen_socket.bind((struct sockaddr*)&addr, sizeof(addr));
        listen_socket.listen(5);
        
        running = true;
        std::cout << "Server listening on port " << port << std::endl;
    }
    
    void stop() {
        running = false;
        listen_socket.shutdown();
        
        // Wait for all threads
        {
            std::lock_guard<std::mutex> lock(thread_mutex);
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }
    }
    
    void run() {
        start();
        
        while (running) {
            try {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                Socket client = listen_socket.accept(
                    (struct sockaddr*)&client_addr, &client_len);
                
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                int client_port = ntohs(client_addr.sin_port);
                
                std::cout << "New client: " << client_ip << ":" << client_port << std::endl;
                
                // Create thread to handle client
                {
                    std::lock_guard<std::mutex> lock(thread_mutex);
                    threads.emplace_back(&TCPServer::handle_client, this,
                                        std::move(client), client_ip, client_port);
                }
            } catch (const SocketException& e) {
                std::cerr << "Accept error: " << e.what() << std::endl;
            }
        }
    }
    
protected:
    // Override in derived class to handle client
    virtual void handle_client_impl(Socket& client, 
                                   const std::string& client_ip,
                                   int client_port) {
        // Default: Echo server
        char buffer[4096];
        try {
            ssize_t n;
            while ((n = client.recv(buffer, sizeof(buffer))) > 0) {
                std::cout << "Received " << n << " bytes from " 
                         << client_ip << std::endl;
                client.send(buffer, n);
            }
        } catch (const SocketException& e) {
            std::cerr << "Client error: " << e.what() << std::endl;
        }
        
        std::cout << "Client disconnected: " << client_ip << std::endl;
    }
    
private:
    void handle_client(Socket client, const std::string& ip, int port) {
        try {
            handle_client_impl(client, ip, port);
        } catch (const std::exception& e) {
            std::cerr << "Unhandled exception: " << e.what() << std::endl;
        }
    }
};

// ============================================================================
// 4. TCP CLIENT CLASS
// ============================================================================

/**
 * Simple TCP Client
 */
class TCPClient {
private:
    Socket socket;
    
public:
    void connect(const std::string& host, int port) {
        std::cout << "Connecting to " << host << ":" << port << std::endl;
        
        socket.create(AF_INET, SOCK_STREAM, 0);
        socket.set_recv_timeout(5);  // 5-second timeout
        
        // Resolve host and connect
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            throw SocketException("Invalid address: " + host);
        }
        
        socket.connect((struct sockaddr*)&addr, sizeof(addr));
        std::cout << "Connected!" << std::endl;
    }
    
    void send_message(const std::string& message) {
        std::cout << "Sending: " << message << std::endl;
        socket.send(message.c_str(), message.length());
    }
    
    std::string recv_message(size_t max_size = 4096) {
        char buffer[4096];
        ssize_t n = socket.recv(buffer, std::min(max_size, sizeof(buffer)));
        
        if (n == 0) {
            throw SocketException("Connection closed by server");
        }
        
        return std::string(buffer, n);
    }
    
    void disconnect() {
        socket.shutdown();
    }
};

// ============================================================================
// 5. UDP SERVER & CLIENT CLASSES
// ============================================================================

/**
 * UDP Server
 */
class UDPServer {
private:
    Socket socket;
    int port;
    
public:
    explicit UDPServer(int p) : port(p) {}
    
    void start() {
        std::cout << "Starting UDP server on port " << port << std::endl;
        
        socket.create(AF_INET, SOCK_DGRAM, 0);
        socket.set_reuse_addr(true);
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        socket.bind((struct sockaddr*)&addr, sizeof(addr));
        std::cout << "UDP server listening on port " << port << std::endl;
    }
    
    void run() {
        start();
        
        char buffer[4096];
        struct sockaddr_in client_addr;
        socklen_t client_len;
        
        while (true) {
            try {
                client_len = sizeof(client_addr);
                ssize_t n = socket.recvfrom(buffer, sizeof(buffer),
                                           (struct sockaddr*)&client_addr,
                                           &client_len);
                
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                int client_port = ntohs(client_addr.sin_port);
                
                std::cout << "Received " << n << " bytes from "
                         << client_ip << ":" << client_port << std::endl;
                
                // Echo back
                socket.sendto(buffer, n, (struct sockaddr*)&client_addr, client_len);
            } catch (const SocketException& e) {
                std::cerr << "UDP error: " << e.what() << std::endl;
            }
        }
    }
};

/**
 * UDP Client
 */
class UDPClient {
private:
    Socket socket;
    struct sockaddr_in server_addr;
    
public:
    void connect(const std::string& host, int port) {
        socket.create(AF_INET, SOCK_DGRAM, 0);
        socket.set_recv_timeout(5);
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
            throw SocketException("Invalid address: " + host);
        }
        
        std::cout << "UDP client ready for " << host << ":" << port << std::endl;
    }
    
    void send_message(const std::string& message) {
        socket.sendto(message.c_str(), message.length(),
                     (struct sockaddr*)&server_addr, sizeof(server_addr));
    }
    
    std::string recv_message() {
        char buffer[4096];
        ssize_t n = socket.recvfrom(buffer, sizeof(buffer), nullptr, nullptr);
        return std::string(buffer, n);
    }
};

// ============================================================================
// 6. ASYNCHRONOUS ECHO SERVER WITH SELECT
// ============================================================================

/**
 * Echo server using select() for multiplexed I/O
 */
class SelectEchoServer {
private:
    Socket listen_socket;
    int port;
    std::vector<Socket> clients;
    
public:
    explicit SelectEchoServer(int p) : port(p) {}
    
    void run() {
        std::cout << "Starting select-based echo server on port " << port << std::endl;
        
        listen_socket.create(AF_INET, SOCK_STREAM, 0);
        listen_socket.set_reuse_addr(true);
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        listen_socket.bind((struct sockaddr*)&addr, sizeof(addr));
        listen_socket.listen(5);
        
        std::cout << "Server listening. Max " << FD_SETSIZE << " clients" << std::endl;
        
        while (true) {
            fd_set read_set;
            FD_ZERO(&read_set);
            FD_SET(listen_socket.get_fd(), &read_set);
            
            int max_fd = listen_socket.get_fd();
            
            for (size_t i = 0; i < clients.size(); i++) {
                if (clients[i].is_valid()) {
                    FD_SET(clients[i].get_fd(), &read_set);
                    if (clients[i].get_fd() > max_fd) {
                        max_fd = clients[i].get_fd();
                    }
                }
            }
            
            struct timeval timeout;
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;
            
            int activity = select(max_fd + 1, &read_set, nullptr, nullptr, &timeout);
            
            if (activity == -1) {
                throw SocketException("select", errno);
            }
            
            if (activity == 0) {
                continue;
            }
            
            // Check for new connections
            if (FD_ISSET(listen_socket.get_fd(), &read_set)) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                try {
                    Socket client = listen_socket.accept(
                        (struct sockaddr*)&client_addr, &client_len);
                    
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                    std::cout << "New client: " << client_ip << std::endl;
                    
                    if (clients.size() < FD_SETSIZE) {
                        clients.push_back(std::move(client));
                    }
                } catch (const SocketException& e) {
                    std::cerr << "Accept error: " << e.what() << std::endl;
                }
            }
            
            // Check clients for data
            for (size_t i = 0; i < clients.size(); i++) {
                if (!clients[i].is_valid()) continue;
                
                if (FD_ISSET(clients[i].get_fd(), &read_set)) {
                    char buffer[4096];
                    try {
                        ssize_t n = clients[i].recv(buffer, sizeof(buffer));
                        
                        if (n <= 0) {
                            std::cout << "Client " << i << " disconnected" << std::endl;
                            clients[i] = Socket();  // Close and reset
                        } else {
                            std::cout << "Client " << i << ": " << n << " bytes" << std::endl;
                            clients[i].send(buffer, n);
                        }
                    } catch (const SocketException& e) {
                        std::cerr << "Client error: " << e.what() << std::endl;
                        clients[i] = Socket();
                    }
                }
            }
        }
    }
};

// ============================================================================
// 7. BANDWIDTH TEST
// ============================================================================

/**
 * Bandwidth test server
 */
void bandwidth_test_server_cpp(int port) {
    std::cout << "Bandwidth test server on port " << port << std::endl;
    
    TCPServer server(port);
    
    // Override to handle bandwidth test
    class BandwidthServer : public TCPServer {
    protected:
        void handle_client_impl(Socket& client, 
                               const std::string& client_ip, 
                               int client_port) override {
            unsigned char buffer[65536];
            unsigned long long bytes_received = 0;
            
            auto start_time = std::chrono::high_resolution_clock::now();
            auto timeout = std::chrono::seconds(10);
            
            try {
                while (true) {
                    ssize_t n = client.recv(buffer, sizeof(buffer));
                    if (n <= 0) break;
                    
                    bytes_received += n;
                    
                    auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
                    if (elapsed > timeout) break;
                }
            } catch (const SocketException& e) {
                // Timeout or error
            }
            
            auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
            
            if (seconds > 0) {
                double throughput = (double)bytes_received / (1024 * 1024) / seconds;
                std::cout << "Throughput: " << std::fixed << std::setprecision(2)
                         << throughput << " MB/s\n";
            }
        }
    } server_test(port);
    
    server_test.run();
}

/**
 * Bandwidth test client
 */
void bandwidth_test_client_cpp(const std::string& host, int port, int duration_sec) {
    std::cout << "Bandwidth test client" << std::endl;
    
    TCPClient client;
    client.connect(host, port);
    
    unsigned char buffer[65536];
    memset(buffer, 0, sizeof(buffer));
    
    unsigned long long bytes_sent = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Sending data for " << duration_sec << " seconds..." << std::endl;
    
    while (true) {
        try {
            client.send_message(std::string((char*)buffer, sizeof(buffer)));
            bytes_sent += sizeof(buffer);
            
            auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
            
            if (seconds >= duration_sec) break;
        } catch (const SocketException& e) {
            std::cerr << "Send error: " << e.what() << std::endl;
            break;
        }
    }
    
    auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    
    if (seconds > 0) {
        double throughput = (double)bytes_sent / (1024 * 1024) / seconds;
        std::cout << std::fixed << std::setprecision(2)
                 << "Throughput: " << throughput << " MB/s\n";
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <example>\n"
                 << "Examples:\n"
                 << "  tcp_server [port]      - TCP echo server\n"
                 << "  tcp_client <host> <port> <message>  - TCP client\n"
                 << "  udp_server [port]      - UDP echo server\n"
                 << "  udp_client <host> <port> <message>  - UDP client\n"
                 << "  select_server [port]   - Select-based server\n"
                 << "  bandwidth_server [port] - Bandwidth test server\n"
                 << "  bandwidth_client <host> <port> [duration]\n";
        return 1;
    }
    
    try {
        std::string example = argv[1];
        
        if (example == "tcp_server") {
            int port = argc > 2 ? atoi(argv[2]) : 8080;
            TCPServer server(port);
            server.run();
            
        } else if (example == "tcp_client") {
            if (argc < 5) {
                std::cerr << "Usage: tcp_client <host> <port> <message>\n";
                return 1;
            }
            TCPClient client;
            client.connect(argv[2], atoi(argv[3]));
            client.send_message(argv[4]);
            std::string response = client.recv_message();
            std::cout << "Response: " << response << std::endl;
            client.disconnect();
            
        } else if (example == "udp_server") {
            int port = argc > 2 ? atoi(argv[2]) : 8081;
            UDPServer server(port);
            server.run();
            
        } else if (example == "udp_client") {
            if (argc < 5) {
                std::cerr << "Usage: udp_client <host> <port> <message>\n";
                return 1;
            }
            UDPClient client;
            client.connect(argv[2], atoi(argv[3]));
            client.send_message(argv[4]);
            std::string response = client.recv_message();
            std::cout << "Response: " << response << std::endl;
            
        } else if (example == "select_server") {
            int port = argc > 2 ? atoi(argv[2]) : 8080;
            SelectEchoServer server(port);
            server.run();
            
        } else if (example == "bandwidth_server") {
            int port = argc > 2 ? atoi(argv[2]) : 9999;
            bandwidth_test_server_cpp(port);
            
        } else if (example == "bandwidth_client") {
            if (argc < 4) {
                std::cerr << "Usage: bandwidth_client <host> <port> [duration]\n";
                return 1;
            }
            int duration = argc > 4 ? atoi(argv[4]) : 10;
            bandwidth_test_client_cpp(argv[2], atoi(argv[3]), duration);
            
        } else {
            std::cerr << "Unknown example: " << example << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

// Compilation:
// g++ -std=c++11 -o socket_examples_cpp socket_examples_cpp.cpp -pthread -Wall -Wextra
//
// Examples:
// ./socket_examples_cpp tcp_server 8080
// ./socket_examples_cpp tcp_client 127.0.0.1 8080 "Hello"
// ./socket_examples_cpp udp_server 8081
// ./socket_examples_cpp select_server 8080
// ./socket_examples_cpp bandwidth_server 9999
// ./socket_examples_cpp bandwidth_client 127.0.0.1 9999 10
