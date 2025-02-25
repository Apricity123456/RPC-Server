#include <iostream>
#include "Signup.pb.h"
int main(){
    ReqSignup message;
    message.set_username("admin");
    message.set_password("123");
    // 序列化
    char buf[100] = {0};
    message.SerializeToArray(buf,100);
    for(int i = 0; i < 100; ++i){
        printf("%02x",buf[i]);
    }
    printf("\n");
}