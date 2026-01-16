#include"capture.hpp"

Capture::Capture() : cap(0, cv::CAP_V4L2), frame_id(-1){
    if(!cap.isOpened()){
        std::cerr<<"카메라를 찾을 수 없음."<<std::endl;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH,640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    
}

void Capture::capture(Save& save_inst){
    while(true){
        cap>>frame;
        frame_id++;

        if(frame.empty()){
            std::cerr<<"프레임이 비었습니다."<<std::endl;
            continue;
        }

        int idx = frame_id % SIZE;
        Array::DataArray[idx].m_cid = std::to_string(frame_id);
        Array::DataArray[idx].original_frame = std::move(frame);

        save_inst.push_to_save_cid_queue(idx);
    }
}