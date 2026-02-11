#pragma once
#include<iostream>
#include<queue>
#include<thread>
#include<atomic>
#include<queue>
#include<mutex>
#include<condition_variable>
#include"sharedResource.hpp"
#include"config.hpp"
#include<nlohmann/json.hpp>
#include <csignal>
extern volatile std::sig_atomic_t keep_running; 

class SaveWorker{
private:
    SharedResourceManager& res;
    Config& cfg;
    std::thread save_thread;
    bool stop_thread = false;
    std::string save_path;

public:
    SaveWorker(SharedResourceManager& r, Config& c);
    ~SaveWorker();
    void start_worker();
    void save_task();
    void do_save(Long idx);
    
 };