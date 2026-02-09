#pragma once
#include<sodium.h>
#include<curlpp/cURLpp.hpp>  // 전역 초기화용
#include<curlpp/Easy.hpp>     // 실제 전송용 (Easy handle)
#include<curlpp/Options.hpp>  // URL, PostFields 등 옵션 설정용
#include<curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include<vector>
#include<cerrno>
#include"config.hpp"
#include"global_context.hpp"
#include"util.hpp"

class Client{
private:
    Config& cfg;
    GlobalContext& gc;
    std::string buildUrl(const std::string& endpoint);

    
public:
    Client(Config& c, GlobalContext& g);
    bool sendPublicKey();
};