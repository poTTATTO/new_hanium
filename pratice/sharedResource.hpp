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

typedef long long Long;

class Slot{
public:
    int id;
    Long frame_id = -1;
    std::atomic<int> tasks_left{0};
    std::atomic<bool> is_occupied{false};
    std::atomic<bool> is_valid{true};
    cv::Mat frame;
    zkAppUtils::byteArray signature;
    

    Slot() : id(-1){}
    Slot(int i) : id(i){}

    void mark_done(std::queue<Long>& send_q, std::mutex& mtx, std::condition_variable& cv){
        if(tasks_left.fetch_sub(1) == 1){
            {
                std::lock_guard<std::mutex> lock(mtx);
                send_q.push(id);
                clear();
            }
            cv.notify_one();
        }
    }

    void clear(){
        is_occupied.store(false);
        frame_id = -1;
        is_valid.store(true);
        frame.release();
        tasks_left.store(3);
    }
};

class SharedResourceManager {
public:    
    Slot slot_pool[300];
    // 각 작업자용 큐와 동기화 도구들
    std::queue<Long> inference_q, save_q, processing_q, send_q;
    std::mutex m_inf, m_save, m_proc, m_send;
    std::condition_variable cv_inf, cv_save, cv_proc, cv_send;

    // Capture 스레드가 호출: 세 개의 큐에 인덱스를 동시에 배분
    SharedResourceManager(){
        for(int i=0; i<300; i++){
            slot_pool[i].id = i;
        }
    }


    void distribute_task(Long idx) {
        {
            std::lock_guard<std::mutex> lock(m_inf);
            inference_q.push(idx);
        }
        cv_inf.notify_one();

        {
            std::lock_guard<std::mutex> lock(m_save);
            save_q.push(idx);
        }
        cv_save.notify_one();

        {
            std::lock_guard<std::mutex> lock(m_proc);
            processing_q.push(idx);
        }
        cv_proc.notify_one();
    }
};