#include"processing.hpp"

ProcessingWorker::ProcessingWorker(SharedResourceManager& r) : res(r), //zk()
    public_key(crypto_sign_PUBLICKEYBYTES), 
    secret_key(crypto_sign_SECRETKEYBYTES)
    
    {
    if(sign_keypair(public_key.data(), secret_key.data()) == -1){
        throw std::runtime_error("키 생성 불가");
    }

}

ProcessingWorker::~ProcessingWorker(){
    stop_thread = true;
    res.cv_proc.notify_all();
    if(process_thread.joinable()) process_thread.join();
}

void ProcessingWorker::start_worker(){
    process_thread = std::thread([this] {process_task();});
}

void ProcessingWorker::process_task(){
    pthread_setname_np(pthread_self(), "Processing Thread");
    while(true){
        Long idx;
        {
            std::unique_lock<std::mutex> lock(res.m_proc);
            res.cv_proc.wait(lock, [this] {return !res.processing_q.empty() || stop_thread;});
            if(stop_thread && res.processing_q.empty()) break;
            idx = res.processing_q.front();
            res.processing_q.pop();

        }
        do_process(idx);
    }
}


int ProcessingWorker::sign_keypair(unsigned char* public_key, unsigned char* secret_key) {

    // 2. 인자 널 체크
    if (public_key == nullptr || secret_key == nullptr) {
        std::cerr << "[ERROR] 키 저장 공간이 유효하지 않습니다." << std::endl;
        return -1;
    }

    // 3. 키 생성 및 결과 반환
    // 성공 시 0, 실패 시 -1 반환 
    int result = crypto_sign_keypair(public_key, secret_key);
    
    if (result != 0) {
        std::cerr << "[ERROR] 키 쌍 생성 중 알 수 없는 오류 발생!" << std::endl;
    } else {
        std::cout << "[SUCCESS] libsodium 키 쌍 생성 완료 " << std::endl;
    }

    return result;
}

std::vector<unsigned char> ProcessingWorker::sign_frame_libsodium(const cv::Mat& frame, const unsigned char* secret_key) {
    // 1. 영상 가공 (libsodium 연산 전 데이터 전처리 )
    if (frame.empty()) {
        std::cerr << "[Processing Error] 입력 프레임이 비어 있습니다!" << std::endl;
        return {}; // 빈 결과 반환 
    }
    cv::Mat gray;
    if (frame.channels() > 1) {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = frame;
    }
    if (!gray.isContinuous()) gray = gray.clone();

    // 2. 해시 생성 (Ed25519 서명용 입력값 준비)
    std::vector<unsigned char> hash(crypto_generichash_BYTES);
    crypto_generichash(
        hash.data(), hash.size(),
        gray.data, gray.total() * gray.elemSize(),
        NULL, 0
    );

    // 3. 서명 연산 (Detached 방식 - 서명값만 생성)
    std::vector<unsigned char> signature(crypto_sign_BYTES);
    
    // 연산 성공 시 0 반환, 실패 시 -1 반환 
    if (crypto_sign_detached(signature.data(), NULL, hash.data(), hash.size(), secret_key) != 0) {
        std::cerr << "[ERROR] 서명 연산 실패!" << std::endl;
        return {}; // 빈 벡터 반환 
    }

    return signature;
}

// void ProcessingWorker::do_process(Long idx){
//    Slot& slot = res.slot_pool[idx];
//    if(slot.is_valid){
//         std::cout<<"[Processing] Processing Slot : "<<idx<<std::endl;
//         std::vector<unsigned char> signature = sign_frame_libsodium(slot.frame, secret_key.data());

