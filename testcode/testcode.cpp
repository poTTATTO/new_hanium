#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<fstream>
#include <sys/stat.h> 
#include<iostream>
#include<thread>
#include<vector>
#include<atomic>
#include<chrono>

struct Counter{
    std::atomic<int> countA{0};
    std::atomic<int> countB{0};
};

void work(std::atomic<int>& target, int iterations){
    for(int i=0; i<iterations; i++){
        target++;
    }
}

int main(){
    // std::string image_path = "cat.jpeg";
    // cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    // cv::Mat img_moved;

    // img_moved = std::move(img);

    // if(img.empty()){
    //     std::cout<<"Could not read the image : "<<image_path<<std::endl;
    //     return 1;
    // }

    // cv::imshow("Display window", img);
    // int k = cv::waitKey(0); // Wait for a keyStroke in the window

    // if(k == 's'){
    //     cv::imwrite("starry_nigh.png", img);
    // }
    // cv::Mat E = cv::Mat::eye(4,4, CV_64F);
    // std::cout<<"M = "<<std::endl<<" "<<E<<std::endl<<std::endl;

    // cv::Mat O = cv::Mat::ones(2,2, CV_32F);
    // std::cout<<"M = "<<std::endl<<" "<<O<<std::endl<<std::endl;

    // cv::Mat Z = cv::Mat::zeros(3,3 ,CV_8UC(2));
    // std::cout<<"M = "<<std::endl<<" "<<Z<<std::endl<<std::endl;

    // cv::Point2f P(5,1);
    // std::cout<<"Point (2D) = " << P << std::endl;

    // cv::Point3f P3f(2,6,7);
    // std::cout<<"Point (3D) = " << P3f<<std::endl;
    
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if(!cap.isOpened()){
        std::cerr<<"카메라를 찾을 수 없습니다."<<std::endl;
    }
    
    cap.set(cv::CAP_PROP_FPS, 30);
    long long frame_id = 0;

    cap.set(cv::CAP_PROP_FRAME_WIDTH,640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame, gray, blurred, edge;
    std::cout<<"시작하려면 아무 키나 누르세요..(종료 : q)"<<std::endl;

    while(true){
        if(frame_id == 300) break;
        cap>>frame;
        frame_id++;

        if(frame.empty()){
            std::cerr<<"프레임이 비엇습니다."<<std::endl;
            break;
        }

        
        std::string path = "/home/babamba/dev/lab/opencv/photo/ID_" + std::to_string(frame_id) + ".jpg";       
        // 저장 시도 및 결과 확인
         auto total_start_time = std::chrono::steady_clock::now();

         
        bool isSaved = cv::imwrite(path, frame);
        if(!isSaved) {
            std::cerr << "저장 실패! 경로를 확인하세요: " << path << std::endl;
        } else {
            std::cout << "저장 완료: " << path << std::endl;
        }
        auto total_end_time = std::chrono::steady_clock::now();

        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();

        std::cout<<duration<<" ms"<<std::endl;
       
    }   
    //     cv::imshow("Preview", frame); 
    //     // cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    //     // cv::GaussianBlur(gray, blurred, cv::Size(5,5),0);
    //     // cv::Canny(blurred, edge, 50, 150);

    //     // cv::imshow("Original", frame);
    //     // cv::imshow("Edge Detection", edge);

    //     // if(cv::waitKey(1) == 'q'){
    //     //     break;
    //     // }
    //     cv::waitKey(1);
    // }

    // cap.release();
    // cv::destroyAllWindows();

    // Counter myCounter;
    // int numIterations = 10000000;


    // auto total_start_time = std::chrono::steady_clock::now();

    // std::thread t1(work, std::ref(myCounter.countA), numIterations);
    // std::thread t2(work, std::ref(myCounter.countB), numIterations);

    // std::cout<<"시작"<<std::endl;
    // t1.join();
    // t2.join();
    // auto total_end_time = std::chrono::steady_clock::now();

    // int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();

    // std::cout<<duration<<" ms"<<std::endl;
    // std::cout<<"최종 결과 - A: "<<myCounter.countA<<", B: "<<myCounter.countB<<std::endl;
    return 0;

}