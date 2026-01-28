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
#include<sodium.h>
#include<iomanip>
#include<cerrno>
#include<zymkey/zkAppUtilsClass.h>
#include<zk_app_utils.h>
#include<zk_b64.h>
#include<condition_variable>

typedef long long Long;
zkAppUtils::byteArray compute_hash_sodium(const cv::Mat& frame){

    cv::Mat gray;
    if(frame.channels() > 1){
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    }else{
        gray = frame;
    }

    if(!gray.isContinuous()) gray = gray.clone();

    zkAppUtils::byteArray hash(crypto_generichash_BYTES);

    crypto_generichash(
        hash.data(),
        hash.size(),
        gray.data,
        gray.total() * gray.elemSize(),
        NULL, 0
    );

    return hash;
}

void sign_with_zymkey(const cv::Mat& frame, int slot){
    try{
        zkAppUtils::zkClass zk;

        zkAppUtils::byteArray digest = compute_hash_sodium(frame);

        zkAppUtils::byteArray* sigPtr = zk.genECDSASigFromDigest(digest, slot);

        if(sigPtr){
            std::cout<<"서명 성공 크기 : "<<sigPtr->size()<<std::endl;

            delete sigPtr;
        }
    }catch( const std::exception& e){
        std::cerr<<"에러 발생"<<e.what()<<std::endl;
    }
}

void create_new_key(int slot) {
    try {
        // [1] zkClass 객체 생성 (생성자에서 zkOpen 자동 호출) ㅋ
        zkAppUtils::zkClass zk;

        // [2] 키 생성 (타입은 헤더에 정의된 ZK_NISTP256 등 사용)
        // // 리턴값은 할당된 슬롯 번호입니다. ㅋ
        // int allocated_slot = zk.genKeyPair(ZK_NISTP256);
        // std::cout << "성공: " << allocated_slot << "번 슬롯에 키 쌍 생성 완료!" << std::endl;

        // [3] 공개키 추출 (byteArray는 std::vector<uint8_t>의 별칭입니다 ㅋ)
        // 헤더 정의: zkAppUtils::byteArray* exportPubKey(int pubkey_slot = 0, bool slot_is_foreign = 0);
        zkAppUtils::byteArray* pubKeyPtr = zk.exportPubKey(slot, false);

        if (pubKeyPtr) {
            std::cout << "공개키 크기: " << pubKeyPtr->size() << " 바이트" << std::endl;
            // 사용이 끝나면 포인터이므로 delete 해줘야 합니다. (헤더 구조상 ㅋ)
            delete pubKeyPtr;
        }

    } catch (const zkAppUtils::commonException& e) {
        std::cerr << "Zymkey 일반 에러: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "기타 에러: " << e.what() << std::endl;
    }
}

class Slot{
public:
    int id;
    Long frame_id = -1;
    std::atomic<int> tasks_left{0};
    
    alignas(std::hardware_destructive_interference_size) std::atomic<bool> is_occupied{false};
    alignas(std::hardware_destructive_interference_size) std::atomic<bool> is_valid{true};
    cv::Mat frame;
    zkAppUtils::byteArray signature;
    

    Slot(){}
    Slot(int i);

};

// 오직 연산만 수행하는 순수 서명 함수 ㅋ
std::vector<unsigned char> sign_frame_libsodium(const cv::Mat& frame, const unsigned char* secret_key) {
    // 1. 영상 가공 (libsodium 연산 전 데이터 전처리 ㅋ)
    cv::Mat gray;
    if (frame.channels() > 1) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = frame;
    }
    if (!gray.isContinuous()) gray = gray.clone();

    // 2. 해시 생성 (Ed25519 서명용 입력값 준비 ㅋ)
    std::vector<unsigned char> hash(crypto_generichash_BYTES);
    crypto_generichash(
        hash.data(), hash.size(),
        gray.data, gray.total() * gray.elemSize(),
        NULL, 0
    );

    // 3. 서명 연산 (Detached 방식 - 서명값만 생성 ㅋ)
    std::vector<unsigned char> signature(crypto_sign_BYTES);
    
    // 연산 성공 시 0 반환, 실패 시 -1 반환 ㅋ
    if (crypto_sign_detached(signature.data(), NULL, hash.data(), hash.size(), secret_key) != 0) {
        std::cerr << "[ERROR] 서명 연산 실패!" << std::endl;
        return {}; // 빈 벡터 반환 ㅋ
    }

    return signature;
}

int main(){


    if(sodium_init() < 0) throw std::runtime_error("Sodium init failed");
    // Slot slot;
    // std::cout<<sizeof(slot);

    std::vector<unsigned char> public_key(crypto_sign_PUBLICKEYBYTES);
    std::vector<unsigned char> secret_key(crypto_sign_SECRETKEYBYTES);

    crypto_sign_keypair(public_key.data(), secret_key.data());
    std::cout << "키 생성 완료! ㅋ" << std::endl;

    std::string image_path = "../test_image_coco.jpg";
    cv::Mat img = cv::imread(image_path);

    if(img.empty()){
        std::cerr<<"이미지를 읽을 수 없습니다. 경로를 확인"<<std::endl;
        return -1;
    }
    
    create_new_key(0);

    auto total_start_time = std::chrono::steady_clock::now();
    for(int i=0; i<1000; i++){
        
        sign_frame_libsodium(img, secret_key.data());
        // sign_with_zymkey(img, 0);
    }
    auto total_end_time = std::chrono::steady_clock::now();

    int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();

    std::cout<<(double)duration / 1000.0 <<" ms"<<std::endl;
       
    return 0;

}