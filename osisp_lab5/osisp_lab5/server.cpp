#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

struct ConnectedClient {
    SOCKET socket;
    std::string name;

    bool operator==(const ConnectedClient& other) const {
        return socket == other.socket;
    }
};

struct Message {
    char deliverTo[50];
    char from[50];
    char text_content[500];
    bool isSetup = false;
};

std::vector<ConnectedClient> clients; // Список клиентских сокетов
std::mutex clientsMutex;

void broadcastMessage(const ConnectedClient& client, Message& message) {
    auto excludeSocket = client.socket;
    std::lock_guard<std::mutex> lock(clientsMutex);

    char* buffer = new char[sizeof(Message)];
    memcpy(buffer, &message, sizeof(Message));

    for (auto receiver : clients) {
        if (receiver.socket != client.socket) {
            send(receiver.socket, buffer, sizeof(Message), 0);
        }
    }
}

void sendMessage(const ConnectedClient& client, Message& message) {
    auto excludeSocket = client.socket;
    std::lock_guard<std::mutex> lock(clientsMutex);

    char* buffer = new char[sizeof(Message)];
    memcpy(buffer, &message, sizeof(Message));

    for (auto pReceiver : clients) {
        if (strcmp(pReceiver.name.c_str(), message.deliverTo) == 0) {
            send(pReceiver.socket, buffer, sizeof(Message), 0);
        }
    }
}

void handleClient(ConnectedClient client) {
    char buffer[BUFFER_SIZE];
    Message message;
    while (true) {
        memset(buffer, 0, sizeof(Message));
        int bytesReceived = recv(client.socket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            // Клиент отключился
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
            closesocket(client.socket);
            break;
        }

        // Обработка полученного сообщения

        memcpy(&message, (void*)buffer, sizeof(Message));

        if (message.isSetup) {
            std::cout << "Received setup for: " << message.from << std::endl;
            for (auto& dest : clients) {
                if (dest.socket == client.socket) {
                    dest.name = message.from;
                }
            }
        }
        else if (strcmp(message.deliverTo, "") == 0) {
            broadcastMessage(client, message);
        }
        else {
            std::cout << "Received: " << message.text_content << ", To: " << message.deliverTo << ", From: " << message.from << std::endl;
            sendMessage(client, message);
        }
    }
}

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "Server is running on port " << SERVER_PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        auto newClient = ConnectedClient();
        newClient.name = "default";
        newClient.socket = clientSocket;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(newClient);
        }

        std::thread clientThread(handleClient, newClient);
        clientThread.detach();
    }

    // Завершение работы
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
