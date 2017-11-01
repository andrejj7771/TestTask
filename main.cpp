#include "image.h"

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <ctime>
#include <cmath>


/*Main Function*/
int main (){
    std::string imgInput = "/home/andrey/TestTask/files/image.bmp";
    std::string videoInput = "/home/andrey/TestTask/files/input.yuv";
    std::string videoOutput = "/home/andrey/TestTask/files/output.yuv";

    FILE *file;
    if ((file = fopen(imgInput.c_str(), "r")) == NULL){
        perror("open");
        return -1;
    }

    bmpHeader imgHead;
    imgHead.ReadHeader(file);

    uint8_t *bgrImage = new uint8_t[imgHead.bfWidth * imgHead.bfHeight * 3];
    uint8_t *yuvImage = new uint8_t[imgHead.bfWidth * imgHead.bfHeight + (imgHead.bfWidth * imgHead.bfHeight / 2)];

    fseek(file, imgHead.bfOffBits, SEEK_SET);
    fread(bgrImage, sizeof(uint8_t), imgHead.bfWidth * imgHead.bfHeight * 3, file);
    fclose(file);

    double start = clock();
    rgb2yuv(10, 8, 2, 7, bgrImage, yuvImage);
    //yuvImage = rgb2yuv(imgHead.bfWidth, imgHead.bfHeight, bgrImage);

    std::cout << round(((clock() - start) / CLOCKS_PER_SEC) * 1000000) / 1000000 << std::endl;

    if ((file = fopen(videoInput.c_str(), "r")) == NULL){
        perror("open");
        return -1;
    }
    FILE *outFile = fopen(videoOutput.c_str(), "w");
    int vWidth = 352;
    int vHeight = 288;
    int frameSize = vWidth * vHeight + vWidth * vHeight / 2;

    uint8_t *frame = new uint8_t[frameSize];
    while (!feof(file)) {
        if (fread(frame, sizeof(uint8_t), frameSize, file) == (size_t)frameSize){
            for (uint32_t i = 0, f_y = 0; i < imgHead.bfHeight * imgHead.bfWidth; i++){
                frame[f_y++] = yuvImage[i];
                if (i % imgHead.bfWidth == 0 && i != 0)
                    f_y += vWidth - imgHead.bfWidth;
            }
            int f_u = vWidth * vHeight;
            for (uint32_t i = imgHead.bfHeight * imgHead.bfWidth + 1; i < (imgHead.bfHeight * imgHead.bfWidth) + (imgHead.bfHeight * imgHead.bfWidth / 4); i++, f_u++){
                frame[f_u] = yuvImage[i];
                frame[f_u + (vWidth * vHeight / 4)] = yuvImage[i + (imgHead.bfHeight * imgHead.bfWidth / 4)];
                if (i % (imgHead.bfWidth / 2) == 0){
                    f_u += (vWidth - imgHead.bfWidth) / 2;
                }
            }
            fwrite(frame, sizeof(uint8_t), frameSize, outFile);
        }
    }
    fclose(file);
    fclose(outFile);

    delete[]bgrImage;
    delete[]yuvImage;
    delete[]frame;
    return 0;
}
