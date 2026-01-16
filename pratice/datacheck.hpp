#pragma once
#include<iostream>
#include<opencv2/opencv.hpp>
#include<atomic>

struct DataCheck {
    std::atomic<int> completion_count;
    std::atomic<bool> is_occupied;
    int cid_num;

    std::string m_cid;
    std::string m_hash;
    std::string m_sign_hash;
    std::string m_OD_result;
    cv::Mat original_frame;

    DataCheck() : cid_num(-1) {
        completion_count.store(0);
        is_occupied.store(false);
    }

    void clear() {
        completion_count.store(0);
        is_occupied.store(false);
        cid_num = -1;
        m_cid.clear();
        m_hash.clear();
        m_sign_hash.clear();
        m_OD_result.clear();
        original_frame.release();
    }
};