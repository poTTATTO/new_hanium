#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <condition_variable> 
#include <mutex>              
#include "sharedResource.hpp"
#include "save.hpp"
#include "capture.hpp"
#include "processing.hpp"
#include "inference.hpp"
#include "global_context.hpp"
#include "config.hpp"
#include "client.hpp"

#include "send.hpp" 
volatile std::sig_atomic_t keep_running = 1;
std::mutex mtx_quit;
std::condition_variable cv_quit;


void signal_handler(int signal) {
    keep_running = 0;
    cv_quit.notify_all(); 
    const char* msg = "\n[Signal] SIGINT 수신. 모든 워커를 정지시킵니다...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "시그널 등록 실패!" << std::endl;
        return 1;
    }

    auto& cfg = Config::getConfigInstance();
    auto& gc = GlobalContext::getGlobalContextInstance();

    Client client(cfg, gc);
    if (!client.sendPublicKey()) {
        std::cerr << "[Fatal] 서버 연결 실패." << std::endl;
        return 1;
    }

    {
        SharedResourceManager res;

        CaptureWorker capturer(res, cfg);
        SaveWorker saver(res, cfg);
        ProcessingWorker processor(res, gc);
        InferenceWorker inferer(res, cfg);
        SendWorker sender(res, cfg);

        capturer.start_worker();
        saver.start_worker();
        processor.start_worker();
        inferer.start_worker();
        sender.start_worker();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  시스템 가동 중... (종료: Ctrl + C)" << std::endl;
        std::cout << "  [Tip] 워커 루프에 yield()가 있는지 확인하세요." << std::endl;
        std::cout << "========================================\n" << std::endl;

        
        std::unique_lock<std::mutex> lck(mtx_quit);
        cv_quit.wait(lck, []{ return keep_running == 0; });

        std::cout << "\n[MAIN] 신호 감지. 자원 해제(Join)를 시작합니다." << std::endl;
        
        
    } 

    std::cout << "[MAIN] 모든 워커 종료 완료. 프로그램이 안전하게 닫힙니다." << std::endl;
    return 0;
}