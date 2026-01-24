#include "hailo/hailort.hpp"
#include "hailo/hef.hpp"
#include "hailo/vstream.hpp"
#include <opencv2/opencv.hpp>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>

using namespace hailort;

const std::vector<std::string> COCO_CLASSES = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle",
    "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple", "sandwich", "orange",
    "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch", "potted plant", "bed",
    "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven",
    "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush"
};

// 참조(&)를 사용하여 복사를 방지함


// HAILO NMS 전용 파싱 함수
std::string perform_inference_and_draw(InputVStream& input, OutputVStream& output, cv::Mat& frame, const std::string& original_path) {
    std::vector<uint8_t> out_buffer(output.get_frame_size());
    input.write(MemoryView(frame.data, input.get_frame_size()));
    output.read(MemoryView(out_buffer.data(), out_buffer.size()));

    float* results = reinterpret_cast<float*>(out_buffer.data());
    Json::Value root(Json::arrayValue);
    cv::Mat canvas = frame.clone();

    // [수정된 파싱 로직]
    int current_idx = 0;
    // 1. 첫 번째 값은 무조건 전체 탐지 개수
    int total_detections = static_cast<int>(results[current_idx++]); 

    // 2. 클래스별 탐지 개수를 먼저 '배열'로 다 읽어둡니다.
    // Hailo NMS 구조상 개수 정보 80개가 먼저 쭉 나오고 그 뒤에 박스들이 몰려 나옵니다.
    std::vector<int> counts(80);
    for (int i = 0; i < 80; ++i) {
        counts[i] = static_cast<int>(results[current_idx++]);
    }

    // 3. 이제 각 클래스별 박스 데이터를 순서대로 읽습니다.
    for (int class_id = 0; class_id < 80; ++class_id) {
        for (int i = 0; i < counts[class_id]; ++i) {
            float ymin = results[current_idx++];
            float xmin = results[current_idx++];
            float ymax = results[current_idx++];
            float xmax = results[current_idx++];
            float confidence = results[current_idx++];

            if (confidence > 0.5f) {
                int px_xmin = static_cast<int>(xmin * 640);
                int px_ymin = static_cast<int>(ymin * 640);
                int px_xmax = static_cast<int>(xmax * 640);
                int px_ymax = static_cast<int>(ymax * 640);

                std::string label = COCO_CLASSES[class_id];

                Json::Value obj;
                obj["class"] = label;
                obj["prob"] = std::round(confidence * 100.0) / 100.0;
                obj["xmin"] = px_xmin; obj["ymin"] = px_ymin;
                obj["xmax"] = px_xmax; obj["ymax"] = px_ymax;
                root.append(obj);

                cv::rectangle(canvas, cv::Point(px_xmin, px_ymin), cv::Point(px_xmax, px_ymax), cv::Scalar(0, 255, 0), 2);
                cv::putText(canvas, label, cv::Point(px_xmin, px_ymin - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
            }
        }
    }

    // 파일 저장
    std::string save_path = original_path.substr(0, original_path.find_last_of('.')) + "+boun.jpg";
    cv::imwrite(save_path, canvas);

    Json::StreamWriterBuilder writer;
    writer["indentation"] = "  ";
    return Json::writeString(writer, root);
}
int main() {
    auto hef_exp = Hef::create("yolov8s_h8.hef"); 
    if (!hef_exp) return 1;
    auto hef = std::move(hef_exp.value());

    auto vdevice_exp = VDevice::create();
    if (!vdevice_exp) return 1;
    auto vdevice = std::move(vdevice_exp.value());

    auto configure_params = hef.create_configure_params(HAILO_STREAM_INTERFACE_PCIE);
    auto network_groups = vdevice->configure(hef, configure_params.value());
    auto& net_group = network_groups.value()[0];

    auto in_params = net_group->make_input_vstream_params(false, HAILO_FORMAT_TYPE_UINT8, 1000, 10, "").value();
    auto out_params = net_group->make_output_vstream_params(false, HAILO_FORMAT_TYPE_FLOAT32, 1000, 10, "").value();

    // [중요] 에러가 났던 101~102행 수정: 명시적으로 std::move 적용
    auto input_vstreams_exp = VStreamsBuilder::create_input_vstreams(*net_group, in_params);
    if (!input_vstreams_exp) return 1;
    std::vector<InputVStream> input_vstreams = std::move(input_vstreams_exp.value());

    auto output_vstreams_exp = VStreamsBuilder::create_output_vstreams(*net_group, out_params);
    if (!output_vstreams_exp) return 1;
    std::vector<OutputVStream> output_vstreams = std::move(output_vstreams_exp.value());

    std::string img_name = "test_image_coco.jpg";
    cv::Mat img = cv::imread(img_name);
    if (img.empty()) return 1;
    
    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(640, 640));

    // 참조로 전달하므로 복사 에러가 발생하지 않음
    std::string json_response = perform_inference_and_draw(input_vstreams[0], output_vstreams[0], resized_img, img_name);

    std::cout << json_response << std::endl;

    return 0;
}