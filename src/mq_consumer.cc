#include <47func.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <string>
struct MqInfo{
    std::string transExchange = "uploadserver.trans";
    std::string transRoutingKey = "oss";
    std::string transQueue = "uploadserver.trans.oss";
};
using namespace AmqpClient;
int main(){
    MqInfo mqinfo;
    Channel::ptr_t channel = Channel::Create();
    channel->BasicConsume(mqinfo.transQueue,mqinfo.transQueue);
    Envelope::ptr_t envelope;
    //真正地去取出消息
    bool flag = channel->BasicConsumeMessage(envelope,3000);
    if(flag == true){
        fprintf(stderr,"Body = %s\n",envelope->Message()->Body().c_str());
    }
    else{
        fprintf(stderr,"time out!\n");
    }
}
