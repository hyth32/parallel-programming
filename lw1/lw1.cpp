#include <windows.h>
#include <iostream>

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    int threadNumber = *(int *) lpParam;
    std::cout << "Thread №" << threadNumber << " is doing its work" << std::endl;
    ExitThread(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Args: *.exe count" << std::endl;
        return 1;
    }

    int threadCount = std::stoi(argv[1]);

    if (threadCount <= 0) {
        std::cout << "Invalid threads count: " << threadCount << std::endl;
        return 1;
    }

    auto *threadHandles = new HANDLE[threadCount];

    for (int i = 0; i < threadCount; ++i) {
        int *threadNumber = new int(i + 1);
        threadHandles[i] = CreateThread(nullptr, 0, &ThreadFunction, threadNumber, CREATE_SUSPENDED, nullptr);
    }

    for (int i = 0; i < threadCount; ++i) {
        ResumeThread(threadHandles[i]);
    }

    WaitForMultipleObjects(threadCount, threadHandles, TRUE, INFINITE);

    delete[] threadHandles;
    return 0;
}