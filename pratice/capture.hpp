#pragma once
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include"capture.hpp"
#include"array.hpp"
#include"save.hpp"

class Capture{
    public:
    cv::VideoCapture cap;
    cv::Mat frame;
    long long frame_id;

    Capture();
    void capture(Save& save_inst);
};