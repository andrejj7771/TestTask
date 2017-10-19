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

struct bmpHeader{
    WORD bfType = 0;
    DWORD bfSize = 0;
    WORD bfReserved1 = 0;
    WORD bfReserved2 = 0;
    DWORD bfOffBits = 0;
};
struct color{
    WORD Red = 0;
    WORD Green = 0;
    WORD Blue = 0;

    void print(){
        cout << "Red: " << Red << endl;
        cout << "Green: " << Green << endl;
        cout << "Blue: " << Blue << endl;
    }
};

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

void writeY(color *mas, int width, int height, int fd){
    for (int i = height - 1; i > -1; i--){
        for (int j = i * width; j < (i + 1) * width; j++){
            WORD Y = ((66 * mas[j].Red + 129 * mas[j].Green + 25 * mas[j].Blue + 128) >> 8) + 16;
            write(fd, &Y, 1);
        }
    }
}
void writeU(color *mas, int width, int height, int fd){
    int h = height / 2;
    h *= 2;
    lseek(fd, width * height, SEEK_SET);
    for (int y = h - 1; y > -1; y -= 2){
        for (int x = 0; x < width; x += 2){
            int i = y * width + x;
            WORD U = ((-38 * mas[i].Red - 74 * mas[i].Green + 112 * mas[i].Blue + 128) >> 8) + 128;
            write(fd, &U, 1);
        }
    }
}
void writeV(color *mas, int width, int height, int fd){
    int h = height / 2;
    h *= 2;
    lseek(fd, width * height + width * height / 4, SEEK_SET);
    for (int y = h - 1; y > -1; y -= 2)
        for (int x = 0; x < width; x += 2){
            int i = y * width + x;
            WORD V = ((112 * mas[i].Red - 94 * mas[i].Green - 18 * mas[i].Blue + 128) >> 8) + 128;
            write(fd, &V, 1);
        }
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

int main(){
    string imgInput = "/home/andrey/nokia.bmp";
    string videoInput = "/home/andrey/input.yuv";
    string videoOutput = "/home/andrey/output.yuv";

    WORD vWidth = 352;
    WORD vHeight = 288;

    int fd = open(imgInput.c_str(), O_RDONLY); //open image
    if (fd == -1)
        perror("open file error");

    bmpHeader imgHead = ReadHeader(fd);

    DWORD width = getWidth(fd);
    DWORD height = getHeight(fd);

    lseek(fd, imgHead.bfOffBits, SEEK_SET);
    color *color_mas = new color[width * height];

    for (DWORD i = 0; i < width * height; i++)
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

    for (int n = 0; n < frame_count; n++){
        int f_c = 0;
        read(fd, frame[n], frame_size);

        for (int i = height - 1; i > -1; i--){
            for (int j = i * width; j < (i + 1) * width; j++){
                WORD Y = ((66 * color_mas[j].Red + 129 * color_mas[j].Green + 25 * color_mas[j].Blue + 128) >> 8) + 16;
                frame[n][f_c++] = Y;
            }
            f_c += (vWidth - width);
        }

        int h = height / 2;
        h *= 2;

        f_c = vWidth * vHeight + 1;
        for (int y = h - 1; y > -1; y -= 2){
            for (int x = 0; x < width; x += 2){
                int i = y * width + x;
                WORD U = ((-38 * color_mas[i].Red - 74 * color_mas[i].Green + 112 * color_mas[i].Blue + 128) >> 8) + 128;
                frame[n][f_c++] = U;
            }
            f_c += (vWidth - width) / 2;
        }

        f_c = vWidth * vHeight + (vWidth * vHeight / 4) + 1;
        for (int y = h - 1; y > -1; y -= 2){
            for (int x = 0; x < width; x += 2){
                int i = y * width + x;
                WORD V = ((112 * color_mas[i].Red - 94 * color_mas[i].Green - 18 * color_mas[i].Blue + 128) >> 8) + 128;
                frame[n][f_c++] = V;
            }
            f_c += (vWidth - width) / 2;
        }

    }
    close(fd);

    fd = open(videoOutput.c_str(), O_RDWR | O_CREAT);
    if (fd == -1)
        perror("open");
    for (int i = 0; i < frame_count; i++)
        write(fd, frame[i], frame_size);
    close(fd);
}
