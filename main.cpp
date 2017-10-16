#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstdlib>
#include <cstdio>

#define WORD u_int16_t
#define DWORD u_int32_t

using namespace std;

struct bmpheader{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
};


DWORD read_dword(int fd){
    unsigned char byte_1[1] = {0};
    unsigned char byte_2[1] = {0};
    unsigned char byte_3[1] = {0};
    unsigned char byte_4[1] = {0};

    read(fd, byte_1, 1);
    read(fd, byte_2, 1);
    read(fd, byte_3, 1);
    read(fd, byte_4, 1);

   return ((((((byte_4[0] << 8) | byte_3[0]) << 8) | byte_2[0]) << 8) | byte_1[0]);
}
WORD read_word(int fd){
    unsigned char byte_1[1];
    unsigned char byte_2[1];

    read(fd, byte_1, 1);
    read(fd, byte_2, 1);

    return ((byte_1[0] << 8) | byte_2[0]);
}

bmpheader ReadHeader(int fd){
    bmpheader res;
    res.bfType = read_word(fd);
    res.bfSize = read_dword(fd);
    res.bfReserved1 = read_word(fd);
    res.bfReserved2 = read_word(fd);
    res.bfOffBits = read_dword(fd);
    return res;
}


int main()
{
    int fd = open("/home/andrey/img.bmp", O_RDWR);

    if (fd == -1)
        perror("Open");

    bmpheader imgHead = ReadHeader(fd);
    cout << hex << imgHead.bfType << endl;
    cout << dec << imgHead.bfSize << endl;
    cout << imgHead.bfReserved1 << endl;
    cout << imgHead.bfReserved2 << endl;
    cout << imgHead.bfOffBits << endl;
    return 0;
}
