#include "Hash.h"
#include "Token.h"
int main(){
    //FileHash filehash("tmp/2.txt");
    //fprintf(stderr,"filehash = %s\n", filehash.m_sha1.c_str());
    std::string username = "admin";
    std::string salt = "12345678";
    Token token(username,salt);
    fprintf(stderr,"token = %s\n", token.m_token.c_str());
}