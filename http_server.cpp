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

// 파일 내용을 읽어 string 타입 지역변수에 할당 후 반환
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

    // 오버로딩된 꺽쇠 연산자를 사용하여 주요 응답 요소를 본문에 붙임
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << content;

    std::string response_str = response.str();
    send(client_socket, response_str.c_str(), response_str.length(), 0);
}

int main() {
    int server_fd, client_socket; // 서버 파일 디스크립터(server_fd) 에 추후 소켓을 구동할수 있는 파일 디스크립터가 할당됨(file descriptor that can be used in later function calls that operate on sockets.)
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // 소켓 설정
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed" << std::endl;
        return -1;
    }

    // 서버 주소 설정
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 최초에는 무명인 소켓에 주소를 할당함
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    // server_fd를 통해 참조된 소켓이 연결 상태일때 연결 모드에 해당하는 값 반환
    if (listen(server_fd, 3) < 0) {
        // 연결 실패
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // 연결이 accept 되었는지 확인하여 연결 실패시 이하 코드 실행 없이 continue
        // 신규 클라이언트 연결 형성 이전까지 이하 블럭(block)
        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // client_socket 에서 
        read(client_socket, buffer, BUFFER_SIZE);
        std::string request(buffer);

        // Parse request (basic GET request handling)
        if (request.find("GET / ") != std::string::npos || request.find("GET /index.html") != std::string::npos) { // 문자열 찾지 못할 시 std::string::npos 반환s
            std::string content = readFile("index.html");
            if (!content.empty()) {
                // 정상
                sendResponse(client_socket, "200 OK", "text/html", content);
            } else {
                // 파일을 열수 없음(not found)
                sendResponse(client_socket, "404 Not Found", "text/html", "<h1>404 Not Found</h1>");
            }
        } else {
            // todo 유효하지 않은 요청에 대응하는 코드로 일부 수정하기
            sendResponse(client_socket, "404 Not Found", "text/html", "<h1>404 Not Found</h1>");
        }

        // 소켓 닫음
        close(client_socket);
    }

    // 서버 소켓 닫음
    // 적절한 시점에 다음과 같이 리소스를 회수할 것
    close(server_fd);
    return 0;
}