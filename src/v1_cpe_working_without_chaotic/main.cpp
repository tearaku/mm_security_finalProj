#include "cpe_approach.h"
#include <stdio.h>
using namespace cv;

int main(int argc, char** argv ) {
    if (argc != 2) {
        printf("usage: main <Image_Path>\n");
        return -1;
    }
    Mat image;
    image = imread(argv[1], 1);
    if (!image.data) {
        printf("No image data.\n");
        return -1;
    }
    cvtColor(image, image, COLOR_RGB2GRAY);
    cpe_approach(&image, 1234, 5678);
    imwrite("test_img.png", image);
    return 0;
}