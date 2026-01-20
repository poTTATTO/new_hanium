#pragma once
#include"sharedResource.hpp"
#include<opencv.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>

class ProcessingWorker{
private:
    SharedResourceManager& res;
    std::thread process_thread;
    bool stop_thread = false;

public:
    ProcessingWorker(SharedResourceManager& r);
    ~ProcessingWorker();
    void start_worker();
    void process_task();
    void do_process();
};