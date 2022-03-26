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
    PRBG generator;
    string* msg;
    unsigned char* msgPtr;
    unsigned char* msgPtrEnd;
    unsigned char curMsgChar;
    int curPosInByte;
public:
    bool hasNextBit() {
        return (msgPtr != msgPtrEnd) || (curPosInByte != (sizeof(unsigned char)*8)-1);
    }
    void encrypt() {
        for (unsigned char* it = msgPtr; it != msgPtrEnd; it++) {
            *it = *it ^ (generator.getNextByte() & 0xff);
        }
        return;
    }
    // Returns 1 for bit 1, 0 for bit 0
    unsigned char getNextBit() {
        if (curPosInByte++ == (sizeof(unsigned char)*8)-1) {
            curMsgChar = *msgPtr;
            msgPtr++;
            curPosInByte = 0;
        }
        unsigned char msgMSB = ((curMsgChar & 0x80) == 0x80) ? 1: 0;
        curMsgChar = curMsgChar << 1;
        return msgMSB;
    }
    MsgBitStream(PRBG genClass, string* _msg) {
        msg = _msg;
        generator = genClass;
        curPosInByte = (sizeof(unsigned char)*8)-1;
        msgPtr = (unsigned char*)malloc(sizeof(unsigned char)*_msg->length());
        msgPtrEnd = msgPtr + _msg->length();
        copy(msg->begin(), msg->end(), msgPtr);        
        curMsgChar = 0;
    }
};

void imageEncryption(Mat* srcImg, int32_t imgKey);
void preProcess(Mat* srcImg);
void dataEmbedding(Mat* srcImg, string* msg, int32_t msgKey);
void dataExtraction(Mat* cipherImg, int32_t imgKey, int32_t msgKey);

// TO-DO: replace the all the minstd_rand0 used here with 1D chaotic PWLM, use fixed-point arithmetic (including key?)
void cpe_approach(Mat* srcImg, int32_t imgKey, int32_t msgKey) {
    preProcess(srcImg);
    imageEncryption(srcImg, imgKey);
    string testStr = "Yooz, test.";
    testStr.push_back(static_cast<char>(MSG_CONTROL_CHAR));
    dataEmbedding(srcImg, &testStr, msgKey);
    dataExtraction(srcImg, imgKey, msgKey);
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

void imageEncryption(Mat* srcImg, int32_t imgKey) {
    minstd_rand0 generator (imgKey);
    vector<uint8_t> pseudoBytes_vec(srcImg->cols * srcImg->rows);
    for (auto it = pseudoBytes_vec.begin(); it != pseudoBytes_vec.end(); it++) {
        *it = generator() % 0xff;
    }
    Mat pseudoBytes(srcImg->rows, srcImg->cols, CV_8U, pseudoBytes_vec.data());
    bitwise_xor(*srcImg, pseudoBytes, *srcImg);
    return;
}

void dataEmbedding(Mat* srcImg, string* msg, int32_t msgKey) {
    minstd_rand0 generator (msgKey);
    PRBG genClass(&generator, msgKey);
    // Encrypt message
    MsgBitStream msgStream(genClass, msg);
    msgStream.encrypt();
    // Data embedding: if msg is larger than allowed capacity, it simply terminates
    for (int row = 1; row < srcImg->rows; row++) {
        for (int col = 1; col < srcImg->cols; col++) {
            if (msgStream.hasNextBit() == false) {
                goto dataEmbedding_loopEnd;
            }
            srcImg->at<uchar>(row, col) ^= (-msgStream.getNextBit() ^ srcImg->at<uchar>(row, col)) & (0x80);
        }
    }
    dataEmbedding_loopEnd:
    return;
}

void dataExtraction(Mat* cipherImg, int32_t imgKey, int32_t msgKey) {
    // Message extraction & decryption
    minstd_rand0 generator_msg (msgKey);
    PRBG genClass(&generator_msg, msgKey);
    vector<unsigned char> secretMsg;
    unsigned int bitCount = 0;
    unsigned char curByte = 0;
    for (int row = 1; row < cipherImg->rows; row++) {
        for (int col = 1; col < cipherImg->cols; col++) {
            if (bitCount == 8) {
                secretMsg.push_back(curByte ^ genClass.getNextByte());
                if (secretMsg.back() == MSG_CONTROL_CHAR) {
                    goto dataExtraction_msgLoopEnd;
                }
                bitCount = 0;
                curByte = 0;
            }
            curByte |= ((cipherImg->at<uchar>(row, col) & 0x80) >> bitCount);
            bitCount++;
        }
    }
    dataExtraction_msgLoopEnd:
    secretMsg.push_back('\0');
    cout << "Secret message is:\n" << secretMsg.data() << endl;
    
    // Image reconstruction
    imageEncryption(cipherImg, imgKey);
    for (int row = 1; row < cipherImg->rows; row++) {
        for (int col = 1; col < cipherImg->cols; col++) {
            int cur_pixel = (int)(cipherImg->at<uchar>(row, col));
            int pred_pixel = ((int)(cipherImg->at<uchar>(row-1, col)) + (int)(cipherImg->at<uchar>(row, col)-1)) / 2;
            int delta_diff_0 = abs(pred_pixel - (cur_pixel ^ ((0 ^ cur_pixel) & (0x80))));
            int delta_diff_1 = abs(pred_pixel - (cur_pixel ^ ((-1 ^ cur_pixel) & (0x80))));
            if (delta_diff_0 < delta_diff_1) {
                cipherImg->at<uchar>(row, col) =  (cur_pixel ^ ((0 ^ cur_pixel) & (0x80)));
            } else {
                cipherImg->at<uchar>(row, col) =  (cur_pixel ^ ((-1 ^ cur_pixel) & (0x80)));
            }
        }
    }
    return;
}