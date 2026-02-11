#include"save.hpp"

SaveWorker::SaveWorker(SharedResourceManager& r, Config& c) : res(r), cfg(c), save_path(cfg.getSavePath()){ }

SaveWorker::~SaveWorker(){
    stop_thread = true;
    res.cv_save.notify_all();
    if(save_thread.joinable()) save_thread.join();
    std::cout<<"Save Thread Destructor"<<std::endl;
}

void SaveWorker::start_worker(){
    save_thread = std::thread([this] { save_task();});
}
void SaveWorker::save_task(){
    pthread_setname_np(pthread_self(), "Save_Thread");
    while(keep_running){
        Long idx;
        {
            std::unique_lock<std::mutex> lock(res.m_save);
            res.cv_save.wait(lock, [this] {return !res.save_q.empty() || stop_thread || !keep_running;});
            if((stop_thread || !keep_running) && res.save_q.empty()) break;
            idx = res.save_q.front();
            res.save_q.pop();
        }

        do_save(idx);
    }

    std::this_thread::yield();
}

void SaveWorker::do_save(Long idx){
    Slot& slot = res.slot_pool[idx];
    if(slot.is_valid){
        std::cout<<"[Save] Processing Slot : "<< idx<<std::endl;

        if(slot.frame.empty()) {
            std::cerr << "[CRITICAL] Slot " << idx << " 의 프레임이 비어 있어 저장을 건너뜁니다! ㅋ" << std::endl;
            slot.mark_done(res.save_q, res.m_save, res.cv_save);
            return;
        }
        std::string save_full_path;
        save_full_path = save_path + std::to_string(slot.frame_id) + ".jpg";
        if(!cv::imwrite(save_full_path, slot.frame)){
            slot.is_valid = false;
            std::cout<<"["<<slot.frame_id<<"]"<<"저장 실패"<<std::endl;
        }else{
            std::cout<<"["<<slot.frame_id<<"]"<< "저장 성공"<<std::endl;
            slot.is_valid.store(true);
        }
    }
    
    slot.mark_done(res.send_q, res.m_send, res.cv_send);

}