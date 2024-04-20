#include "bitmap_image.hpp"
#include <windows.h>
#include <iostream>
#include <vector>
#include <chrono>

int BLUR_WIDTH = 5;
int BLUR_SIZE = 10;

class Data {
public:
    bitmap_image *initImage;
    bitmap_image *blurredImage;
    int startRow;
    int endRow;
};

void boxBlur(bitmap_image *image, bitmap_image *blurredImage, int startRow, int endRow, int blurSize) {
    int width = image->width();
    int height = image->height();

    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < width; ++j) {
            int redSum = 0;
            int greenSum = 0;
            int blueSum = 0;
            int count = 0;

            for (int di = -blurSize; di <= blurSize; ++di) {
                for (int dj = -blurSize; dj <= blurSize; ++dj) {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
                        rgb_t pixel;
                        image->get_pixel(nj, ni, pixel);

                        redSum += pixel.red;
                        greenSum += pixel.green;
                        blueSum += pixel.blue;

                        ++count;
                    }
                }
            }

            rgb_t blurredPixel;
            blurredPixel.red = redSum / count;
            blurredPixel.green = greenSum / count;
            blurredPixel.blue = blueSum / count;

            blurredImage->set_pixel(j, i, blurredPixel);
        }
    }
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
    auto *tData = (struct Data *) lpParam;
    boxBlur(tData->initImage, tData->blurredImage, tData->startRow, tData->endRow, BLUR_SIZE);
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

    auto startTime = std::chrono::high_resolution_clock::now();

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

            handles[j] = CreateThread(nullptr, 0, &ThreadProc, (LPVOID) tData, CREATE_SUSPENDED, nullptr);

            if (handles[j] == nullptr) continue;

            DWORD_PTR affinityMask = (DWORD_PTR) 1 << (j % coresCount);
            SetThreadAffinityMask(handles[j], affinityMask);
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

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Time: " << duration << " ms" << std::endl;

    return 0;
}