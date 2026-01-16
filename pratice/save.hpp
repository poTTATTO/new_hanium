#include<iostream>
#include<queue>
#include"datacheck.hpp"
#include<thread>
#include<atomic>


class Save{

public:
    Save();
    ~Save();
    void start_save_thread(int frame_id);
    void push_to_save_cid_queue(int);

private:
    std::string default_path = "home/babamba/deb/lab/opencv/photo/ID_";
    std::thread save_thread;
    std::queue<int> save_cid_queue;
    void save_image_task(int frame_id);
    void save_image(int frame_id);
};