#include"global_context.hpp"

GlobalContext::GlobalContext(){
    if(sodium_init() < 0) throw std::runtime_error("sodium_init 실패");
    public_key.resize(crypto_sign_PUBLICKEYBYTES);
    private_key.resize(crypto_sign_SECRETKEYBYTES);

    if(signKeyPair(public_key.data(), private_key.data()) == -1){
        throw std::runtime_error("sodium 키 생성 실패");
    }
    
    try{
        curlpp::initialize();
    }catch (const std::exception& e){
        throw std::runtime_error(std::string("curlpp_init failed : ") + e.what());
    }
    
    std::cout<<"--- 전역 자원 초기화 성공 ---"<<std::endl;
}

GlobalContext::~GlobalContext(){

}

static GlobalContext& GlobalContext::getGlobalContextInstance(){
    static GlobalContext instance;
    return instance;
}

const std::vector<unsigned char>& GlobalContext::getPublicKey() const{
    return public_key;
}


const std::vector<unsigned char>& GlobalContext::getPrivateKey() const{
    return private_key;
}


int GlobalContext::signKeyPair(unsigned char* public_key, unsigned char* private_key){
    if(public_key == nullptr || private_key == nullptr){
        std::cerr<<"[ERROR] 키 저장 공간이 유효하지 않습니다."<<std::endl;
        return -1;
    }

    int result = crypto_sign_keypair(public_key, private_key);

    if(result != 0){
        std::cerr<<"[ERROR] 키 쌍 생성 중 오류 발생"<<std::endl;
    }else{
        std::cout<<"[SUCCUSS] libsodium 키 쌍 생성 완료"<<std::endl;
    }

    return result;
}


