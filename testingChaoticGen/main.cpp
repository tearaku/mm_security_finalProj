#include "cpe_approach.h"
#include "customStruct.h"
#include <stdio.h>
#include <random>
using namespace cv;

void genKeys(double* msgKey, struct imageSecretKey* imgKey, int seed, unsigned int delta1, unsigned int delta2) {
    std::minstd_rand0 gen(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_real_distribution<double> dist2(0.0, 0.5);
    *msgKey = dist(gen);
    imgKey->key1 = std::make_pair(dist2(gen), dist(gen));
    imgKey->key2 = std::make_pair(dist2(gen), dist(gen));
    imgKey->delta = std::make_pair(delta1, delta2);
    return;
}

int main(int argc, char** argv ) {
    if (argc != 2) {
        printf("usage: main <Image_Path>\n");
        return -1;
    }
    Mat image;
    image = imread(argv[1], IMREAD_GRAYSCALE);
    if (!image.data) {
        printf("No image data.\n");
        return -1;
    }
    //cvtColor(image, image, COLOR_RGB2GRAY);
    // Randomly generate necessary keys
    int seed = 530181060;
    unsigned int delta_1 = 7;
    unsigned int delta_2 = 21;
    double msgKey;
    struct imageSecretKey imgKey;
    genKeys(&msgKey, &imgKey, seed, delta_1, delta_2);
    std::string secretMsg = "Shh, very secret.";
    // Begin
    cpe_approach(&image, secretMsg, msgKey, imgKey);
    imwrite("test_img.png", image);
    return 0;
}