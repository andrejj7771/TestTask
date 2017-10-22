#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <GL/glut.h>
#include <GL/glu.h>
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

WORD getHeight(int fd);
WORD getWidth(int fd);

void writeY(u_int8_t *frame, color *color_mas, int iWidth, int iHeight, int vWidth);
void writeU(u_int8_t *frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight);
void writeV(u_int8_t *frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight);
void writeFrames(u_int8_t **frames, color *color_mas, int frame_count, int iWidth, int iHeight, int vWidth, int vHeight);
u_int8_t *RGBVideo(u_int8_t *Y, u_int8_t *U, u_int8_t *V, WORD w, WORD h);
u_int8_t *colorVideo(int *tmp, u_int8_t *U, u_int8_t *V, WORD w, WORD h);

/*GL funtions*/
void Display();
GLint loadFrame(u_int8_t *frame, int w, int h);
void delFrame(GLuint texture);
void drawFrame();

GLint texture = 0;
u_int8_t **frame;
int main(int argc, char **argv){
    string imgInput = "/home/andrey/nokia.bmp";
    string videoInput = "/home/andrey/input.yuv";
    string videoOutput = "/home/andrey/output.yuv";

    int vWidth = 352;
    int vHeight = 288;

    int fd = open(imgInput.c_str(), O_RDONLY); //open image
    if (fd == -1){
        perror("open file error");
        return -1;
    }

    bmpHeader imgHead;
    imgHead.ReadHeader(fd);

    WORD width = getWidth(fd);
    WORD height = getHeight(fd);

    lseek(fd, imgHead.bfOffBits, SEEK_SET);
    color *color_mas = new color[width * height];

    for (int i = 0; i < width * height; i++)
        color_mas[i] = read_color(fd);

    close(fd);

    fd = open(videoInput.c_str(), O_RDWR); //open input video
    if (fd == -1){
        perror("open");
        return -1;
    }

    struct stat st;
    fstat(fd, &st);
    int frame_size = vWidth * vHeight + vWidth * vHeight / 2; //image size / (Y + U + V) = frame count
    int frame_count = st.st_size / (frame_size);

    frame = new u_int8_t*[frame_count];
    for (int i = 0; i < frame_count; i++)
        frame[i] = new u_int8_t[frame_size];

    float start = clock();
    for (int i = 0; i < frame_count; i++){
        read(fd, frame[i], frame_size);
    }
    close(fd);

    thread wFrames(writeFrames, ref(frame), ref(color_mas), frame_count, width, height, vWidth, vHeight);
    //writeFrames(frame, color_mas, frame_count, width, height, vWidth, vHeight);

    if (wFrames.joinable())
        wFrames.join();
    cout << round(((clock() - start) / CLOCKS_PER_SEC) * 1000) / 1000 << endl;

    fd = open(videoOutput.c_str(), O_RDWR | O_CREAT); //open/create output video
    if (fd == -1)
        perror("open");
    for (int i = 0; i < frame_count; i++)
        write(fd, frame[i], frame_size);
    close(fd);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Video");
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, vWidth, 0, vHeight);
    glutDisplayFunc(Display);
    glutMainLoop();


    for (int i = 0; i < frame_count; i++)
        delete[]frame[i];
    delete[]frame;
    delete[]color_mas;

    return 0;
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

void writeY (u_int8_t *frame, color *color_mas, int iWidth, int iHeight, int vWidth){
    int f_c = 0;
    for (int i = iHeight - 1; i > -1; i--){
        for (int j = i * iWidth; j < (i + 1) * iWidth; j++){
            u_int8_t Y = ((66 * color_mas[j].Red + 129 * color_mas[j].Green + 25 * color_mas[j].Blue + 128) >> 8) + 16;
            frame[f_c++] = Y;
        }
        f_c += (vWidth - iWidth);
    }
}
void writeU(u_int8_t *frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight){
    int f_c = vWidth * vHeight + 1;
    for (int y = iHeight - 1; y > -1; y -= 2){
        for (int x = 0; x < iWidth; x += 2){
            int i = y * iWidth + x;
            u_int8_t U = ((-38 * color_mas[i].Red - 74 * color_mas[i].Green + 112 * color_mas[i].Blue + 128) >> 8) + 128;
            frame[f_c++] = U;
        }
        f_c += (vWidth - iWidth) / 2;
    }
}
void writeV(u_int8_t *frame, color *color_mas, int iWidth, int iHeight, int vWidth, int vHeight){
    int f_c = vWidth * vHeight + (vWidth * vHeight / 4) + 1;
    for (int y = iHeight - 1; y > -1; y -= 2){
        for (int x = 0; x < iWidth; x += 2){
            int i = y * iWidth + x;
            u_int8_t V = ((112 * color_mas[i].Red - 94 * color_mas[i].Green - 18 * color_mas[i].Blue + 128) >> 8) + 128;
            frame[f_c++] = V;
        }
        f_c += (vWidth - iWidth) / 2;
    }
}
void writeFrames(u_int8_t **frame, color *color_mas, int frame_count, int iWidth, int iHeight, int vWidth, int vHeight){
    for (int i = 0; i < frame_count; i++){
        writeY(frame[i], color_mas, iWidth, iHeight, vWidth);
        writeU(frame[i], color_mas, iWidth, iHeight, vWidth, vHeight);
        writeV(frame[i], color_mas, iWidth, iHeight, vWidth, vHeight);
    }
}

