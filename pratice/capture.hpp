#pragma once
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include"capture.hpp"
#include"sharedResource.hpp"
#include<thread>
#include<cerrno>
#include<atomic>

#define TEST_COUNT 3

class CaptureWorker{
private:    
    SharedResourceManager& res;
    std::thread capture_thread;
    bool stop_thread = false;

public:
    cv::VideoCapture cap;
    std::atomic<Long> frame_id;

    CaptureWorker(SharedResourceManager& r);
    // ~CaptureWorker();
    void start_worker();
    void capture_task();
    void do_capture();
    void slot_init(Long idx, cv::Mat& frame);
};