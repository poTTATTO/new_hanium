#include"send.hpp"

SendWorker::SendWorker(SharedResourceManager& r, Config& c) : res(r), cfg(c),
server_url(c.getServerUrl()){
  
}

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
    try{
        std::cout<<"[Send] Slot : "<<idx<<std::endl;
        
        std::string json_packet = create_packet(idx, slot);
        if(json_packet.empty()){
          slot.clear();
          return;
        }

        using namespace curlpp::options;
        
        std::string final_url = server_url + "/test/data";
        
        request.setOpt(new Url(final_url));

        std::list<std::string> header;
        header.push_back("Content-Type: application/json");
        request.setOpt(new HttpHeader(header));

        request.setOpt(new PostFields(json_packet));
        request.setOpt(new PostFieldSize(json_packet.length()));

        request.setOpt(new Timeout(5));

        request.perform();

        long response_code = curlpp::Info<CURLINFO_RESPONSE_CODE, long>::get(request);
        std::cout<<"[Send :" <<idx<< "] 전송 성공! HTTP code : "<< response_code<<std::endl;
   }catch (curlpp::RuntimeError& e){
      std::cerr<<"[Send] 런타임 에러 : "<<e.what()<<std::endl;
   }catch (curlpp::LogicError& e){
      std::cerr<<"[Send] 로직 에러 : "<<e.what()<<std::endl;
   }
            
               
  }

  slot.clear();

}

std::string SendWorker::create_packet(Long idx, Slot& slot){
  
  
  std::vector<unsigned char> buf = Util::zipping_image_to_bin(slot.frame);
  
  std::string base64_image = Util::to_base64(buf);
  std::string hex_hash = Util::to_hex(slot.hash);
  std::string base64_sign = Util::to_base64(slot.signature);

  nlohmann::json j;

  j["header"]["id"] = slot.frame_id.load();
  j["header"]["mediaType"] = "image/jpeg";
  j["header"]["timestamp"] = Util::get_current_timestamp();

  j["security"]["hash"] = hex_hash;
  j["security"]["signed_hash"] = base64_sign;

  j["analysis"]["object_detection"] = slot.detection_result;

  j["payload"] = base64_image;

  return j.dump();
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