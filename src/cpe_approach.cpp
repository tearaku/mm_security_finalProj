#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include "cpe_approach.h"
#include "chaoticGenerator.h"
#include "inspect.h"
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

void cpe_approach(Mat* srcImg, string secretMsg, int32_t msgKey, struct imageSecretKey imgKey, int cipherOutID) {
    cout << "***Preprocessing...\n";
    preProcess(srcImg);
    cout << "***Encrypting image...\n";
    imageEncryption(srcImg, imgKey);
    outputMatHistogram(srcImg, "histo_preEmb" + to_string(cipherOutID) + ".png");
    imwrite("cipherImg" +  to_string(cipherOutID) + ".png", *srcImg);
    cout << "***Embedding data into cipher text...\n";
    string toHide = secretMsg;
    toHide.push_back(static_cast<char>(MSG_CONTROL_CHAR));
    dataEmbedding(srcImg, &toHide, msgKey);
    outputMatHistogram(srcImg, "histo_postEmb" + to_string(cipherOutID) + ".png");
    imwrite("markedCipherImg" +  to_string(cipherOutID) + ".png", *srcImg);
    cout << "***Extracing message and reconstructing image...\n";
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

void imageEncryption(Mat* srcImg, struct imageSecretKey imgKey) {
    CPRBG generator(imgKey.key1, imgKey.key2, imgKey.delta);
    vector<uint8_t> pseudoBytes_vec(srcImg->cols * srcImg->rows);
    for (auto it = pseudoBytes_vec.begin(); it != pseudoBytes_vec.end(); it++) {
        *it = generator.getNextByte();
    }
    Mat pseudoBytes(srcImg->rows, srcImg->cols, CV_8U, pseudoBytes_vec.data());
    bitwise_xor(*srcImg, pseudoBytes, *srcImg);
    return;
}

void dataEmbedding(Mat* srcImg, string* msg, int32_t msgKey) {
    // Encrypt message
    minstd_rand0 generator (msgKey);
    MsgBitStream msgStream(&generator, msg);
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

void dataExtraction(Mat* cipherImg, struct imageSecretKey imgKey, int32_t msgKey) {
    // Message extraction & decryption
    minstd_rand0 generator_msg (msgKey);
    vector<unsigned char> secretMsg;
    unsigned int bitCount = 0;
    unsigned char curByte = 0;
    for (int row = 1; row < cipherImg->rows; row++) {
        for (int col = 1; col < cipherImg->cols; col++) {
            if (bitCount == 8) {
                secretMsg.push_back(curByte ^ generator_msg());
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