#include"capture.hpp"

CaptureWorker::CaptureWorker(SharedResourceManager& r) : res(r) , cap(0, cv::CAP_V4L2), frame_id(-1){
    if(!cap.isOpened()){
        std::cerr<<"카메라를 찾을 수 없음."<<std::endl;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH,640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
}

void CaptureWorker::start_worker(){
    capture_thread = std::thread([this] {capture_task();});
}

void CaptureWorker::capture_task(){
    pthread_setname_np(pthread_self(),"Capture_Thread");

    while(true){
        
        do_capture();

        if(frame.empty()){
            std::cerr<<"프레임이 비었습니다."<<std::endl;
            continue;
        }
        frame_id++;
        Long idx = frame_id % SIZE;

        if(res.slot_pool[idx].is_occupied.load()){
            std::cerr<<"[Capture] Slot"<<idx<<" is occupied. Dropping frame "<<frame_id<<std::endl;
            continue;
        }
        
        slot_init(idx);
        res.distribute_task(idx);
    }
}

void CaptureWorker::do_capture(){
       cap>>frame;

}

void CaptureWorker::slot_init(Long idx){
        Slot& slot = res.slot_pool[idx];

        slot.is_valid.store(true);
        slot.tasks_left.store(TEST_COUNT);
        slot.frame = std::move(frame);
        slot.frame_id = frame_id;
        slot.is_occupied.store(true);
}