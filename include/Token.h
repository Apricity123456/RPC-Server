#include <47func.h>
#include <string>
class Token{
    // 40byte  32byte MD5（用户+盐值）结果 8byte 日期
public:
    std::string m_token;
    Token(std::string username, std::string salt);
private:
    std::string _username;
    std::string _salt;
};