#include <windows.h>
#include <iostream>
#include <vector>

#define MAX_MESSAGE_SIZE 256
#define MAX_NAME_SIZE 32

struct Message {
	char sender[MAX_NAME_SIZE];
	char text[MAX_MESSAGE_SIZE];
};

#define PIPE_NAME "\\\\.\\pipe\\ChatPipe"

std::vector<HANDLE> clientPipes;
std::vector<HANDLE> clientThreads;
bool startChat = false;

void BroadcastMessage(const Message& msg) {
	for (HANDLE pipe : clientPipes) {
		DWORD bytesWritten;
		WriteFile(pipe, &msg, sizeof(msg), &bytesWritten, NULL);
	}
}

void threadFunc(LPVOID lpParam) {
	HANDLE clientPipe = (HANDLE)lpParam;
	Message msg;
	while (true) {
		DWORD bytesRead;
		DWORD bytesAvailable = 0;
		//std::cout << "super.\n" << clientPipe;
		if (PeekNamedPipe(clientPipe, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
			BOOL result = ReadFile(clientPipe, &msg, sizeof(msg), &bytesRead, NULL);
			//std::cout << "sup.\n";

			if (result && bytesRead > 0) {
				std::cout << "[" << msg.sender << "] " << msg.text << "\n";
				BroadcastMessage(msg);
			}
			else {
				std::cout << "Client disconnected.\n";
				break;
			}
		}
		Sleep(100);
	}

	CloseHandle(clientPipe);
}

int main() {
	
	// allows one more client after flag setting
	auto commandThread = CreateThread(NULL, 0, [](LPVOID lpParam) -> DWORD {
		std::string input;
		std::cin >> input;
		if (input == "init") startChat = true;
		return 0;
		}, NULL, 0, NULL);

	while (!startChat) {
		HANDLE hPipe = CreateNamedPipeA(
			PIPE_NAME,
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			sizeof(Message),
			sizeof(Message),
			0,
			NULL
		);

		if (hPipe == INVALID_HANDLE_VALUE) {
			std::cerr << "Failed to create named pipe.\n";
			return 1;
		}

		std::cout << "Waiting for client connection...\n";

		if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
			std::cout << "Client connected.\n";
			clientPipes.push_back(hPipe);
		}
		else {
			CloseHandle(hPipe);
		}

	}

	CloseHandle(commandThread);

	for (auto pipe : clientPipes) {
		auto thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFunc, pipe, 0, NULL);
		clientThreads.push_back(thread);
	}

	HANDLE* tmp = new HANDLE[clientThreads.size()];

	for (int i = 0; i < clientThreads.size(); i++) {
		tmp[i] = clientThreads.at(i);
	}

	for (int i = 0; i < clientThreads.size(); i++) {
		WaitForSingleObject(tmp[i], INFINITE);
	}
	return 0;
}