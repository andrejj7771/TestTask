#ifndef RGBYUV_H
#define RGBYUV_H

#endif // RGBYUV_H

#include <cstdint>
#include <cstdio>

uint32_t read_dword(FILE *file){
    uint8_t byte_1 = 0;
    uint8_t byte_2 = 0;
    uint8_t byte_3 = 0;
    uint8_t byte_4 = 0;

    fread(&byte_1, sizeof(uint8_t), 1, file);
    fread(&byte_2, sizeof(uint8_t), 1, file);
    fread(&byte_3, sizeof(uint8_t), 1, file);
    fread(&byte_4, sizeof(uint8_t), 1, file);

   return ((((((byte_4 << 8) | byte_3) << 8) | byte_2) << 8) | byte_1);
}
uint16_t read_word(FILE *file){
    uint8_t byte_1 = 0;
    uint8_t byte_2 = 0;

    fread(&byte_1, sizeof(uint8_t), 1, file);
    fread(&byte_2, sizeof(uint8_t), 1, file);

    return ((byte_1 << 8) | byte_2);
}

//BMP header structure
struct bmpHeader{
    uint16_t bfType = 0;
    uint32_t bfSize = 0;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 0;
    uint32_t bfWidth = 0;
    uint32_t bfHeight = 0;

    void ReadHeader(FILE *file){
        bfType = read_word(file);
        bfSize = read_dword(file);
        bfReserved1 = read_word(file);
        bfReserved2 = read_word(file);
        bfOffBits = read_dword(file);
        getWidth(file);
        getHeight(file);
    }
    void getWidth(FILE *file){
        fseek(file, 18, SEEK_SET);
        bfWidth = read_dword(file);
    }
    void getHeight(FILE *file){
        fseek(file, 22, SEEK_SET);
        bfHeight = read_dword(file);
    }
};

uint8_t *rgb2yuv(uint32_t Width, uint32_t Height, uint8_t *bgrArr){
    int y = 0;
    int u = Width * Height;
    int v = u + Width * Height / 4;
    uint8_t *yuvArr = new uint8_t[Width * Height + (Width * Height / 2)];
    bool skiprowFlag = false;
    for (int i = Width * Height - Width; i >= 0; i -= Width){
        for (uint32_t j = i; j < (i + Width); j++){
            yuvArr[y++] = ((66 * bgrArr[3 * j + 2] + 129 * bgrArr[3 * j + 1] + 25 * bgrArr[3 * j] + 128) >> 8) + 16;
            if (j % 2 == 0){
                if (skiprowFlag)
                    continue;
                yuvArr[u++] = ((-38 * bgrArr[3 * j + 2] - 74 * bgrArr[3 * j + 1] + 112 * bgrArr[3 * j] + 128) >> 8) + 128;
                yuvArr[v++] = ((112 * bgrArr[3 * j + 2] - 94 * bgrArr[3 * j + 1] - 18 * bgrArr[3 * j] + 128) >> 8) + 128;
            }
        }
        if (skiprowFlag)
            skiprowFlag = false;
        else skiprowFlag = true;
    }
    return yuvArr;
}

