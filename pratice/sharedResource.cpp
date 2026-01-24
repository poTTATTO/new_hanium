#include<sharedResource.hpp>

Slot::Slot() : id(-1){}
Slot::Slot(int i) : id(id){}

void Slot::mark_done(std::queue<Long>& send_q, std::mutex& mtx, std::condition_variable& cv){
    if(tasks_left.fetch_sub(1) == 1){
            {
                std::lock_guard<std::mutex> lock(mtx);
                send_q.push(id);
                clear();
            }
            cv.notify_one();
        }
}

void Slot::clear(){
        is_occupied.store(false);
        frame_id = -1;
        is_valid.store(true);
        frame.release();
        tasks_left.store(3);
}


SharedResourceManager::SharedResourceManager(){
        for(int i=0; i<300; i++){
            slot_pool[i].id = i;
        }
    }


void SharedResourceManager::distribute_task(Long idx) {
        {
            std::lock_guard<std::mutex> lock(m_inf);
            inference_q.push(idx);
        }
        cv_inf.notify_one();

        {
            std::lock_guard<std::mutex> lock(m_save);
            save_q.push(idx);
        }
        cv_save.notify_one();

        {
            std::lock_guard<std::mutex> lock(m_proc);
            processing_q.push(idx);
        }
        cv_proc.notify_one();

}