WORD getWidth(int fd){
    lseek(fd, 18, SEEK_SET);
    return read_dword(fd);
}
WORD getHeight(int fd){
    lseek(fd, 22, SEEK_SET);
    return read_dword(fd);
}
void Display(){
    glEnable(GL_TEXTURE_2D);
    for (int i = 0; i < 300; i++){
        glClear(GL_COLOR_BUFFER_BIT);
        texture = loadFrame(frame[i], 352, 288);
        drawFrame();
        delFrame(texture);

        glutSwapBuffers();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
    }
    glutSwapBuffers();
}
GLint loadFrame(u_int8_t *frame, int w, int h){
    GLuint texture;

    u_int8_t *Y = new u_int8_t[w * h];
    u_int8_t *U = new u_int8_t[w * h / 4];
    u_int8_t *V = new u_int8_t[w * h / 4];

    for (int i = 0; i < w * h; i++)
        Y[i] = frame[i];

    int offset = (w * h) + (w * h / 4);
    for (int i = w * h; i < offset; i++)
        U[i - (w * h)] = frame[i];

    for (int i = offset; i < offset + (w * h / 4); i++)
        V[i - offset] = frame[i];

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_LINEAR);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,  RGBVideo(Y, U, V, w, h));

    return texture;
}

u_int8_t *RGBVideo(u_int8_t *Y, u_int8_t *U, u_int8_t *V, WORD w, WORD h){
    int *tmp = new int[w * h * 3];

    for (int i = (w * 3)*(h - 1); i >= 0; i -= w * 3){
        for (int j = 0; j < w; j++, Y++){
            tmp[i + 3 * j] =  (*Y - 16);             //R
            tmp[i + 3 * j + 1] = (*Y - 16);        //G
            tmp[i + 3 * j + 2] = (*Y - 16);        //B
        }
    }
    return colorVideo(tmp, U, V, w, h);

}
u_int8_t *colorVideo(int *tmp, u_int8_t *U, u_int8_t *V, WORD w, WORD h){
    u_int8_t *imageData = new u_int8_t[w * h * 3];
    for (int i = (h * w * 3) - (w * 3 * 2); i >= 0; i -= w * 3 * 2){
        for (int j = 0; j < w; j++){
            if (j != 0 && j % 2){
                V++;
                U++;
            }
            tmp[i + 3 * j] += 1.370705 * (*V - 128);                                         //R
            if (tmp[i + 3 * j] < 0)
                tmp[i + 3 * j] = 0;
            if (tmp[i + 3 * j] > 255)
                tmp[i + 3 * j] = 255;
            imageData[i + 3 * j] = (u_int8_t)tmp[i + 3 * j] * 220 / 256;

            tmp[i + 3 * j + 1] -= 0.698001 * (*U - 128) - 0.337633 * (*V - 128) + 10;        //G
            if (tmp[i + 3 * j + 1] < 0)
                tmp[i + 3 * j + 1] = 0;
            if (tmp[i + 3 * j + 1] > 255)
                tmp[i + 3 * j + 1] = 255;
            imageData[i + 3 * j + 1] = (u_int8_t)tmp[i + 3 * j + 1] * 220 / 256;

            tmp[i + 3 * j + 2] += 1.732446 * (*U - 128);                                     //B
            if (tmp[i + 3 * j + 2] < 0)
                tmp[i + 3 * j + 2] = 0;
            if (tmp[i + 3 * j + 2] > 255)
                tmp[i + 3 * j + 2] = 255;
            imageData[i + 3 * j + 2] = (u_int8_t)tmp[i + 3 * j + 2] * 220 / 256;

            /*---------------------------------------------------------------------------------------------*/

            tmp[i + 3 * (j + w)] += 1.370705 * (*V - 128);                                   //R
            if (tmp[i + 3 * (j + w)] < 0)
                tmp[i + 3 * (j + w)] = 0;
            if (tmp[i + 3 * (j + w)] > 255)
                tmp[i + 3 * (j + w)] = 255;
            imageData[i + 3 * (j + w)] = (u_int8_t)tmp[i + 3 * (j + w)] * 220 / 256;

            tmp[i + 3 * (j + w) + 1] -= 0.698001 * (*U - 128) - 0.337633 * (*V - 128) + 10;  //G
            if (tmp[i + 3 * (j + w) + 1] < 0)
                tmp[i + 3 * (j + w) + 1] = 0;
            if (tmp[i + 3 * (j + w) + 1] > 255)
                tmp[i + 3 * (j + w) + 1] = 255;
            imageData[i + 3 * (j + w) + 1] = (u_int8_t)tmp[i + 3 * (j + w) + 1] * 220 / 256;

            tmp[i + 3 * (j + w) + 2] += 1.732446 * (*U - 128);                               //B
            if (tmp[i + 3 * (j + w) + 2] < 0)
                tmp[i + 3 * (j + w) + 2] = 0;
            if (tmp[i + 3 * (j + w) + 2] > 255)
                tmp[i + 3 * (j + w) + 2] = 255;
            imageData[i + 3 * (j + w) + 2] = (u_int8_t)tmp[i + 3 * (j + w) + 2] * 220 / 256;

        }
    }
    return imageData;
}

void delFrame(GLuint texture){
    glDeleteTextures(1, &texture);
}
void drawFrame(){
    glBindTexture(GL_TEXTURE_2D, texture);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2d(0, 0);
        glTexCoord2d(1, 0); glVertex2d(352, 0);
        glTexCoord2d(1, 1); glVertex2d(352, 288);
        glTexCoord2d(0, 1); glVertex2d(0, 288);
    glEnd();
    glDisable(GL_TEXTURE_2D);

}

