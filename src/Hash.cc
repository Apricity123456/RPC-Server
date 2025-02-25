#include "Hash.h"
#include <openssl/sha.h>
FileHash::FileHash(std::string filepath):m_filepath(filepath){
    int fd = open(filepath.c_str(),O_RDONLY);
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    char buf[4096];
    while(1){
        bzero(buf,sizeof(buf));
        ssize_t sret = read(fd,buf,sizeof(buf));
        if(sret == 0){
            break;
        }
        SHA1_Update(&ctx,buf,sret);
    }
    unsigned char sha[20];
    SHA1_Final(sha,&ctx);
    char num[3];
    m_sha1 = "";
    for(int i = 0; i < 20; ++i){
        bzero(num,sizeof(num));
        sprintf(num,"%02x",sha[i]);
        m_sha1 += num;
    }
}