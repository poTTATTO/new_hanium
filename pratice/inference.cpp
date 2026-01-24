#include"inference.hpp"

InferenceWorker::InferenceWorker(SharedResourceManager& r) : res(r){}

InferenceWorker::~InferenceWorker(){
    stop_thread = true;
    res.cv_inf.notify_all();
    if(inference_thread.joinable()) inference_thread.join();
}

void InferenceWorker::start_worker(){
    inference_thread = std::thread([this] {inference_task();});
}

void InferenceWorker::inference_task(){
    pthread_setname_np(pthread_self(), "Inference Thread");
}


void InferenceWorker::do_inference(Long idx){
    
}