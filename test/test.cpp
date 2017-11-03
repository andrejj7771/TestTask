#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <cstdint>
#include <thread>
#include <ctime>
#include "../image.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE(Tests)
BOOST_AUTO_TEST_CASE(ConvertTest){
    srand(time(0));
    uint32_t Width = 2000;
    uint32_t Height = 2000;
    uint8_t *mas = new uint8_t[Width * Height * 3];
    uint8_t *yuvT = new uint8_t[Width * Height + (Width * Height / 2)];
    uint8_t *yuvS = new uint8_t[Width * Height + (Width * Height / 2)];

    for (int i = 0; i < 3 * Width * Height; i++)
        mas[i] = rand() % 256;

    std::thread th1(rgb2yuvPart, Width, Height, Height / 4, Height / 4 - 1, std::ref(mas), std::ref(yuvT));
    std::thread th2(rgb2yuvPart, Width, Height, Height / 4, Height / 2 - 1, std::ref(mas), std::ref(yuvT));
    std::thread th3(rgb2yuvPart, Width, Height, Height / 4, 3 * Height / 4 - 1, std::ref(mas), std::ref(yuvT));
    std::thread th4(rgb2yuvPart, Width, Height, Height / 4, Height - 1, std::ref(mas), std::ref(yuvT));

    yuvS = rgb2yuv(Width, Height, mas);

    if (th1.joinable())
        th1.join();
    if (th2.joinable())
        th2.join();
    if (th3.joinable())
        th3.join();
    if (th4.joinable())
        th4.join();

    for (int i = 0; i < Width * Height + (Width * Height / 2); i++)
        BOOST_CHECK_EQUAL(yuvS[i], yuvT[i]);
}
BOOST_AUTO_TEST_SUITE_END()

