//#include <windows.h>
//#include <iostream>
//#include <vector>
//#include <thread>
//#include <algorithm>
//
//LARGE_INTEGER frequency, start, endTime;
//
//void startTimer() {
//    QueryPerformanceFrequency(&frequency);
//    QueryPerformanceCounter(&start);
//}
//
//void endTimer() {
//    QueryPerformanceCounter(&endTime);
//    double elapsed = static_cast<double>(endTime.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
//    std::cout << "Elapsed time: " << elapsed << " ms" << std::endl;
//}
//
//// heavy calculation
//void processData(int* data, size_t size) {
//    std::sort(data, data + size);
//}
//
//int main() {
//    const char* filename = "data.bin";
//
//    HANDLE hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//    if (hFile == INVALID_HANDLE_VALUE) {
//        std::cerr << "Could not open file." << std::endl;
//        return 1;
//    }
//
//    size_t fileSize = GetFileSize(hFile, NULL);
//    if (fileSize == INVALID_FILE_SIZE) {
//        std::cerr << "Could not get file size." << std::endl;
//        CloseHandle(hFile);
//        return 1;
//    }
//
//    const size_t dataSize = fileSize;
//
//    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
//    if (!hMapping) {
//        std::cerr << "Could not create file mapping." << std::endl;
//        CloseHandle(hFile);
//        return 1;
//    }
//
//    int* mappedData = (int*)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, dataSize);
//    if (!mappedData) {
//        std::cerr << "Could not map view of file." << std::endl;
//        CloseHandle(hMapping);
//        CloseHandle(hFile);
//        return 1;
//    }
//
//    const int threadCount = 1;
//    std::vector<std::thread> threads;
//    size_t chunkSize = dataSize / threadCount / sizeof(int);
//
//    for (int i = 0; i < threadCount; ++i) {
//        threads.push_back(std::thread(processData, mappedData + i * chunkSize, chunkSize));
//    }
//
//    std::cout << "File: " << filename << std::endl;
//    std::cout << "Threads launched: " << threadCount << std::endl;
//    std::cout << "File size: " << fileSize / (1024 * 1024) << " MB" << std::endl;
//
//    startTimer();
//
//    for (auto& t : threads) {
//        t.join();
//    }
//
//    endTimer();
//
//    UnmapViewOfFile(mappedData);
//    CloseHandle(hMapping);
//    CloseHandle(hFile);
//
//    return 0;
//}