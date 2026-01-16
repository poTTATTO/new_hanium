#include"save.hpp"


Save::Save(): save_cid_queue(){}

void Save::save_image(int framd_id){

    default_path += std::to_string(frame_id) +",jpg";
    
}
void Save::push_to_save_cid_queue(int idx){
    save_cid_queue.push(idx);
}

void Save::start_save_thread(int frame_id){
    save_thread = std::thread(Save::save_image_task, this, std::ref(frame_id));
}

void Save::save_image_task(){
    save_image();
}

Save::~Save(){}