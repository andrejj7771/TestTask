#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <thread>

#define WORD u_int16_t
#define DWORD u_int32_t

using namespace std;

DWORD   read_dword(int fd);
WORD    read_word(int fd);

struct bmpHeader{
    WORD bfType = 0;
    DWORD bfSize = 0;
    WORD bfReserved1 = 0;
    WORD bfReserved2 = 0;
    DWORD bfOffBits = 0;

    void ReadHeader(int fd){
        bfType = read_word(fd);
        bfSize = read_dword(fd);
        bfReserved1 = read_word(fd);
        bfReserved2 = read_word(fd);
        bfOffBits = read_dword(fd);
    }
};
struct color{
    u_int8_t Red = 0;
    u_int8_t Green = 0;
    u_int8_t Blue = 0;
};

color read_color(int fd);

u_int8_t getHeight(int fd);
u_int8_t getWidth(int fd);

void writeY(u_int8_t **frame, color *color_mas, int iWidth, int iHeight, int vWidth, int ind);
void writeU(u_int8_t **frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight, int ind);
void writeV(u_int8_t **frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight, int ind);

int main(){
    string imgInput = "/home/andrey/nokia.bmp";
    string videoInput = "/home/andrey/input.yuv";
    string videoOutput = "/home/andrey/output.yuv";

    int vWidth = 352;
    int vHeight = 288;

    int fd = open(imgInput.c_str(), O_RDONLY); //open image
    if (fd == -1)
        perror("open file error");

    bmpHeader imgHead;
    imgHead.ReadHeader(fd);

    u_int8_t width = getWidth(fd);
    u_int8_t height = getHeight(fd);

    lseek(fd, imgHead.bfOffBits, SEEK_SET);
    color *color_mas = new color[width * height];

    for (int i = 0; i < width * height; i++)
        color_mas[i] = read_color(fd);

    close(fd);

    fd = open(videoInput.c_str(), O_RDWR); //open input video
    if (fd == -1)
        perror("open");

    struct stat st;
    fstat(fd, &st);
    int frame_size = vWidth * vHeight + vWidth * vHeight / 2; //image size / (Y + U + V) = frame count
    int frame_count = st.st_size / (frame_size);

    u_int8_t **frame = new u_int8_t*[frame_count];
    for (int i = 0; i < frame_count; i++)
        frame[i] = new u_int8_t[frame_size];

    for (int i = 0; i < frame_count; i++){
        read(fd, frame[i], frame_size);

        writeY(frame, color_mas, width, height, vWidth, i);
        writeU(frame, color_mas, width, height, vWidth, vHeight, i);
        writeV(frame, color_mas, width, height, vWidth, vHeight, i);
    }
    close(fd);

    fd = open(videoOutput.c_str(), O_RDWR | O_CREAT); //open output video
    if (fd == -1)
        perror("open");
    for (int i = 0; i < frame_count; i++)
        write(fd, frame[i], frame_size);
    close(fd);

    for (int i = 0; i < frame_count; i++)
        delete[]frame[i];
    delete[]frame;
    delete[]color_mas;
}


DWORD read_dword(int fd){
    u_int8_t byte_1 = 0;
    u_int8_t byte_2 = 0;
    u_int8_t byte_3 = 0;
    u_int8_t byte_4 = 0;

    read(fd, &byte_1, 1);
    read(fd, &byte_2, 1);
    read(fd, &byte_3, 1);
    read(fd, &byte_4, 1);

   return ((((((byte_4 << 8) | byte_3) << 8) | byte_2) << 8) | byte_1);
}
WORD read_word(int fd){
    unsigned char byte_1 = 0;
    unsigned char byte_2 = 0;

    read(fd, &byte_1, 1);
    read(fd, &byte_2, 1);

    return ((byte_1 << 8) | byte_2);
}

color read_color(int fd){
    color res_color;
    u_int8_t tmp[3] = {0, 0, 0};
    read(fd, tmp, 3);
    res_color.Blue = tmp[0];
    res_color.Green = tmp[1];
    res_color.Red = tmp[2];
    return res_color;
}

void writeY (u_int8_t **frame, color *color_mas, int iWidth, int iHeight, int vWidth, int ind){
    int f_c = 0;
    for (int i = iHeight - 1; i > -1; i--){
        for (int j = i * iWidth; j < (i + 1) * iWidth; j++){
            u_int8_t Y = ((66 * color_mas[j].Red + 129 * color_mas[j].Green + 25 * color_mas[j].Blue + 128) >> 8) + 16;
            frame[ind][f_c++] = Y;
        }
        f_c += (vWidth - iWidth);
    }
}
void writeU(u_int8_t **frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight, int ind){
    int h = iHeight / 2;
    h *= 2;

    int f_c = vWidth * vHeight + 1;
    for (int y = h - 1; y > -1; y -= 2){
        for (int x = 0; x < iWidth; x += 2){
            int i = y * iWidth + x;
            u_int8_t U = ((-38 * color_mas[i].Red - 74 * color_mas[i].Green + 112 * color_mas[i].Blue + 128) >> 8) + 128;
            frame[ind][f_c++] = U;
        }
        f_c += (vWidth - iWidth) / 2;
    }
}
void writeV(u_int8_t **frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight, int ind){
    int h = iHeight / 2;
    h *= 2;
    int f_c = vWidth * vHeight + (vWidth * vHeight / 4) + 1;
    for (int y = h - 1; y > -1; y -= 2){
        for (int x = 0; x < iWidth; x += 2){
            int i = y * iWidth + x;
            u_int8_t V = ((112 * color_mas[i].Red - 94 * color_mas[i].Green - 18 * color_mas[i].Blue + 128) >> 8) + 128;
            frame[ind][f_c++] = V;
        }
        f_c += (vWidth - iWidth) / 2;
    }
}

u_int8_t getWidth(int fd){
    lseek(fd, 18, SEEK_SET);
    return read_dword(fd);
}
u_int8_t getHeight(int fd){
    lseek(fd, 22, SEEK_SET);
    return read_dword(fd);
}

