#include"config.hpp"

Config& Config::getConfigInstance(){
    static Config instance;
    return instance;
}

Config::Config(){
    readLoggerCfg();
    BuilServerUrl();
}
    
int Config::getWidth() const{
    return width;
}

int Config::getHeight() const {
    return height;
}
int Config::getFps() const{
    return fps;
}

std::string Config::getServerIp() const {
    return server_ip;
}
int Config::getServerPort() const {
    return server_port;
}

std::string Config::getHefPath() const {
    return hef_path;
}

std::string Config::getSavePath() const{
    return save_path;
}
std::string Config::getServerUrl() const{
    return server_url;
}

void Config::BuilServerUrl() {
    server_url = "http://" + server_ip + ":" + std::to_string(server_port);
}
void Config::readLoggerCfg(){
    std::ifstream file("../Sys_cfg.json");
    if(!file.is_open()){
        throw std::runtime_error("Sys_cfg.json 파일을 열 수 없음"); 
    }

    nlohmann::json j;
    file>>j;

    auto& logger_json = j["Logger"];

    this->width = logger_json.value("width", 640);
    this->height = logger_json.value("height", 640);
    this->fps = logger_json.value("fps", 30);

    this->hef_path = logger_json.value("hef path", "../yolov8s.hef");
    this->save_path = logger_json.value("save path", "../photo");

    this->server_ip = logger_json.value("Server IP addr", "127.0.0.1");
    this->server_port = logger_json.value("Server port", 8080);

    std::cout << "\n========== 설정 확인 ==========" << std::endl;
    std::cout << "[WIDTH       : ] " << this->width << std::endl;
    std::cout << "[HEIGHT      : ] " << this->height << std::endl;
    std::cout << "[FPS         : ] " << this->fps << std::endl;
    std::cout << "[HEF_PATH    : ] " << this->hef_path<<std::endl;
    std::cout << "[SAVE_PATH   : ] " << this->save_path<<std::endl;
    std::cout << "[SERVER_IP   : ] " << this->server_ip << std::endl;
    std::cout << "[SERVER_PORT : ] " << this->server_port << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "정보가 맞습니까? (y/n, 5초 후 자동 시작): " << std::flush;

    char answer = 'y'; // 기본값은 yes
    bool input_received = false;

    for (int i = 5; i > 0; --i) {
        // \r을 사용해 줄 시작점으로 이동 후 덮어쓰기
        std::cout << "\r정보가 맞습니까? (y/n) [" << i << "초 남음]: " << std::flush;

        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        tv.tv_sec = 1; // 1초씩 끊어서 대기
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

        if (ret > 0) {
            std::cin >> answer;
            input_received = true;
            break; // 입력받으면 루프 탈출
        }
    }

    if (input_received) {
        if (answer == 'n' || answer == 'N') {
            std::cout << "\n설정을 거부했습니다. 프로그램을 종료합니다." << std::endl;
            throw std::runtime_error("User rejected configuration.");
        }
    } else {
        std::cout << "\n[입력 없음] 5초가 지나 자동으로 시작합니다." << std::endl;
    }

    if(input_received){
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}