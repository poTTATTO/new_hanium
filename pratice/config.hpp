#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<nlohmann/json.hpp>
#include<fstream>
#include<unistd.h>
#include<sys/select.h> // select 시스템 콜을 위해 필요

class Config{

private:
    int width;
    int height;
    int fps;

    std::string server_ip;
    int server_port;

    std::string hef_path;

public:
    int getWidth() const;
    int getHeight() const;

    std::string getServerIp() const;
    int getServerPort() const;

    std::string getHefPath() const;
    void readLoggerCfg();

    Config();
    ~config();
};