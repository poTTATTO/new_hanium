#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<fstream>
#include<sys/stat.h> 
#include<iostream>
#include<thread>
#include<atomic>
#include<chrono>
#include<csignal>
#include"sharedResource.hpp"
#include"save.hpp"
#include"capture.hpp"
#include"processing.hpp"
#include"inference.hpp"
#include<sodium.h>
 
void init_dependencies();
std::atomic<bool> g_running{true};

void signal_handler(int signal){
    if(signal == SIGINT){
        std::cout<<"종료 합니다."<<std::endl;
        g_running = false;
    }
}
int main(){
   std::signal(SIGINT, signal_handler);

//    auto start_time = std::chrono::steady_clock::now();

//    while(std::chrono::steady_clock::now() - start_time < std::chrono::seconds(20)){
    std::string hef_path = "/home/cloud9/sangmin/new_hanium/pratice/yolov8s.hef";
    init_dependencies();
    SharedResourceManager res;

    CaptureWorker capturer(res);
    SaveWorker saver(res);
    ProcessingWorker processor(res);
    InferenceWorker inferer(res, hef_path);
    



    

    capturer.start_worker();
    saver.start_worker();
    processor.start_worker();
    inferer.start_worker();

    std::cout<<"종료 하려면 Ctrl + C를 누르세요"<<std::endl;

    while(g_running){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
   

   
    std::cout<<"[MAIN] 정상 종료"<<std::endl;

    return 0;

}



void init_dependencies(){
    if(sodium_init()<0){
        throw std::runtime_error("Sodium init failed");
    }
    curlpp::initialize();
    std::cout<<"library(sodium, curlpp) init complete"<<std::endl;
}

