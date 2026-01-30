#include"send.hpp"

SendWorker::SendWorker(SharedResourceManager& r) : res(r){}

SendWorker::~SendWorker(){
    stop_thread = true;
    res.cv_send.notify_all();
    if(send_thread.joinable()) send_thread.join();
}


void SendWorker::start_worker(){
    send_thread = std::thread([this] {send_task();});

}

void SendWorker::send_task(){
    pthread_setname_np(pthread_self(), "Send Thread");
    while(true){
        Long idx;
        {
            std::unique_lock<std::mutex> lock(res.m_send);
            res.cv_send.wait(lock, [this] {return !res.send_q.empty();});
            if(stop_thread && res.send_q.empty()) break;
            idx = res.send_q.front();
            res.send_q.pop();
        }
        do_send(idx);
    }

}

void SendWorker::do_send(Long idx){
    
}

