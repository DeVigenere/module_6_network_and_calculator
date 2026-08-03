#include "Calculator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::string buildMessage(const std::string& source, const std::string& payload) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%dT%H:%M:%SZ");
    std::stringstream message;
    message << "{\"source_service\":\"" << source << "\","
        << "\"timestamp_utc\":\"" << ss.str() << "\","
        << "\"payload\":\"" << payload << "\"}";
    return message.str();
}

bool initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "error initialization winsock" << std::endl;
        return false;
    }
    return true;
}

std::string catchfullrecv(SOCKET sock) {
    std::string response;
    char buffer[BUFFER_SIZE] = { 0 };
    int timeout = 2000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    while (true) {
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes > 0) {
            response += std::string(buffer, bytes);
        }
        else if (bytes == 0) {
            break;
        }
        else {
            break;
        }
    }
    if (!response.empty()) {
        std::cout << "server response: " << response << std::endl;
    }
    return response;
}

bool sendMessage(const std::string& message) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "error create socket" << std::endl;
        return false;
    }
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "error connect to serever(unavailable)" << std::endl;
        closesocket(sock);
        return false;
    }
    auto bytes_sent = send(sock, message.c_str(), message.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        std::cerr << "error sending" << std::endl;
        closesocket(sock);
        return false;
    }
    catchfullrecv(sock);
    closesocket(sock);
    return true;
}

int main() {
    if (!initWinsock()) {
        return 1;
    }
    Calculator calc;
    struct Operation {
        int a, b;
        char op;
    };
    Operation ops[] = {
        {10, 5, '+'},
        {20, 4, '-'},
        {7, 3, '*'},
        {15, 3, '/'},
        {8, 0, '/'}
    };
    int sent_count = 0;
    int total_ops = sizeof(ops) / sizeof(ops[0]);
    for (const auto& op : ops) {
        std::string result;
        try {
            switch (op.op) {
            case '+':
                result = std::to_string(calc.Add(op.a, op.b));
                break;
            case '-':
                result = std::to_string(calc.Subtract(op.a, op.b));
                break;
            case '*':
                result = std::to_string(calc.Multiply(op.a, op.b));
                break;
            case '/':
                result = std::to_string(calc.Divide(op.a, op.b));
                break;
            }
            std::string payload = std::to_string(op.a) + " " + op.op + " " +
                std::to_string(op.b) + " = " + result;
            std::string message = buildMessage("calculator", payload);
            std::cout << "\nsending: " << message << std::endl;
            if (sendMessage(message)) {
                sent_count++;
            }
            else {
                std::cerr << "cannot send message" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "error calculation: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); //for demo
    }
    std::cout << "\nsending " << sent_count << " from " << total_ops << " message" << std::endl;
    WSACleanup();
    system("pause");
    return 0;
}