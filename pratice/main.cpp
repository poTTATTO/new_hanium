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
#include<sodium.h>
#include <curlpp/cURLpp.hpp>  // 전역 초기화용
#include <curlpp/Easy.hpp>     // 실제 전송용 (Easy handle)
#include <curlpp/Options.hpp>  // URL, PostFields 등 옵션 설정용

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

   try{

        void init_dependencies();
        SharedResourceManager res;

        SaveWorker saver(res);
        CaptureWorker capturer(res);
        ProcessingWorker processor(res);

        saver.start_worker();
        capturer.start_worker();
        processor.start_worker();

        std::cout<<"종료 하려면 Ctrl + C를 누르세요"<<std::endl;

        while(g_running){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
   }catch(const std::exception& e){
    std::cerr<<"에러 발생 : "<<e.what()<<std::endl;
    return 1;
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

