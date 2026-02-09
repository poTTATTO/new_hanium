#include"client.hpp"



Client::Client(Config& c, GlobalContext& g) : 
    cfg(c), gc(g), 
    server_ip(cfg.getServerIp()), server_port(cfg.getServerPort()){

    }

std::string Client::buildUrl(const std::string& endpoint){
    return "http://" + server_ip + ":" + std::to_string(server_port) + endpoint;
}

bool Client::sendPublicKey(){
    try{

        const auto& pub_key_vec = gc.getPublicKey();
        std::string key_str = to_base64(pub_key_vec);

        // 1. Easy 핸들 생성
        curlpp::Easy request;
       
        // 2. URL 설정
        request.setOpt(new curlpp::options::Url(buildUrl("/publicKey")));
        request.setOpt(new curlpp::options::ConnectTimeout(3)); // 연결 시도 3초 대기
        request.setOpt(new curlpp::options::Timeout(5));        // 전체 작업 5초 대기
        // 3. Header 설정
        std::list<std::string> header;
        header.push_back("Content-Type: text/plain");
        request.setOpt(new curlpp::options::HttpHeader(header));

        // 4. POST 데이터 및 크기 설정
        request.setOpt(new curlpp::options::PostFields(key_str));
        request.setOpt(new curlpp::options::PostFieldSize(key_str.length()));

        // 5. 응답 결과 수신 설정 
        std::stringstream response_stream;
        request.setOpt(new curlpp::options::WriteStream(&response_stream));

        request.perform();
    long response_code = curlpp::infos::ResponseCode::get(request);

    if (response_code == 200) {
        std::cout << "[SUCCESS] 서버가 공개키를 정상적으로 수락함 (200 OK)" << std::endl;
        return true;
    } else {
        std::cerr << "[ERROR] 서버 응답 오류! 코드: " << response_code << std::endl;
        // 500이나 404인 경우 응답 바디(response_stream)에 에러 원인이 있을 수 있음 
        std::cerr << "서버 메시지: " << response_stream.str() << std::endl;
        return false;
    }
    } catch (const std::exception& e){
        std::cerr <<"[Network Error] " << e.what() <<std::endl;
        return false;
    }
    
}