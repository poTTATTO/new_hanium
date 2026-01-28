#pragma once
#include"sharedResource.hpp"
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>
#include "hailo/hailort.hpp"
#include<vector>
#include<string>
#include<memory>

class InferenceWorker{
private:
    SharedResourceManager& res;
    std::thread inference_thread;
    bool stop_thread = false;

public:
    InferenceWorker(SharedResourceManager& r);
    ~InferenceWorker();
    void start_worker();
    void inference_task();
    void do_inference(Long idx);
};