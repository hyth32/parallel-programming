#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>

std::chrono::steady_clock::time_point startTime;

class Thread {
public:
    size_t threadNum;
};

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    auto *data = (Thread *) lpParam;

    double load = 0;
    for (int i = 0; i < 20; i++) {
        for (int k = 0; k < 1000; k++) {
            for (int j = 0; j < 1000; j++) {
                load = i * j;
                load = pow(i, j);
                load = sqrt(i);
            }
        }
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::string result = std::to_string(data->threadNum) + "|" + std::to_string(duration);

        std::ofstream file;
        file.open("output.txt", std::ios_base::app);
        if (file.is_open()) {
            file << result << std::endl;
            file.close();
        } else {
            std::cout << "Error opening output file" << std::endl;
        }
    }

    ExitThread(0);
}

int main() {
    auto *handles = new HANDLE[2];
    startTime = std::chrono::steady_clock::now();

    for (int i = 0; i < 2; i++) {
        auto *data = new Thread();
        data->threadNum = i + 1;

        handles[i] = CreateThread(nullptr, 0, &ThreadProc, data, CREATE_SUSPENDED, nullptr);
    }

    SetThreadPriority(handles[1], THREAD_PRIORITY_HIGHEST);

    for (int i = 0; i < 2; i++) {
        if (handles[i] != nullptr) {
            ResumeThread(handles[i]);
        }
    }

    WaitForMultipleObjects(2, handles, TRUE, INFINITE);

    return 0;
}