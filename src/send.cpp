#include"send.hpp"

SendWorker::SendWorker(SharedResourceManager& r, Config& c) : res(r), cfg(c){}

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
   Slot& slot = res.slot_pool[idx];
   
   if(slot.is_valid.load() && slot.is_occupied.load()){
        std::cout<<"[Send] Slot : "<<idx<<std::endl;
        cv::Mat tmp = slot.frame;
        
        if(!tmp.isContinuous()) tmp = tmp.clone();
        
        std::vector<unsigned char> buf = Util::zipping_image_to_bin(slot.frame);
        
        std::string base64_image = to_base64(buf);
        std::string base64_hash = Util::to_base64_sodium(slot.hash);
        std::string base64_sign = Util::to_base64_sodium(slot.signature);
        



   }else{
        slot.clear();
   }
}

/*
{
  "header": {
    "id": "device_001",
    "mediaType": "image/jpeg",
    "timestamp": 1707462000
  },
  "security": {
    "hash": "8fce...원본해시", // base64로 인코딩됨
    "signed_hash": "a1b2...서명값" //base 64로 인코딩됨
  },
  "analysis": {
    "object_detection": [
      { "label": "person", "confidence": 0.98, "box": [10, 20, 100, 200] },
      { "label": "car", "confidence": 0.85, "box": [150, 50, 300, 400] }
    ]
  },
  "payload": "iVBORw0KGgoAAAANSUhEUgAA..." //base64로 인코딩됨
}
*/