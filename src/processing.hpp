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
#include"global_context.hpp"
#include"util.hpp"
#define USE_KEY_SLOT 0

class ProcessingWorker{
private:
    SharedResourceManager& res;
    GlobalContext& gc;
    // zkAppUtils::zkClass zk;
    std::thread process_thread;
    bool stop_thread = false; 
    std::vector<unsigned char> public_key;
    std::vector<unsigned char> private_key;

public:
    ProcessingWorker(SharedResourceManager& r, GlobalContext& gc);
    ~ProcessingWorker();
    void start_worker();
    void process_task();
    void do_process(Long idx);
    int sign_keypair(unsigned char* public_key,unsigned char* private_key);
    std::vector<unsigned char> hash_frame_libsodium(const cv::Mat& frame);
    std::vector<unsigned char> sign_frame_libsodium(const std::vector<unsigned char>& hash, const unsigned char* private_key); 
    // zkAppUtils::byteArray compute_hash_sodium(const cv::Mat& frame);
    // zkAppUtils::byteArray* sign_with_zymkey(const cv::Mat& frame, int slot);
    // void create_new_key(int);
};