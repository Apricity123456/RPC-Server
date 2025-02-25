#include <iostream>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <string>
struct MqInfo{
    std::string transExchange = "uploadserver.trans";
    std::string transRoutingKey = "oss";
};
using namespace AmqpClient;
int main(){
    MqInfo mqinfo;
    Channel::ptr_t channel = Channel::Create();
    //pause(); 
    BasicMessage::ptr_t message = BasicMessage::Create("Hello");
    channel->BasicPublish(mqinfo.transExchange,mqinfo.transRoutingKey,message);
    
}
