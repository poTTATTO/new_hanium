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
#include"global_context.hpp"
#include"config.hpp"
#include"client.hpp"

std::atomic<bool> g_running{true};

void signal_handler(int signal){
    if(signal == SIGINT){
        std::cout<<"종료 합니다."<<std::endl;
        g_running = false;
    }
}

int main(){
   std::signal(SIGINT, signal_handler);

    auto& cfg = Config::getConfigInstance();
    auto& gc = GlobalContext::getGlobalContextInstance();

    Client client(cfg, gc);
    client.sendPublicKey();

    SharedResourceManager res;

    CaptureWorker capturer(res, cfg);
    SaveWorker saver(res, cfg);
    ProcessingWorker processor(res);
    InferenceWorker inferer(res, cfg);
    



    

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



