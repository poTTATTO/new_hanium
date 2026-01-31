#include"capture.hpp"

CaptureWorker::CaptureWorker(SharedResourceManager& r, Config& c) : res(r), cfg(c), cap(0, cv::CAP_V4L2), frame_id(-1){
    if(!cap.isOpened()){
        std::cerr<<"카메라를 찾을 수 없음."<<std::endl;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, cfg.getWidth());
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.getHeight());
    cap.set(cv::CAP_PROP_FPS, cfg.getFps()); 
}

void CaptureWorker::start_worker(){
    capture_thread = std::thread([this] {capture_task();});
}

void CaptureWorker::capture_task(){
    pthread_setname_np(pthread_self(),"Capture_Thread");

    while(true){
        cv::Mat frame;
        cap >> frame;

        if(frame.empty()){
            std::cerr<<"프레임이 비었습니다."<<std::endl;
            continue;
        }
        frame_id++;
        Long idx = frame_id % SLOT_POOL_SIZE;

        if(res.slot_pool[idx].is_occupied.load()){
            std::cerr<<"[Capture] Slot"<<idx<<" is occupied. Dropping frame "<<frame_id<<std::endl;
            continue;
        }
        
        slot_init(idx, frame);
        res.distribute_task_to_save(idx);
        res.distribute_task_to_proc(idx);
        res.distribute_task_to_inf(idx);
    }
}

void CaptureWorker::do_capture(){
}

void CaptureWorker::slot_init(Long idx, cv::Mat& frame){
        Slot& slot = res.slot_pool[idx];
        slot.frame_id.store(frame_id);
        slot.tasks_left.store(TEST_COUNT);
        if(!frame.isContinuous()){
            frame = frame.clone();
        }
        if(frame.cols != 640 || frame.rows != 640){
            cv::Mat canvas = cv::Mat::zeros(640, 640, frame.type());
            frame.copyTo(canvas(cv::Rect(0, 0, frame.cols, std::min(frame.rows, 640))));
            slot.frame = std::move(canvas);
        }else{
            slot.frame = std::move(frame);
        }
        
        slot.is_valid.store(true);
        slot.is_occupied.store(true);
}