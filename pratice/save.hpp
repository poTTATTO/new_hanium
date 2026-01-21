#pragma once
#include<iostream>
#include<queue>
#include<thread>
#include<atomic>
#include<queue>
#include<mutex>
#include<condition_variable>
#include"sharedResource.hpp"

class SaveWorker{
private:
    SharedResourceManager& res;
    std::thread save_thread;
    bool stop_thread = false;

public:
    SaveWorker(SharedResourceManager& r);
    ~SaveWorker();
    void start_worker();
    void save_task();
    void do_save(Long idx);
    
 };