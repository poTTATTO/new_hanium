#include "hailo/hailort.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include<nlohmann/json.hpp>

using json = nlohmann::json;
struct Detection{
    int cls;
    float prob;
    std::vector<int>bbox;
};
std::string convert_to_json_string(const std::vector<Detection>& detection);

const std::vector<std::string> COCO_LABELS = { "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush" };

int main() {
    // 1. Expected 객체를 변수로 받고 명시적으로 move 수행
    auto vdevice_exp = hailort::VDevice::create();
    if (!vdevice_exp) return -1;
    auto vdevice = std::move(vdevice_exp.value());

    auto infer_model_exp = vdevice->create_infer_model("/home/cloud9/sangmin/new_hanium/testcode/yolov8s.hef");
    if (!infer_model_exp) return -1;
    auto infer_model = std::move(infer_model_exp.value());

    infer_model->input()->set_format_type(HAILO_FORMAT_TYPE_UINT8);
    infer_model->output()->set_format_type(HAILO_FORMAT_TYPE_FLOAT32);

    auto configured_model_exp = infer_model->configure();
    auto configured_model = std::move(configured_model_exp.value());

    auto bindings_exp = configured_model.create_bindings();
    auto bindings = std::move(bindings_exp.value());

    // 2. 이미지 로드 및 전처리
    cv::Mat original_img = cv::imread("/home/cloud9/sangmin/new_hanium/testcode/test_image_coco.jpg");
    if (original_img.empty()) return -1;

    cv::Mat resized_img;
    cv::resize(original_img, resized_img, cv::Size(640, 640));
    cv::cvtColor(resized_img, resized_img, cv::COLOR_BGR2RGB);

    // 3. 버퍼 설정
    bindings.input()->set_buffer(hailort::MemoryView(resized_img.data, resized_img.total() * resized_img.elemSize()));

    size_t output_size = infer_model->output()->get_frame_size();
    std::vector<float> output_data(output_size / sizeof(float));
    bindings.output()->set_buffer(hailort::MemoryView(output_data.data(), output_size));

    // 4. 추론 실행 (Job 소유권 명시적 이동)
    auto job_exp = configured_model.run_async(bindings);
    if (!job_exp) return -1;
    hailort::AsyncInferJob job = std::move(job_exp.value()); 
    job.wait(std::chrono::milliseconds(2000));

    // 5. 스캐닝 파싱 (Stride 501 의존하지 않고 전수 조사)
    
// ... (헤더 및 라벨 정의 생략) ...

// 5. [교정] 5-Stride 기반 전수 조사 파싱
// 5. [최종 확정] 순차적 클래스 파싱 로직

// float* results = output_data.data();
// int current_ptr = 0; // 버퍼를 읽어 나갈 포인터

// std::cout << "\n--- [검출 성공] 올바른 객체 ID 매칭 시작 ---" << std::endl;

// for (int cls = 0; cls < 80; ++cls) {
//     // 1. 현재 위치의 값은 해당 클래스의 '검출 개수'입니다.
//     int count = static_cast<int>(results[current_ptr]);
//     current_ptr++; // 개수 다음 칸으로 이동

//     for (int i = 0; i < count; ++i) {
//         // 2. 검출 개수가 있을 때만 5개의 데이터(y1, x1, y2, x2, score)를 읽습니다.
//         float y1_f = results[current_ptr + 0];
//         float x1_f = results[current_ptr + 1];
//         float y2_f = results[current_ptr + 2];
//         float x2_f = results[current_ptr + 3];
//         float score = results[current_ptr + 4];

//         if (score >= 0.4f) {
//             std::string label = COCO_LABELS[cls]; // 현재 루프의 cls가 진짜 ID입니다!

//             int x1 = std::clamp((int)(x1_f * original_img.cols), 0, original_img.cols);
//             int y1 = std::clamp((int)(y1_f * original_img.rows), 0, original_img.rows);
//             int x2 = std::clamp((int)(x2_f * original_img.cols), 0, original_img.cols);
//             int y2 = std::clamp((int)(y2_f * original_img.rows), 0, original_img.rows);

//             std::cout << "[" << label << "] " << (int)(score * 100) << "% 발견!" << std::endl;

//             // 시각화
//             cv::rectangle(original_img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
//             cv::putText(original_img, label, cv::Point(x1, y1 - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
//         }
//         current_ptr += 5; // 다음 객체로 이동
//     }
    
//     // 검출 개수가 0인 클래스라도 포인터는 이미 current_ptr++로 인해 다음 클래스 개수 칸을 가리킵니다.
// }
// cv::imwrite("real_final_result.jpg", original_img);
    
    std::vector<Detection> detections;
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
                detections.push_back(d);
            }
            current_ptr += 5;
        }
    }

    if (detections.empty()) {
        std::cout << "탐지된 객체가 없습니다. ㅋ" << std::endl;
    } else {
        std::cout << "--- 탐지 결과 (" << detections.size() << "건) ---" << std::endl;
        
        for (const auto& det : detections) {
            std::cout << "[Class]: " << det.cls 
                    << " [Prob]: " << det.prob 
                    << " [BBox]: [" << det.bbox[0] << ", " << det.bbox[1] << ", " 
                    << det.bbox[2] << ", " << det.bbox[3] << "]" << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    std::cout<<convert_to_json_string(detections)<<std::endl;

    return 0;
}


std::string convert_to_json_string(const std::vector<Detection>& detections) {
    json root;

    // 1. 데이터 개수 삽입
    root["number of data"] = detections.size();

    // 2. 각 탐지 결과를 인덱스 번호를 키(Key)로 하여 삽입
    for (size_t i = 0; i < detections.size(); ++i) {
        json obj;
        obj["cls"] = detections[i].cls;
        obj["prob"] = detections[i].prob;
        obj["bbox"] = detections[i].bbox; // std::vector는 자동으로 JSON 배열로 변환됨

        // 인덱스를 문자열 키로 사용하여 추가
        root[std::to_string(i)] = obj;
    }

    // 3. JSON 객체를 최종 전송용 문자열로 변환 (직렬화)
    // dump()의 인자로 숫자를 주면 들여쓰기(Pretty-print)가 적용됩니다. 
    // 전송 시에는 인자 없이 dump()만 써서 크기를 최소화하세요.
    return root.dump(); 
}