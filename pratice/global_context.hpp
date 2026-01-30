#include<sodium.h>
#include <curlpp/cURLpp.hpp>  // 전역 초기화용
#include <curlpp/Easy.hpp>     // 실제 전송용 (Easy handle)
#include <curlpp/Options.hpp>  // URL, PostFields 등 옵션 설정용
#include<vector>
#include<cerrno>

class GlobalContext{

public:
    static GlobalContext& getGlobalContextInstance();
    const std::vector<unsigned char>& getPublicKey() const;
    const std::vector<unsigned char>& getPrivateKey() const;

private:
    int signKeyPair(unsigned char* public_key, unsigned char* private_key);
    GlobalContext();
    ~GlobalContext();
    std::vector<unsigned char> public_key;
    std::vector<unsigned char> private_key;

};