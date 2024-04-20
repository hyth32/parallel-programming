#include "bitmap_image.hpp"
#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

int BLUR_WIDTH = 5;
int BLUR_SIZE = 1000000000;

std::chrono::high_resolution_clock::time_point programStartTime;
std::mutex mtx;

class Data {
public:
    bitmap_image *initImage;
    bitmap_image *blurredImage;
    int startRow;
    int endRow;
    int threadIndex;
};

std::vector<std::pair<int, long long>> durations;

void boxBlur(bitmap_image *image, bitmap_image *blurredImage, int startRow, int endRow, int blurSize, int threadIndex) {
    int width = image->width();
    int height = image->height();

    std::vector<std::vector<int>> integralImageR(height, std::vector<int>(width));
    std::vector<std::vector<int>> integralImageG(height, std::vector<int>(width));
    std::vector<std::vector<int>> integralImageB(height, std::vector<int>(width));

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            rgb_t pixel;
            image->get_pixel(j, i, pixel);

            integralImageR[i][j] = pixel.red + (i > 0 ? integralImageR[i - 1][j] : 0) + (j > 0 ? integralImageR[i][j - 1] : 0) - (i > 0 && j > 0 ? integralImageR[i - 1][j - 1] : 0);
            integralImageG[i][j] = pixel.green + (i > 0 ? integralImageG[i - 1][j] : 0) + (j > 0 ? integralImageG[i][j - 1] : 0) - (i > 0 && j > 0 ? integralImageG[i - 1][j - 1] : 0);
            integralImageB[i][j] = pixel.blue + (i > 0 ? integralImageB[i - 1][j] : 0) + (j > 0 ? integralImageB[i][j - 1] : 0) - (i > 0 && j > 0 ? integralImageB[i - 1][j - 1] : 0);
        }
    }

    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < width; ++j) {
            int x1 = std::max(0, j - blurSize);
            int y1 = std::max(0, i - blurSize);
            int x2 = std::min(width - 1, j + blurSize);
            int y2 = std::min(height - 1, i + blurSize);

            int count = (x2 - x1 + 1) * (y2 - y1 + 1);

            int redSum = integralImageR[y2][x2] - (x1 > 0 ? integralImageR[y2][x1 - 1] : 0) - (y1 > 0 ? integralImageR[y1 - 1][x2] : 0) + (x1 > 0 && y1 > 0 ? integralImageR[y1 - 1][x1 - 1] : 0);
            int greenSum = integralImageG[y2][x2] - (x1 > 0 ? integralImageG[y2][x1 - 1] : 0) - (y1 > 0 ? integralImageG[y1 - 1][x2] : 0) + (x1 > 0 && y1 > 0 ? integralImageG[y1 - 1][x1 - 1] : 0);
            int blueSum = integralImageB[y2][x2] - (x1 > 0 ? integralImageB[y2][x1 - 1] : 0) - (y1 > 0 ? integralImageB[y1 - 1][x2] : 0) + (x1 > 0 && y1 > 0 ? integralImageB[y1 - 1][x1 - 1] : 0);

            rgb_t blurredPixel;
            blurredPixel.red = redSum / count;
            blurredPixel.green = greenSum / count;
            blurredPixel.blue = blueSum / count;

            blurredImage->set_pixel(j, i, blurredPixel);

            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - programStartTime).count();
            mtx.lock();
            durations.emplace_back(threadIndex, duration);
            mtx.unlock();
        }
    }
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    Data *tData = (struct Data *) lpParam;
    boxBlur(tData->initImage, tData->blurredImage, tData->startRow, tData->endRow, BLUR_SIZE, tData->threadIndex);
    ExitThread(0);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cout << "Args: *.exe input.bmp output.bmp cores threads" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    int coresCount = std::stoi(argv[3]);
    int threadsCount = std::stoi(argv[4]);

    programStartTime = std::chrono::high_resolution_clock::now();

    bitmap_image image(inputFile);
    bitmap_image newImage(inputFile);

    if (!image) {
        std::cerr << "Could not open image" << std::endl;
        return 1;
    }

    auto *handles = new HANDLE[threadsCount];

    int threadRows = image.height() / threadsCount;

    newImage.setwidth_height(image.width() / BLUR_WIDTH * BLUR_WIDTH, image.height());

    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < threadsCount; j++) {
            int start_row = j * threadRows;
            int end_row = 0;
            if (j == threadsCount - 1) {
                end_row = image.height();
            } else {
                end_row = (j + 1) * threadRows;
            }

            Data *tData = new Data();

            tData->initImage = &image;
            tData->blurredImage = &newImage;
            tData->startRow = start_row;
            tData->endRow = end_row;
            tData->threadIndex = j;

            handles[j] = CreateThread(nullptr, 0, &ThreadProc, (LPVOID) tData, CREATE_SUSPENDED, NULL);

            if (handles[j] == nullptr) continue;

//            int priority;
//            if (j == 0) {
//                priority = THREAD_PRIORITY_ABOVE_NORMAL;
//            } else {
//                priority = THREAD_PRIORITY_LOWEST;
//            }
//            SetThreadPriority(handles[j], priority);

//            DWORD_PTR affinity_mask = (DWORD_PTR)1 << (i % coresCount);
//            SetThreadAffinityMask(handles[i], affinity_mask);
        }

        for (int k = 0; k < threadsCount; k++) {
            if (handles[k] == nullptr) {
                std::cout << "Couldn't create thread" << k << std::endl;
                return 1;
            }
            ResumeThread(handles[k]);
        }

        WaitForMultipleObjects(threadsCount, handles, TRUE, INFINITE);
    }

    newImage.save_image(outputFile);

    std::ofstream outfile("durations.txt", std::ios_base::out);
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &pair : durations) {
        outfile << "Thread " << pair.first + 1 << " | " << pair.second << " ms" << std::endl;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - programStartTime).count();

    std::cout << "Time: " << duration << " ms" << std::endl;

    return 0;
}