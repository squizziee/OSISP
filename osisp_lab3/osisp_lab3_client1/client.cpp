#include <windows.h>
#include <iostream>
#include <string>

#define PIPE_NAME "\\\\.\\pipe\\ChatPipe"

#define MAX_MESSAGE_SIZE 256
#define MAX_NAME_SIZE 32

struct Message {
    char sender[MAX_NAME_SIZE];
    char text[MAX_MESSAGE_SIZE];
};

void threadFunc(LPVOID lpParam) {
    HANDLE pipe = (HANDLE)lpParam;
    Message receivedMsg;

    while (true) {

        DWORD bytesRead;
        DWORD bytesAvailable;
        if (PeekNamedPipe(pipe, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
            BOOL result = ReadFile(pipe, &receivedMsg, sizeof(receivedMsg), &bytesRead, NULL);

            if (result && bytesRead > 0) {
                std::cout << "\r[" << receivedMsg.sender << "] " << receivedMsg.text << "\n> ";
            }
            else {
                std::cout << "Disconnected from server.\n";
                break;
            }
        }
    }

    CloseHandle(pipe);
}

int main() {
    HANDLE hPipe;

    // Подключение к серверу
    while (true) {
        hPipe = CreateFileA(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (hPipe != INVALID_HANDLE_VALUE) break;

        std::cout << "Waiting for server...\n";
        Sleep(1000);
    }

    std::cout << "Connected to the server.\n";

    Message msg;
    std::string input;

    std::cout << "Enter your name: ";
    std::cin.getline(msg.sender, MAX_NAME_SIZE);

    auto thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunc, hPipe, 0, NULL);

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "exit") break;
        strncpy_s(msg.text, input.c_str(), MAX_MESSAGE_SIZE);

        DWORD bytesWritten;
        WriteFile(hPipe, &msg, sizeof(msg), &bytesWritten, NULL);
    }

    CloseHandle(hPipe);
    return 0;
}
