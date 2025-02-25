#include <47func.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <alibabacloud/oss/OssClient.h>
#include <wfrest/json.hpp>
using namespace AlibabaCloud::OSS;
using Json = nlohmann::json;
using namespace AmqpClient;
struct OSSInfo{
    std::string Bucket = "";
    std::string EndPoint = "";
    std::string AccessKeyID = "";
    std::string AccessKeySecret = "";
};
enum StoreType {
    LOCAL,
    OSS
};
struct MqInfo{
    enum StoreType CurrentStoreType = StoreType::OSS;
    bool IsAsyncTransferEnable = true;
    std::string transExchange = "uploadserver.trans";
    std::string transRoutingKey = "oss";
    std::string transQueue = "uploadserver.trans.oss";
};
int main(){
    MqInfo mqinfo;
    Channel::ptr_t channel = Channel::Create();
    channel->BasicConsume(mqinfo.transQueue,mqinfo.transQueue);
    InitializeSdk();
    while(true){
        Envelope::ptr_t envelope;
        bool flag = channel->BasicConsumeMessage(envelope,15000);
        if(flag == false){
            fprintf(stderr,"time out!\n");
            continue;
        }

        Json fileJson = Json::parse(envelope->Message()->Body());
        std::string filepath = fileJson["filepath"];
        std::string OSSpath = "test3/";
        OSSpath += fileJson["filehash"];

        // 创建一个OSS属性的对象
        ClientConfiguration conf; // 默认属性
        OSSInfo info;
        OssClient client(info.EndPoint, info.AccessKeyID, info.AccessKeySecret, conf);
        auto outcome = client.PutObject(info.Bucket, OSSpath, filepath);
        if (outcome.isSuccess() == false){
            fprintf(stderr, "Fail, code = %s, msg = %s, requestID = %s\n",
                    outcome.error().Code().c_str(),
                    outcome.error().Message().c_str(),
                    outcome.error().RequestId().c_str());
        }
    }
}