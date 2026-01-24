#pragma once
#include"sharedResource.hpp"
#include<opencv.hpp>
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>
#include<curlpp>


class SendWorker{
private:
    SharedResourceManager& res;
    std::thread send_thread;
    bool stop_thread = false;

public:
    SendWorker(SharedResourceManager& r);
    ~SendWorker();
    void start_worker();
    void send_task();
    void do_send();

};