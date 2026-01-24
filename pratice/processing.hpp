#pragma once
#include"sharedResource.hpp"
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>
#include<zymkey/zkAppUtilsClass.h>
#include<zk_app_utils.h>
#include<zk_b64.h>
#include<sodium.h>
#define USE_KEY_SLOT 0

class ProcessingWorker{
private:
    SharedResourceManager& res;
    zkAppUtils::zkClass zk;
    std::thread process_thread;
    bool stop_thread = false;

public:
    ProcessingWorker(SharedResourceManager& r);
    ~ProcessingWorker();
    void start_worker();
    void process_task();
    void do_process(Long idx);
    zkAppUtils::byteArray compute_hash_sodium(const cv::Mat& frame);
    zkAppUtils::byteArray* sign_with_zymkey(const cv::Mat& frame, int slot);
    void create_new_key(int);
};