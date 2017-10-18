#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#define WORD u_int16_t
#define DWORD u_int32_t

#define WIDTH 320
#define HEIGHT 240

using namespace std;

struct bmpHeader{
    WORD bfType = 0;
    DWORD bfSize = 0;
    WORD bfReserved1 = 0;
    WORD bfReserved2 = 0;
    DWORD bfOffBits = 0;
};
struct imgInfo{

};
struct color{
    unsigned int Red = 0;
    unsigned int Green = 0;
    unsigned int Blue = 0;

    void print(){
        cout << "Red: " << Red << endl;
        cout << "Green: " << Green << endl;
        cout << "Blue: " << Blue << endl;
    }
};

DWORD read_dword(int fd){
    unsigned char byte_1 = 0;
    unsigned char byte_2 = 0;
    unsigned char byte_3 = 0;
    unsigned char byte_4 = 0;

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
    unsigned char tmp[3] = {0, 0, 0};
    read(fd, tmp, 3);
    res_color.Blue = (unsigned int)tmp[0];
    res_color.Green = (unsigned int)tmp[1];
    res_color.Red = (unsigned int)tmp[2];
    return res_color;
}

void saveYUV(color *mas, int width, int height){
    int size = width * height;

    int fd = open("/home/andrey/f.yuv", O_WRONLY);
    if (fd < 0)
        perror("open file error");

    for (int i = height - 1; i > -1; i--)
        for (int j = i*width; j < (i + 1) * width; j++){
           u_int8_t Y = ((66 * mas[j].Red + 129 * mas[j].Green + 25 * mas[j].Blue + 128) >> 8) + 16;
           write(fd, &Y, 1);
        }
    int h = height / 2;
    h *= 2;
    for (int y = h - 1; y > -1; y -= 2)
        for (int x = 0; x < width; x += 2){
            int i = y * width + x;
            u_int8_t U = ((-38 * mas[i].Red - 74 * mas[i].Green + 112 * mas[i].Blue + 128) >> 8) + 128;
            write(fd, &U, 1);
        }
    for (int y = h - 1; y > -1; y -= 2)
        for (int x = 0; x < width; x += 2){
            int i = y * width + x;
            u_int8_t V = ((112 * mas[i].Red - 94 * mas[i].Green - 18 * mas[i].Blue + 128) >> 8) + 128;
            write(fd, &V, 1);
        }
    close(fd);
}
bmpHeader ReadHeader(int fd){
    bmpHeader res;
    res.bfType = read_word(fd);
    res.bfSize = read_dword(fd);
    res.bfReserved1 = read_word(fd);
    res.bfReserved2 = read_word(fd);
    res.bfOffBits = read_dword(fd);
    return res;
}

DWORD getWidth(int fd){
    lseek(fd, 18, SEEK_SET);
    return read_dword(fd);
}
DWORD getHeight(int fd){
    lseek(fd, 22, SEEK_SET);
    return read_dword(fd);
}
int main()
{
    int fd = open("/home/andrey/tiger.bmp", O_RDWR);

    if (fd == -1)
        perror("Open");

    bmpHeader imgHead = ReadHeader(fd);

    DWORD width = getWidth(fd);
    DWORD height = getHeight(fd);

    lseek(fd, imgHead.bfOffBits, SEEK_SET);
    color *color_mas = new color[width * height];

    for (int i = 0; i < width * height; i++)
            color_mas[i] = read_color(fd);

    close(fd);

    saveYUV(color_mas, width, height);
    return 0;
}
