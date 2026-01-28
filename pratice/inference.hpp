#pragma once
#include"sharedResource.hpp"
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<thread>
#include<iostream>
#include "hailo/hailort.hpp"
#include<vector>
#include<string>
#include<memory>



class InferenceWorker{
private:
    SharedResourceManager& res;
    std::thread inference_thread;
    bool stop_thread = false;
    std::unique_ptr<hailort::VDevice> vdevice;
    std::shared_ptr<hailort::InferModel> infer_model;
    hailort::ConfiguredInferModel configured_model;
    hailort::ConfiguredInferModel::Bindings bindings;
    std::vector<float> output_data;
    size_t output_size;
    std::string hef_path;

    //COCO 라벨 데이터
    static const std::vector<std::string> COCO_LABELS;
public:
    explicit InferenceWorker(SharedResourceManager& r, const std::string hef_path);
    ~InferenceWorker();
    InferenceWorker(const InferenceWorker& i) = delete;
    InferenceWorker& operator = (const InferenceWorker&) = delete;

    void start_worker();
    void inference_task();
    void do_inference(Long idx);
    std::vector<Detection> parse_to_list();
};