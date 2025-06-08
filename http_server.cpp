#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

// Read file content into a string
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Send HTTP response
void sendResponse(int client_socket, const std::string& status, const std::string& content_type, const std::string& content) {
    std::stringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << content;

    std::string response_str = response.str();
    send(client_socket, response_str.c_str(), response_str.length(), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed" << std::endl;
        return -1;
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Accept client connection
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // Read client request
        read(client_socket, buffer, BUFFER_SIZE);
        std::string request(buffer);

        // Parse request (basic GET request handling)
        if (request.find("GET / ") != std::string::npos || request.find("GET /index.html") != std::string::npos) {
            std::string content = readFile("index.html");
            if (!content.empty()) {
                sendResponse(client_socket, "200 OK", "text/html", content);
            } else {
                sendResponse(client_socket, "404 Not Found", "text/html", "<h1>404 Not Found</h1>");
            }
        } else {
            sendResponse(client_socket, "404 Not Found", "text/html", "<h1>404 Not Found</h1>");
        }

        // Close client socket
        close(client_socket);
    }

    // Close server socket (unreachable in this example)
    close(server_fd);
    return 0;
}