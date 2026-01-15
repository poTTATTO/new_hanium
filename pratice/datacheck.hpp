#pragma once
#include<iostream>
#include<opencv2/opencv.hpp>

typedef struct DataCheck{
    std::string CID = "None";
    cv::Mat data = cv::Mat();

}DataCheck;