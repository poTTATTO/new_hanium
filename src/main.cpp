#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <unistd.h>
#include <cstring>

#include "sharedResource.hpp"
#include "save.hpp"
#include "capture.hpp"
#include "processing.hpp"
#include "inference.hpp"
#include "global_context.hpp"
#include "config.hpp"
#include "client.hpp"
#include "send.hpp"

// 1. 시그널 플래그: volatile sig_atomic_t가 가장 안전합니다.
volatile std::sig_atomic_t keep_running = 1;

// 2. 시그널 핸들러: printf 대신 로우 레벨 write 사용 (안전성 확보)
void signal_handler(int signal) {
    keep_running = 0;
    const char* msg = "\n[Signal] 종료 신호(SIGINT) 수신. 정리를 시작합니다...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    // 3. sigaction 설정: 시그널이 시스템 콜(pause)을 즉시 깨우도록 함
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // SA_RESTART를 명시적으로 제거하여 즉각 반응 유도

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        std::cerr << "시그널 등록 실패!" << std::endl;
        return 1;
    }

    // 4. 싱글톤 자원 초기화
    auto& cfg = Config::getConfigInstance();
    auto& gc = GlobalContext::getGlobalContextInstance();

    // 5. 서버 접속 테스트 (공개키 전송)
    Client client(cfg, gc);
    if (!client.sendPublicKey()) {
        std::cerr << "[Fatal] 서버와 연결할 수 없습니다. 시스템을 종료합니다." << std::endl;
        return 1;
    }

    { // <--- [RAII Scope] 이 중괄호를 벗어날 때 객체들이 역순으로 파괴됩니다.
        SharedResourceManager res;

        // 워커 생성 (선언 순서: res -> 워커들)
        // 파괴 순서: sender -> inferer -> processor -> saver -> capturer -> res
        CaptureWorker capturer(res, cfg);
        SaveWorker saver(res, cfg);
        ProcessingWorker processor(res, gc);
        InferenceWorker inferer(res, cfg);
        SendWorker sender(res, cfg);

        // 스레드 가동
        capturer.start_worker();
        saver.start_worker();
        processor.start_worker();
        inferer.start_worker();
        sender.start_worker();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  시스템 가동 중... (종료: Ctrl + C)" << std::endl;
        std::cout << "========================================\n" << std::endl;

        // 6. 메인 루프: pause()가 깨어나면 keep_running을 단호하게 체크
        while(keep_running) {
            pause(); 
        }

        std::cout << "[MAIN] 루프 종료. 워커 스레드 Join 절차 진입..." << std::endl;
    } // <--- 여기서 모든 워커의 Destructor가 호출되며 join()이 일어남

    std::cout << "\n[MAIN] 모든 자원이 성공적으로 해제되었습니다." << std::endl;
    std::cout << "[MAIN] 프로그램을 안전하게 종료합니다." << std::endl;

    return 0;
}