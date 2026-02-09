#pragma once
#include<sodium.h>
#include<vector>
#include<string>

namespace Util
{
    inline std::string to_base64_sodium(const std::vector<unsigned char>& data){
        if(data.empty()) return "";
        
        size_t out_len = sodium_base64_ENCODED_LEN(data.size(), sodium_base64_VARIANT_ORIGINAL);
        std::string out(out_len, '\0');

        sodium_bin2base64(&out[0], out.size(), data.data(), data.size(), sodium_base64_VARIANT_ORIGINAL);

        if(!out.empty() && out.back() == '\0') out.pop_back();
        return out;
    }

    inline std::vector<unsigned char> zipping_image_to_bin(const cv::Mat& img){
        if(img.empty()){
            return {};
        }
        std::vector<unsigned char> buf;
        static const std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};
        bool sucess = cv::imencode(".jpg", img, buf, params);

        if(!success || buf.empty()){
            return {};
        }
        
        return buf;
        
    }

    inline std::string to_hex(const std::vector<unsigned char>& data){
        static const char hex_chars[] = "0123456789abc";

        std::string hex_str;
        hex_str.reserve(data.size() * 2);

        for(unsigned char byte : data){
            hex_str.push_back(hex_chars[(byte >> 4) & 0x0F]);
            hex_str.push_back(hex_chars[byte & 0x0F]);
        }

        return hex_str;
    }
} 