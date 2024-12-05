#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <ws2tcpip.h> // Для inet_pton

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8080

struct Message {
    char deliverTo[50];
    char from[50];
    char text_content[500];
    bool isSetup = false;
};

void receiveMessages(SOCKET clientSocket) {
    char buffer[sizeof(Message)];
    Message message;
    while (true) {
        memset(buffer, 0, sizeof(Message));
        int bytesReceived = recv(clientSocket, buffer, sizeof(Message), 0);
        if (bytesReceived <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            break;
        }

        memcpy(&message, (void*)buffer, sizeof(Message));

        std::cout << "\r                                                                            ";
        printf_s("\r[%s] %s\n", message.from, message.text_content);

        //std::cout << message.text_content << ", From: " << message.from << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    const char* serverIp = "127.0.0.1";
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Connected to server!" << std::endl;

    std::thread receiveThread(receiveMessages, clientSocket);
    receiveThread.detach();

    Message message;

    std::cout << "Choose name: ";
    std::cin.getline(message.from, 50);

    message.isSetup = true;
    while (true) {
        if (!message.isSetup) {
            std::cout << "Deliver to (skip to broadcast): ";
            std::cin.getline(message.deliverTo, 50);

            std::cout << "Message: ";
            std::cin.getline(message.text_content, 500);

            if (strcmp(message.text_content, "/exit") == 0) {
                break;
            }
        }

        char* buffer = new char[sizeof(Message)];
        memcpy(buffer, &message, sizeof(Message));

        send(clientSocket, buffer, sizeof(Message), 0);

        message.isSetup = false;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
