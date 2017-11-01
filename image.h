#ifndef RGBYUV_H
#define RGBYUV_H

#endif // RGBYUV_H

#include <cstdint>
#include <cstdio>

uint32_t read_dword(FILE* file);
uint16_t read_word(FILE* file);
uint8_t *rgb2yuv(uint32_t Width, uint32_t Height, uint8_t *bgrArr);
void rgb2yuvPart(uint32_t Width, uint8_t Height, uint32_t pHeight, uint32_t row, uint8_t *bgrArr, uint8_t *yuvArr);
bool saveYUV(char *path, uint8_t *yuvArr, uint32_t size);
//BMP header structure
struct bmpHeader{
    uint16_t bfType = 0;
    uint32_t bfSize = 0;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 0;
    uint32_t bfWidth = 0;
    uint32_t bfHeight = 0;

    void ReadHeader(FILE* file);
    void getWidth(FILE* file);
    void getHeight(FILE* file);
};

