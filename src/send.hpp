#pragma once
#include"sharedResource.hpp"
#include"config.hpp"
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>
#include <curlpp/cURLpp.hpp>  // 전역 초기화용
#include <curlpp/Easy.hpp>     // 실제 전송용 (Easy handle)
#include <curlpp/Options.hpp>  // URL, PostFields 등 옵션 설정용
#include<string>

class SendWorker{
private:
    SharedResourceManager& res;
    Config& cfg;
    std::thread send_thread;
    bool stop_thread = false;

public:
    SendWorker(SharedResourceManager& r, Config& c);
    ~SendWorker();
    void start_worker();
    void send_task();
    void do_send(Long idx);
};