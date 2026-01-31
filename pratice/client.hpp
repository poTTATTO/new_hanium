#pragma once
#include<sodium.h>
#include <curlpp/cURLpp.hpp>  // 전역 초기화용
#include <curlpp/Easy.hpp>     // 실제 전송용 (Easy handle)
#include <curlpp/Options.hpp>  // URL, PostFields 등 옵션 설정용
#include<vector>
#include<cerrno>

class Client{

public:
    Client()
}