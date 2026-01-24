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

}

void SendWorker::do_send(){
    
}