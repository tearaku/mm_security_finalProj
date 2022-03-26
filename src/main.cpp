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

void seeKeyVal(double* msgKey, struct imageSecretKey* imgKey, int seed, unsigned int delta1, unsigned int delta2) {
    std::minstd_rand0 gen(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_real_distribution<double> dist2(0.0, 0.5);
    std::cout << "msgKey: " << dist(gen) << std::endl;
    std::cout << "imgKey1(p_val, init_val): " << dist2(gen) << ", " << dist(gen) << std::endl;
    std::cout << "imgKey2(p_val, init_val): " << dist2(gen) << ", " << dist(gen) << std::endl;
    std::cout << "delta vals(delta_1, delta_2): " << delta1 << ", " << delta2 << std::endl;
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
    // Randomly generate necessary keys
    int seed = 530181060;
    unsigned int delta_1 = 7;
    unsigned int delta_2 = 21;
    double msgKey;
    struct imageSecretKey imgKey;
    //seeKeyVal(&msgKey, &imgKey, seed, delta_1, delta_2);
    genKeys(&msgKey, &imgKey, seed, delta_1, delta_2);
    std::string secretMsg = "Shh, very secret.";
    // Generate ciphertext C_1 of image
    cpe_approach(&image, secretMsg, msgKey, imgKey, 1);
    imwrite("output_1.png", image);
    // Generate ciphertext C_2 of image, after modifying one pixel (taking its complement)
    image = imread(argv[1], IMREAD_GRAYSCALE);
    image.at<uchar>(128, 128) = 0;
    cpe_approach(&image, secretMsg, msgKey, imgKey, 2);
    imwrite("output_2.png", image);
    return 0;
}