#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <thread>

uint32_t   read_dword(FILE *file);
uint16_t    read_word(FILE *file);

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

    int y = 0;
    int u = imgHead.bfWidth * imgHead.bfHeight;
    int v = u + imgHead.bfWidth * imgHead.bfHeight / 4;
    bool skiprowFlag = false;
    for (int i = imgHead.bfWidth * imgHead.bfHeight - imgHead.bfWidth; i >= 0; i -= imgHead.bfWidth){
        for (uint32_t j = i; j < (i + imgHead.bfWidth); j++){
            yuvImage[y++] = ((66 * bgrImage[3 * j + 2] + 129 * bgrImage[3 * j + 1] + 25 * bgrImage[3 * j] + 128) >> 8) + 16;
            if (j % 2 == 0){
                if (skiprowFlag)
                    continue;
                yuvImage[u++] = ((-38 * bgrImage[3 * j + 2] - 74 * bgrImage[3 * j + 1] + 112 * bgrImage[3 * j] + 128) >> 8) + 128;
                yuvImage[v++] = ((112 * bgrImage[3 * j + 2] - 94 * bgrImage[3 * j + 1] - 18 * bgrImage[3 * j] + 128) >> 8) + 128;
            }
        }
        if (skiprowFlag)
            skiprowFlag = false;
        else skiprowFlag = true;
    }

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
