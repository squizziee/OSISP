#include <windows.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <string>

#define BLOCK_SIZE 1024
#define NUM_BLOCKS 100
#define SHARED_MEMORY_NAME "Global\\SharedMemory"

struct Block {
    bool isOccupied;
    char data[BLOCK_SIZE]; 
};

struct SharedMemory {
    Block blocks[NUM_BLOCKS];
    HANDLE mutexes[NUM_BLOCKS];
};

HANDLE hMapFile;  
SharedMemory* pSharedMemory;  

std::mutex console_mutex;
void print_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << message << std::endl;
}

bool InitializeSharedMemory() {
    hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemory), SHARED_MEMORY_NAME);
    if (hMapFile == NULL) {
        std::cerr << "CreateFileMapping failed! Error: " << GetLastError() << std::endl;
        return false;
    }

    pSharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
    if (pSharedMemory == NULL) {
        std::cerr << "MapViewOfFile failed! Error: " << GetLastError() << std::endl;
        return false;
    }

    for (int i = 0; i < NUM_BLOCKS; ++i) {
        pSharedMemory->blocks[i].isOccupied = false;
        pSharedMemory->mutexes[i] = CreateMutexW(NULL, FALSE, NULL);
        if (pSharedMemory->mutexes[i] == NULL) {
            std::cerr << "CreateMutex failed for block " << i << ". Error: " << GetLastError() << std::endl;
            return false;
        }
    }

    return true;
}

int RequestBlock() {
    for (int i = 0; i < NUM_BLOCKS; ++i) {
        DWORD dwWaitResult = WaitForSingleObject(pSharedMemory->mutexes[i], INFINITE);
        if (dwWaitResult == WAIT_OBJECT_0) {
            if (!pSharedMemory->blocks[i].isOccupied) {
                pSharedMemory->blocks[i].isOccupied = true;
                ReleaseMutex(pSharedMemory->mutexes[i]);
                return i;
            }
            ReleaseMutex(pSharedMemory->mutexes[i]);
        }
    }
    return -1;
}

void ReleaseBlock(int blockIndex) {
    if (blockIndex >= 0 && blockIndex < NUM_BLOCKS) {
        WaitForSingleObject(pSharedMemory->mutexes[blockIndex], INFINITE);
        pSharedMemory->blocks[blockIndex].isOccupied = false;
        ReleaseMutex(pSharedMemory->mutexes[blockIndex]);
    }
}

void testFunction(int threadId) {
    int blockIndex = RequestBlock();
    if (blockIndex != -1) {
        print_message("Thread " + std::to_string(threadId) + " got block " + std::to_string(blockIndex));
        //std::cout << "Thread " << threadId << " got block " << blockIndex << std::endl;
        Sleep(1000);
        ReleaseBlock(blockIndex);
        print_message("Thread " + std::to_string(threadId) + " released block " + std::to_string(blockIndex));
        //std::cout << "Thread " << threadId << " released block " << blockIndex << std::endl;
    }
    else {
        print_message("Thread " + std::to_string(threadId) + " could not get a block");
        //std::cout << "Thread " << threadId << " could not get a block" << std::endl;
    }
}

void RunTests() {
    std::vector<HANDLE> threads;

    for (int i = 0; i < NUM_BLOCKS + 5; ++i) {
        threads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)testFunction, (LPVOID)i, 0, NULL));
    }

    for (auto handle : threads) {
        WaitForSingleObject(handle, INFINITE);
    }

    for (auto thread : threads) {
        CloseHandle(thread);
    }
}

int main() {
    if (!InitializeSharedMemory()) {
        std::cerr << "Failed to initialize shared memory!" << std::endl;
        return 1;
    }

    RunTests();

    for (int i = 0; i < NUM_BLOCKS + 5; ++i) {
        CloseHandle(pSharedMemory->mutexes[i]);
    }
    UnmapViewOfFile(pSharedMemory);
    CloseHandle(hMapFile);

    return 0;
}
