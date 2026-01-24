#pragma once
#include"sharedResource.hpp"
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>
#include <curlpp/cURLpp.hpp>  // 전역 초기화용
#include <curlpp/Easy.hpp>     // 실제 전송용 (Easy handle)
#include <curlpp/Options.hpp>  // URL, PostFields 등 옵션 설정용


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