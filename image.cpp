#include "image.h"

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
void rgb2yuvPart(uint32_t Width, uint32_t Height, uint32_t pHeight, uint32_t row, uint8_t *bgrArr, uint8_t *yuvArr){
    int y = int(Width * Height - Width * (row + 1));
    int u = (Width * Height + Width * Height / 4) - (Width * (row + 1) / 4);
    int v = (Width * Height + Width * Height / 2) - (Width * (row + 1) / 4);
    bool skiprowFlag = false;
    for (int i = Width * row; i >= int(Width * (row + 1) - pHeight * Width); i -= Width){
        for (int j = i; j < (i + Width); j++){
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
}
bool saveYUV(char *path, uint8_t *yuvArr, uint32_t size){
    FILE *f = fopen(path, "w");
    fwrite(yuvArr, sizeof(uint8_t), size, f);
    fclose(f);
}

//BMP header functions
void bmpHeader::ReadHeader(FILE *file){
    bfType = read_word(file);
    bfSize = read_dword(file);
    bfReserved1 = read_word(file);
    bfReserved2 = read_word(file);
    bfOffBits = read_dword(file);
    getWidth(file);
    getHeight(file);
}
void bmpHeader::getWidth(FILE *file){
    fseek(file, 18, SEEK_SET);
    bfWidth = read_dword(file);
}
void bmpHeader::getHeight(FILE *file){
    fseek(file, 22, SEEK_SET);
    bfHeight = read_dword(file);
}
