#include "hailo/hailort.hpp"
#include "hailo/hef.hpp"
#include "hailo/vstream.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <numeric>
#include <vector>

using namespace hailort;

int main() {
    // [확인 1] 클래스명 Hef, 함수명 create
    auto hef_exp = Hef::create("yolov8s_h8l.hef");
    if (!hef_exp) return 1;
    auto hef = std::move(hef_exp.value());

    auto vdevice_exp = VDevice::create();
    if (!vdevice_exp) return 1;
    auto vdevice = std::move(vdevice_exp.value());

    auto configure_params = hef.create_configure_params(HAILO_STREAM_INTERFACE_PCIE);
    auto network_groups = vdevice->configure(hef, configure_params.value());
    auto& net_group = network_groups.value()[0];

    // [확인 2] 스케줄러 환경이므로 수동 activate()는 생략합니다.

    // [확인 3] 인자 5개 필수, 출력은 반드시 FLOAT32
    auto in_params_exp = net_group->make_input_vstream_params(false, HAILO_FORMAT_TYPE_UINT8, 1000, 10, "");
    auto out_params_exp = net_group->make_output_vstream_params(false, HAILO_FORMAT_TYPE_FLOAT32, 1000, 10, "");

    // [확인 4] 복사 생성자가 없으므로 std::move 필수
    auto in_v_exp = VStreamsBuilder::create_input_vstreams(*net_group, in_params_exp.value());
    auto out_v_exp = VStreamsBuilder::create_output_vstreams(*net_group, out_params_exp.value());
    
    auto input_vstreams = std::move(in_v_exp.value());
    auto output_vstreams = std::move(out_v_exp.value());

    // 테스트 이미지 로드
    cv::Mat img = cv::imread("test_image_coco.jpg");
    if (img.empty()) return 1;
    cv::resize(img, img, cv::Size(640, 640));

    int iterations = 100;
    std::vector<uint8_t> out_buffer(output_vstreams[0].get_frame_size());
    std::vector<double> latencies;

    std::cout << iterations << "회 성능 측정 루프 시작..." << std::endl;
    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // 추론 실행
        input_vstreams[0].write(MemoryView(img.data, input_vstreams[0].get_frame_size()));
        output_vstreams[0].read(MemoryView(out_buffer.data(), out_buffer.size()));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = end - start;
        latencies.push_back(ms.count());
    }

    double avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / iterations;
    std::cout << "\n[결과] 평균 지연 시간: " << avg << " ms | FPS: " << 1000.0 / avg << std::endl;

    return 0;
}