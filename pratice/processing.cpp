#include<processing.hpp>

ProcessingWorker::ProcessingWorker(SharedResourceManager& r) : res(r){}

ProcessingWorker::~ProcessingWorker(){
    stop_thread = true;
    res.cv_proc.notify_all();
    if(save_thread.joinable()) process_thread.join();
}

void ProcessingWorker::start_worker(){
    process_thread = std::thread([this] {save_task();});
}

void ProcessingWorker::process_task(){

}

void ProcessingWorker::do_process(){

}
