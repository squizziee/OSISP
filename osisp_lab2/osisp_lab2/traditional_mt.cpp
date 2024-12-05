#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

LARGE_INTEGER frequency, start, endTime;

void startTimer() {
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
}

void endTimer() {
    QueryPerformanceCounter(&endTime);
    double elapsed = static_cast<double>(endTime.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
    std::cout << "Elapsed time: " << elapsed << " ms" << std::endl;
}

// heavy calculation
void processData(int* data, size_t size) {
    std::sort(data, data + size);
}

int main() {
    const char* filename = "data.bin";

    HANDLE hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not open file." << std::endl;
        return 1;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "Could not get file size." << std::endl;
        CloseHandle(hFile);
        return 1;
    }

    const size_t dataSize = fileSize;

    int* data = new int[dataSize / sizeof(int)];

    DWORD bytesRead;
    ReadFile(hFile, data, dataSize, &bytesRead, NULL);

    const int threadCount = 12;
    size_t chunkSize = (dataSize / sizeof(int)) / threadCount;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i) {
        int* chunkStart = data + i * chunkSize;
        size_t currentChunkSize = (i == threadCount - 1) ? (dataSize / sizeof(int)) - i * chunkSize : chunkSize;
        threads.emplace_back(processData, chunkStart, currentChunkSize);
    }

    std::cout << "File: " << filename << std::endl;
    std::cout << "Threads launched: " << threadCount << std::endl;
    std::cout << "File size: " << fileSize / (1024 * 1024) << " MB" << std::endl;

    startTimer();

    for (auto& t : threads) {
        t.join();
    }

    endTimer();

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    DWORD bytesWritten;
    WriteFile(hFile, data, dataSize, &bytesWritten, NULL);

    delete[] data;

    CloseHandle(hFile);

    return 0;
}