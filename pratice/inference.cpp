#include"inference.hpp"

const std::vector<std::string> InferenceWorker::COCO_LABELS = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle",
    "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
    "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed",
    "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven",
    "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
};

InferenceWorker::InferenceWorker(SharedResourceManager& r, Config& c) : res(r), cfg(c), hef_path(c.getHefPath()){
    // 1. VDevice 초기화
    auto vdevice_exp = hailort::VDevice::create();
    if(!vdevice_exp) throw std::runtime_error("VDeice 생성 실패");
    vdevice = std::move(vdevice_exp.value());

    // 2. 모델 로드 및 입출력 포맷 설정
    auto infer_model_exp = vdevice->create_infer_model(hef_path);
    if(!infer_model_exp) throw std::runtime_error("모델 로드 실패!");
    infer_model = std::move(infer_model_exp.value());

    infer_model->input()->set_format_type(HAILO_FORMAT_TYPE_UINT8);
    infer_model->output()->set_format_type(HAILO_FORMAT_TYPE_FLOAT32);

    // 3.모델 구성 및 바인이딩 생성
    auto configured_model_exp = infer_model->configure();
    if(!configured_model_exp) throw std::runtime_error("모델 구성 실패");
    configured_model = std::move(configured_model_exp.value());

    auto bindings_exp = configured_model.create_bindings();
    if(!bindings_exp) throw std::runtime_error("바인딩 생성 실패");
    bindings = std::move(bindings_exp.value());

    // 4. 출력 버퍼 할당 및 바이딩
    output_size = infer_model->output()->get_frame_size();
    output_data.resize(output_size / sizeof(float));
    bindings.output()->set_buffer(hailort::MemoryView(output_data.data(), output_size));
}

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
    while(true){
        Long idx;
        {
            std::unique_lock<std::mutex> lock(res.m_inf);
            res.cv_inf.wait(lock, [this] {return !res.inference_q.empty() || stop_thread; });
            if(stop_thread && res.inference_q.empty()) break;
            idx = res.inference_q.front();
            res.inference_q.pop();
        }
        do_inference(idx);
    }
}


void InferenceWorker::do_inference(Long idx){
    Slot& slot = res.slot_pool[idx];

    if(slot.is_valid.load() && !slot.frame.empty()){
        std::cout << "[Inference] Slot : "<<idx<<std::endl;
        cv::Mat tmp = slot.frame;
        
        if(!tmp.isContinuous()) tmp = tmp.clone();
        cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);
        std::vector<Detection> detection;
        bindings.input()->set_buffer(hailort::MemoryView(tmp.data, tmp.total() * tmp.elemSize()));
        
        auto job_exp = configured_model.run_async(bindings);
        if(!job_exp){
            slot.is_valid.store(false);
            return;
        }
        auto job = std::move(job_exp.value());
        job.wait(std::chrono::milliseconds(1000));
        

        detection = parse_to_list();
        

        
    ///*
        if (detection.empty()) {
            std::cout << "탐지된 객체가 없습니다." << std::endl;
        } else {
            std::cout << "--- 탐지 결과 (" << detection.size() << "건) ---" << std::endl;
            
            for (const auto& det : detection) {
                std::cout << "[Class]: " << det.cls 
                        << " [Prob]: " << det.prob 
                        << " [BBox]: [" << det.bbox[0] << ", " << det.bbox[1] << ", " 
                        << det.bbox[2] << ", " << det.bbox[3] << "]" << std::endl;
            }
            std::cout << "---------------------------" << std::endl;
        }
            
        //    */
        slot.detection_result = std::move(convert_to_json_string(detection));

        std::cout<<"["<<slot.frame_id.load()<<"] 추론 성공"<<std::endl;
        
    }else{
        std::cout<<"추론 실패"<<std::endl;
    }
    
    slot.mark_done(res.send_q, res.m_send, res.cv_send);
}



std::vector<Detection> InferenceWorker::parse_to_list(){
    
    std::vector<Detection> current_detections;
    float* results = output_data.data();
    int current_ptr = 0;

    for (int cls = 0; cls < 80; ++cls) {
        int count = static_cast<int>(results[current_ptr++]);
        for (int i = 0; i < count; ++i) {
            float* det = &results[current_ptr];
            float score = det[4];

            if (score >= 0.45f) {
                Detection d;
                d.cls = cls;
                d.prob = score;
                // [x1, y1, x2, y2] 순서로 저장
                d.bbox = { (int)(det[1] * 640), (int)(det[0] * 640), 
                           (int)(det[3] * 640), (int)(det[2] * 640) };
                current_detections.push_back(d);
            }
            current_ptr += 5;
        }
    }
    return current_detections;
}

std::string InferenceWorker::convert_to_json_string(const std::vector<Detection>& detections) {
    // 1. 최상위 객체를 배열(Array) 형식으로 선언 
    nlohmann::json j_array = nlohmann::json::array();

    for (const auto& det : detections) {
        json obj;
        // 클래스 번호를 문자열 라벨로 변환 
        obj["class"] = COCO_LABELS[det.cls]; 
        obj["prob"] = std::round(det.prob * 100) / 100.0; 
        
        // bbox [x1, y1, x2, y2] 순서를 개별 키로 분리 
        obj["xmin"] = det.bbox[0];
        obj["ymin"] = det.bbox[1];
        obj["xmax"] = det.bbox[2];
        obj["ymax"] = det.bbox[3];

        // 배열에 추가 
        j_array.push_back(obj);
    }

    // 실전 전송 시에는 dump()만 써서 한 줄로 
    return j_array.dump();
}