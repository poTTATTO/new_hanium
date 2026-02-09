#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<unistd.h>
#include<sys/select.h> // select 시스템 콜을 위해 필요
#include <limits>
class Config{

private:
    int width;
    int height;
    int fps;

    std::string server_ip;
    int server_port;

    std::string hef_path;
    std::string save_path;

    std::string server_url;

    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

public:
    static Config& getConfigInstance();

    int getWidth() const;
    int getHeight() const;
    int getFps() const;

    std::string getServerIp() const;
    int getServerPort() const;
    void BuilServerUrl();
    std::string getServerUrl() const;
    
    std::string getHefPath() const;
    std::string getSavePath() const;
    void readLoggerCfg();

};