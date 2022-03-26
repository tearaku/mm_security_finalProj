#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include "cpe_approach.h"
#include "chaoticGenerator.h"
using namespace cv;
using namespace std;

#define MSG_CONTROL_CHAR 4

class MsgBitStream{
    minstd_rand0* generator;
    string* msg;
    unsigned char* msgPtr;
    unsigned char* msgPtrEnd;
    unsigned char curMsgChar;
    int curPosInByte;
public:
    bool hasNextBit() {
        return (msgPtr != msgPtrEnd) || (curPosInByte != CHAR_BIT-1);
    }
    void encrypt() {
        for (unsigned char* it = msgPtr; it != msgPtrEnd; it++) {
            *it = *it ^ (*generator)();
        }
        return;
    }
    // Returns 1 for bit 1, 0 for bit 0
    unsigned char getNextBit() {
        if (curPosInByte++ == CHAR_BIT-1) {
            curMsgChar = *msgPtr;
            msgPtr++;
            curPosInByte = 0;
        }
        unsigned char msgMSB = ((curMsgChar & 0x80) == 0x80) ? 1: 0;
        curMsgChar = curMsgChar << 1;
        return msgMSB;
    }
    MsgBitStream(minstd_rand0* device_ptr, string* _msg) {
        msg = _msg;
        generator = device_ptr;
        curPosInByte = CHAR_BIT-1;
        msgPtr = (unsigned char*)malloc(sizeof(unsigned char)*_msg->length());
        msgPtrEnd = msgPtr + _msg->length();
        copy(msg->begin(), msg->end(), msgPtr);        
        curMsgChar = 0;
    }
};

void imageEncryption(Mat* srcImg, struct imageSecretKey imgKey);
void preProcess(Mat* srcImg);
void dataEmbedding(Mat* srcImg, string* msg, int32_t msgKey);
void dataExtraction(Mat* cipherImg, struct imageSecretKey imgKey, int32_t msgKey);

// TO-DO: replace the all the minstd_rand0 used here with 1D chaotic PWLM, use fixed-point arithmetic (including key?)
void cpe_approach(Mat* srcImg, string secretMsg, int32_t msgKey, struct imageSecretKey imgKey) {
    preProcess(srcImg);
    imageEncryption(srcImg, imgKey);

    imwrite("cipherImg.png", *srcImg);
    return;
}

void preProcess(Mat* srcImg) {
    Mat processedImg = srcImg->clone();
    for (int row = 1; row < srcImg->rows; row++) {
        for (int col = 1; col < srcImg->cols; col++) {
            int inv_pixel = ((int)(srcImg->at<uchar>(row, col)) + 128) % 256;
            int pred_pixel = ((int)(srcImg->at<uchar>(row-1, col)) + (int)(srcImg->at<uchar>(row, col)-1)) / 2;
            int delta_diff = abs(pred_pixel - (int)(srcImg->at<uchar>(row, col)));
            int delta_inv_diff = abs(pred_pixel - inv_pixel);
            if (delta_diff >= delta_inv_diff) {
                if ((int)(srcImg->at<uchar>(row, col)) < 128) {
                    processedImg.at<uchar>(row, col) = pred_pixel - 63;
                } else {
                    processedImg.at<uchar>(row, col) = pred_pixel + 63;
                }
            }
        }
    }
    srcImg = &processedImg;
    return;
}

void imageEncryption(Mat* srcImg, struct imageSecretKey imgKey) {
    CPRBG generator(imgKey.key1, imgKey.key2, imgKey.delta);
    vector<uint8_t> pseudoBytes_vec(srcImg->cols * srcImg->rows);
    for (auto it = pseudoBytes_vec.begin(); it != pseudoBytes_vec.end(); it++) {
        *it = generator.getNextByte();
    }

    Mat bgr_planes(srcImg->rows, srcImg->cols, CV_8U, pseudoBytes_vec.data());
    int histSize = 256;
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange = { range };
    bool uniform = true, accumulate = false;
    Mat b_hist;
    calcHist( &bgr_planes, 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
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
    imwrite("calcHist_Demo.png", histImage);
    return;
}