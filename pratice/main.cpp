#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<fstream>
#include <sys/stat.h> 
#include<iostream>
#include"array.hpp"

int main(){
    
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if(!cap.isOpened()){
        std::cerr<<"카메라를 찾을 수 없습니다."<<std::endl;
    }
    
    cap.set(cv::CAP_PROP_FPS, 30);
    long long frame_id = -1;

    cap.set(cv::CAP_PROP_FRAME_WIDTH,640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame, gray, blurred, edge;
    std::cout<<"시작하려면 아무 키나 누르세요..(종료 : q)"<<std::endl;

    while(true){
        cap>>frame;
        frame_id++;

        if(frame_id == SIZE) break;

        if(frame.empty()){
            std::cerr<<"프레임이 비엇습니다."<<std::endl;
            break;
        }

        int idx = frame_id % SIZE;
        Array::DataArray[idx].CID = std::to_string(frame_id);
        Array::DataArray[idx].data = std::move(frame);
        std::cout<<"Move"<<idx<<std::endl;
        std::string path = "/home/babamba/dev/lab/opencv/photo/ID_" + std::to_string(frame_id) + ".jpg";       
        // 저장 시도 및 결과 확인


        // bool isSaved = cv::imwrite(path, frame);
        // if(!isSaved) {
        //     std::cerr << "저장 실패! 경로를 확인하세요: " << path << std::endl;
        // } else {
        //     std::cout << "저장 완료: " << path << std::endl;
        // }

        // cv::imshow("Preview", frame); 
        // cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        // cv::GaussianBlur(gray, blurred, cv::Size(5,5),0);
        // cv::Canny(blurred, edge, 50, 150);

        // cv::imshow("Original", frame);
        // cv::imshow("Edge Detection", edge);

        // if(cv::waitKey(1) == 'q'){
        //     break;
        // }
    }

    for(size_t i =0; i<SIZE; i++){
        std::cout<<i<<" "<<"CID : "<< Array::DataArray[i].CID<<std::endl;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;

}