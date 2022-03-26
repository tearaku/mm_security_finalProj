#ifndef CPE_APPROACH_H
#define CPE_APPROACH_H

#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <utility>

void cpe_approach(cv::Mat* srcImg, std::string secretMsg, int32_t msgKey, struct imageSecretKey imgKey);
 
#endif