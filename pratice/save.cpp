#include"save.hpp"

SaveWorker::SaveWorker(SharedResourceManager& r) : res(r){ }

SaveWorker::~SaveWorker(){
    stop_thread = true;
    res.cv_save.notify_all();
    if(save_thread.joinable()) save_thread.join();
}

void SaveWorker::start_worker(){
    save_thread = std::thread([this] { save_task();});
}
void SaveWorker::save_task(){
    pthread_setname_np(pthread_self(), "Save_Thread");
    while(true){
        Long idx;
        {
            std::unique_lock<std::mutex> lock(res.m_save);
            res.cv_save.wait(lock, [this] {return !res.save_q.empty() || stop_thread;});
            if(stop_thread && res.save_q.empty()) break;
            idx = res.save_q.front();
            res.save_q.pop();
        }

        do_save(idx);
    }
}

void SaveWorker::do_save(Long idx){
    Slot& slot = res.slot_pool[idx];
    if(slot.is_valid){
        std::cout<<"[Save] Processing Slot : "<< idx<<std::endl;
        std::string save_path = "/home/cloud9/sangmin/new_hanium/photo/ID_" + std::to_string(slot.frame_id) + ".jpg";
        if(!cv::imwrite(save_path, slot.frame)){
            slot.is_valid = false;
            std::cout<<"["<<slot.frame_id<<"]"<<"저장 실패"<<std::endl;
        }else{
            std::cout<<"["<<slot.frame_id<<"]"<< "저장 성공"<<std::endl;
        }
    }
    
    slot.mark_done(res.send_q, res.m_send, res.cv_send);

}