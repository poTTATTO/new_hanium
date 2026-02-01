#pragma once
#include<iostream>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<vector>
#include<zymkey/zkAppUtilsClass.h>
#include<zk_app_utils.h>
#include<zk_b64.h>
#include<new>
#define SLOT_POOL_SIZE 1500
typedef long long Long;

struct Detection{
    int cls;
    float prob;
    std::vector<int> bbox;
};

class Slot{
public:
    int id;
    std::atomic<long> frame_id = -1;
    std::atomic<int> tasks_left{0};
    
    alignas(std::hardware_destructive_interference_size) std::atomic<bool> is_occupied{false};
    alignas(std::hardware_destructive_interference_size) std::atomic<bool> is_valid{true};
    cv::Mat frame;
    std::vector<unsigned char> signature;
    std::string detection_result;
    // zkAppUtils::byteArray signature;
    

    Slot();
    Slot(int i);

    void mark_done(std::queue<Long>& send_q, std::mutex& mtx, std::condition_variable& cv);
    void clear();
};

class SharedResourceManager {
public:    
    Slot slot_pool[SLOT_POOL_SIZE];
    // 각 작업자용 큐와 동기화 도구들
    std::queue<Long> inference_q, save_q, processing_q, send_q;
    std::mutex m_inf, m_save, m_proc, m_send;
    std::condition_variable cv_inf, cv_save, cv_proc, cv_send;

    // Capture 스레드가 호출: 세 개의 큐에 인덱스를 동시에 배분
    SharedResourceManager();
    void distribute_task_to_save(Long idx);
    void distribute_task_to_proc(Long idx);
    void distribute_task_to_inf(Long idx);

};