#include "Token.h"
#include <openssl/md5.h>
Token::Token(std::string username, std::string salt):
_username(username),_salt(salt){
    unsigned char md[16];
    std::string temp = username + salt;
    MD5((const unsigned char *)temp.c_str(),temp.size(),md);
    m_token = "";
    char num[3];
    for(int i = 0; i < 16; ++i){
        bzero(num,3);
        sprintf(num,"%02x",md[i]);
        m_token += num;
    }

    time_t now = time(NULL);
    struct tm * ptm = localtime(&now);
    m_token += std::to_string(ptm->tm_mon+1)+std::to_string(ptm->tm_mday)+std::to_string(ptm->tm_hour) + std::to_string(ptm->tm_min);
}