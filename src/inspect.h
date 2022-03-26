#ifndef INSPECT_H
#define INSPECT_H
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <random>
#include <numeric>
#include <vector>
#include <unordered_set>
using namespace cv;

typedef struct pixelSet{
    std::vector<int> col;
    std::vector<int> row;
} PxSet;

// Code taken directly from: OpenCV website & https://tinyurl.com/2p8hw4rw
void outputMatHistogram(Mat* plane, std::string outputName) {
    int histSize = 256;
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange = { range };
    bool uniform = true, accumulate = false;
    Mat b_hist;
    calcHist( plane, 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );
    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    for( int i = 0; i < histSize; i++ )
    {
        line( histImage, Point(bin_w*(i), hist_h),
              Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
              Scalar( 255, 0, 0), 2, 8, 0  );
    }
    imwrite(outputName, histImage);
}
/*
std::vector<int> getSampleList(int N, int k, int seed) {
    std::vector<int> candidate(N-1);
    std::iota (candidate.begin(), candidate.end(), 1);
    std::shuffle(candidate.begin(), candidate.end(), std::default_random_engine(seed));
    return (std::vector<int> result(candidate.begin(), candidate.end()+k));
}

void getNeighbour(std::vector<int>* set, std::vector<int>* result, int direction) {
    for (auto *it = set->begin(); it != set->end(); it++) {
        result->push_back(*it + direction);
    }
    return;
}

double expectedPixelVal(Mat* srcImg, PxSet* set) {
    auto *colIter = set->col.begin();
    auto *rowIter = set->row.begin();
    int sizeN = set->col.length();
    double result = 0;
    while ((colIter != set->col.end()) && (rowIter != set->row.end())) {
        result += srcImg->at<uchar>(*rowIter, *colIter) / sizeN;
    }
    return result;
}

double expectedPixelVal_square(Mat* srcImg, PxSet* set) {
    auto *colIter = set->col.begin();
    auto *rowIter = set->row.begin();
    int sizeN = set->col.length();
    double result = 0;
    while ((colIter != set->col.end()) && (rowIter != set->row.end())) {
        double pxVal = srcImg->at<uchar>(*rowIter, *colIter);
        result += (pxVal * pxVal) / sizeN;
    }
    return result;
}

void calCorrelation(Mat *img, int sampleSize) {
    // === Randomly select sampleSize pixel as base ===
    PxSet basePixels;
    basePixels.col = getSampleList(img->cols, sampleSize, 1234);
    basePixels.row = getSampleList(img->rows, sampleSize, 1234);
    double basePixel_expectedVal = expectedPixelVal(img, &basePixels);
    double basePixel_variance = expectedPixelVal_square(img, &basePixels) - (basePixel_expectedVal * basePixel_expectedVal);
    // === Horizontal pixels ===
    PxSet hori_neighbour;
    getNeighbour(&basePixels.row, &hori_neighbour.row, -1);
    std::copy(basePixels.col.begin(), basePixels.col.end(), std::back_inserter(hori_neighbour.col));
    double horiPixel_expectedVal = expectedPixelVal(img, &hori_neighbour);
    double horiPixel_variance = expectedPixelVal_square(img, &hori_neighbour) - (horiPixel_expectedVal * horiPixel_expectedVal);
    // === Vertical pixels ===
    PxSet verti_neighbour;
    getNeighbour(&basePixels.col, &verti_neighbour.col, -1);
    std::copy(basePixels.row.begin(), basePixels.row.end(), std::back_inserter(verti_neighbour.row));
    double vertiPixel_expectedVal = expectedPixelVal(img, &verti_neighbour);
    double vertiPixel_variance = expectedPixelVal_square(img, &verti_neighbour) - (vertiPixel_expectedVal * vertiPixel_expectedVal);
    
    // Horizontal variance
    double hori_denom = std::sqrt(basePixel_variance * horiPixel_variance);
    std::vector<double> hori_corCoe;
    auto *base_ColIter = basePixels->col.begin();
    auto *base_RowIter = basePixels->row.begin();
    auto *hori_RowIter = hori_neighbour->row.begin();
    while (base_ColIter != basePixels->col.end()) {
        unsigned char pixel = img->at<uchar>(*base_RowIter, *base_ColIter);
        unsigned char pixel_neighour = img->at<uchar>(*hori_RowIter, *base_ColIter);
        numerator.push_back(std::abs(pixel - basePixel_expectedVal) * std::abs(pixel_neighour - horiPixel_expectedVal));
        base_ColIter++;
        base_RowIter++;
        hori_RowIter++;
    }

    std::vector<double> numerator;
    auto *base_ColIter = basePixels->col.begin();
    auto *base_RowIter = basePixels->row.begin();
    auto *hori_RowIter = hori_neighbour->row.begin();
    while (base_ColIter != basePixels->col.end()) {
        unsigned char pixel = img->at<uchar>(*base_RowIter, *base_ColIter);
        unsigned char pixel_neighour = img->at<uchar>(*hori_RowIter, *base_ColIter);
        numerator.push_back(std::abs(pixel - basePixel_expectedVal) * std::abs(pixel_neighour - horiPixel_expectedVal));
        base_ColIter++;
        base_RowIter++;
        hori_RowIter++;
    }
    double numerator_val = 0;
    for (auto *it = numerator.begin(); it != numerator.end(); it++) {
        numerator_val += *it / sampleSize;
    }


}
*/
#endif