//         if(signature.empty()) {
//             slot.is_valid = false;
//             std::cout<<"["<<slot.frame_id<<"]"<<"서명 실패"<<std::endl;
//         }else{
//             slot.signature = signature;
//             std::cout<<"["<<slot.frame_id<<"]"<<"서명 성공"<<std::endl;
//             slot.is_valid.store(true);
//         }
//    }
//    slot.mark_done(res.processing_q, res.m_proc, res.cv_proc);
// }
void ProcessingWorker::do_process(Long idx){
    Slot& slot = res.slot_pool[idx];

    // 1. 무결성 확인 및 이미지 존재 여부 '이중 체크' 
    if(slot.is_valid.load() && !slot.frame.empty()) {
        std::cout << "[Processing] Processing Slot : " << idx << std::endl;
        
    
        // 2. 연산 수행 
        std::vector<unsigned char> signature = sign_frame_libsodium(slot.frame, secret_key.data());

        if(signature.empty()) {
            // 어느 한 스레드라도 문제 생기면 바로 폐기
            slot.is_valid.store(false);
            std::cout << "[" << slot.frame_id.load() << "] 서명 실패" << std::endl;
        } else {
            // 결과 저장 
            slot.signature = signature; 
            std::cout << "[" << slot.frame_id.load() << "] 서명 성공" << std::endl;
            // 이미 true이므로 상태 유지 
        }
    } else if (slot.frame.empty()) {
    
        std::cerr << "[CRITICAL] 슬롯 " << idx << " 가 유효하나 이미지가 비어있음!" << std::endl;
        slot.is_valid.store(false); 
    }
    
    slot.mark_done(res.send_q, res.m_send, res.cv_send);
}

// void ProcessingWorker::do_process(Long idx){
//    Slot& slot = res.slot_pool[idx];
//    if(slot.is_valid){
//         std::cout<<"[Processing] Processing Slot : "<<idx<<std::endl;
//         zkAppUtils::byteArray* sigPtr = sign_with_zymkey(slot.frame,USE_KEY_SLOT);
//         if(sigPtr == NULL) {
//             slot.is_valid = false;
//             std::cout<<"["<<slot.frame_id<<"]"<<"서명 실패"<<std::endl;
//         }else{
//             slot.signature = *sigPtr;
//             delete sigPtr;
//             std::cout<<"["<<slot.frame_id<<"]"<<"서명 성공"<<std::endl;
//         }
//    }
//    slot.mark_done(res.processing_q, res.m_proc, res.cv_proc);
// }


// zkAppUtils::byteArray ProcessingWorker::compute_hash_sodium(const cv::Mat& frame){

//     cv::Mat gray;
//     if(frame.channels() > 1){
//         cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
//     }else{
//         gray = frame;
//     }

//     if(!gray.isContinuous()) gray = gray.clone();

//     zkAppUtils::byteArray hash(crypto_generichash_BYTES);

//     crypto_generichash(
//         hash.data(),
//         hash.size(),
//         gray.data,
//         gray.total() * gray.elemSize(),
//         NULL, 0
//     );

//     return hash;
// }

// zkAppUtils::byteArray* ProcessingWorker::sign_with_zymkey(const cv::Mat& frame, int slot){
//     try{
//         zkAppUtils::byteArray digest = compute_hash_sodium(frame);
//         if(digest.empty()) return nullptr;
        
//         zkAppUtils::byteArray* sigPtr = zk.genECDSASigFromDigest(digest, slot);

//         if(sigPtr){
//             return sigPtr;
//         }
//     }catch( const std::exception& e){
//         std::cerr<<"에러 발생"<<e.what()<<std::endl;
//         return nullptr;
//     }

//     return nullptr;
// }

// void ProcessingWorker::create_new_key(int slot) {
//     try {
//         // [1] zkClass 객체 생성 (생성자에서 zkOpen 자동 호출) 
//         zkAppUtils::zkClass zk;

//         // [2] 키 생성 (타입은 헤더에 정의된 ZK_NISTP256 등 사용)
//         // // 리턴값은 할당된 슬롯 번호입니다. 
//         // int allocated_slot = zk.genKeyPair(ZK_NISTP256);
//         // std::cout << "성공: " << allocated_slot << "번 슬롯에 키 쌍 생성 완료!" << std::endl;

//         // [3] 공개키 추출 (byteArray는 std::vector<uint8_t>의 별칭입니다 )
//         // 헤더 정의: zkAppUtils::byteArray* exportPubKey(int pubkey_slot = 0, bool slot_is_foreign = 0);
//         zkAppUtils::byteArray* pubKeyPtr = zk.exportPubKey(slot, false);

//         if (pubKeyPtr) {
//             std::cout << "공개키 크기: " << pubKeyPtr->size() << " 바이트" << std::endl;
//             // 사용이 끝나면 포인터이므로 delete 해줘야 합니다. (헤더 구조상 )
//             delete pubKeyPtr;
//         }

//     } catch (const zkAppUtils::commonException& e) {
//         std::cerr << "Zymkey 일반 에러: " << e.what() << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "기타 에러: " << e.what() << std::endl;
//     }
// }