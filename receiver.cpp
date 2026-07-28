#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8080;
const int BUFFER_SIZE = 4096;

bool initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "error initialization winsock" << std::endl;
        return false;
    }
    return true;
}

bool isValidMessage(const std::string& msg) {
    return msg.find("\"source_service\"") != std::string::npos &&
        msg.find("\"timestamp_utc\"") != std::string::npos &&
        msg.find("\"payload\"") != std::string::npos;
}

void printMessage(const std::string& message) {
    std::cout << "received " << std::endl;
    std::cout << message << std::endl;
    if (isValidMessage(message)) {
        std::cout << "format message correct" << std::endl;
        size_t sourcePos = message.find("\"source_service\":\"");
        if (sourcePos != std::string::npos) {
            size_t start = sourcePos + 18; //hardcode, i know)
            size_t end = message.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "source: " << message.substr(start, end - start) << std::endl;
            }
        }
        size_t payloadPos = message.find("\"payload\":\"");
        if (payloadPos != std::string::npos) {
            size_t start = payloadPos + 11;// +1 :)
            size_t end = message.find("\"", start);
            if (end != std::string::npos) {
                std::cout << "data: " << message.substr(start, end - start) << std::endl;
            }
        }
    }
    else {
        std::cout << "incorrect message" << std::endl;
    }
}

DWORD WINAPI handleClient(LPVOID param) {
    SOCKET client_socket = (SOCKET)param;
    char buffer[BUFFER_SIZE] = { 0 };
    std::cout << "new connection" << std::endl;
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read == SOCKET_ERROR) {
            std::cerr << "error read" << std::endl;
            break;
        }
        else if (bytes_read == 0) {
            std::cout << "client disconnected" << std::endl;
            break;
        }
        std::string received(buffer, bytes_read);
        printMessage(received);
        const char* response = "message received"; //send() waiting char* not string
        send(client_socket, response, strlen(response), 0);
    }
    closesocket(client_socket);
    return 0;
}

int main() {
    if (!initWinsock()) {
        return 1;
    }
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "error creating socket" << std::endl;
        WSACleanup();
        return 1;
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cerr << "error setsockopt" << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "maybe " << PORT << " already used" << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    if (listen(server_fd, 5) == SOCKET_ERROR) {
        std::cerr << "error listen" << std::endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    std::cout << "service was started" << std::endl;
    std::cout << "port: " << PORT << std::endl;
    std::vector<HANDLE> threads;
    while (true) {
        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);

        SOCKET client_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen);
        if (client_socket == INVALID_SOCKET) {
            if (true) {
                std::cerr << "error accept" << std::endl;
            }
            continue;
        }
        HANDLE thread = CreateThread(NULL, 0, handleClient, (LPVOID)client_socket, 0, NULL);
        if (thread != NULL) {
            threads.push_back(thread);
        }
        else {
            std::cerr << "error creating thread" << std::endl;
            closesocket(client_socket);
        }
    }
    std::cout << "waiting when close threads" << std::endl;
    for (HANDLE h : threads) {
        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
    }
    closesocket(server_fd);
    WSACleanup();
    std::cout << "service stop" << std::endl;
    system("pause");
    return 0;
}