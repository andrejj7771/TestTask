#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstdlib>
#include <cstdio>

#define WORD u_int16_t
#define DWORD u_int32_t

using namespace std;

WORD read_word(int fd){
    unsigned char byte_1[1];
    unsigned char byte_2[1];

    read(fd, byte_1, 1);
    read(fd, byte_2, 1);

    return ((byte_1[0] << 8) | byte_2[0]);
}
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

int main()
{
    int fd = open("/home/andrey/img.bmp", O_RDWR);
    char *buff = new char[1];

    if (fd == -1)
        perror("Open");
    cout << hex << read_word(fd) << endl ;
    cout << dec << read_dword(fd) << endl;
    cout << read_word(fd) << endl ;
    cout << read_word(fd) << endl ;
    DWORD seek = read_dword(fd);
    cout << seek << endl;
    return 0;
}
