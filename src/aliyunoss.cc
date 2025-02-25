#include <string>
#include <alibabacloud/oss/OssClient.h>
using namespace AlibabaCloud::OSS;
struct OSSInfo{
    std::string Bucket = "";
    std::string EndPoint = "";
    std::string AccessKeyID = "";
    std::string AccessKeySecret = "";
};
int main(){
    // 初始化SDK
    InitializeSdk();
    // 创建一个OSS属性的对象
    ClientConfiguration conf;//默认属性
    OSSInfo info;
    OssClient client(info.EndPoint,info.AccessKeyID,info.AccessKeySecret,conf);

    // //client的PutObject方法 可以上传文件
    // std::string ossPath = "test/2.txt";
    // auto outcome = client.PutObject(info.Bucket,ossPath,"tmp/2.txt");
    // if(outcome.isSuccess() == false){
    //     fprintf(stderr,"Fail, code = %s, msg = %s, requestID = %s\n",
    //         outcome.error().Code().c_str(),
    //         outcome.error().Message().c_str(),
    //         outcome.error().RequestId().c_str()
    //     );
    // }
    
    // 生成一个下载链接
    std::string ossPath = "test/2.txt";
    time_t expire = time(NULL) + 1200;//让下载链接存在20分钟的有效期
    auto outcome = client.GeneratePresignedUrl(info.Bucket,ossPath,expire,Http::Get);
    if(outcome.isSuccess() == false){
        fprintf(stderr,"Fail, code = %s, msg = %s, requestID = %s\n",
            outcome.error().Code().c_str(),
            outcome.error().Message().c_str(),
            outcome.error().RequestId().c_str()
        );        
    }
    else{
        fprintf(stderr,"Url = %s\n",outcome.result().c_str());
    }
    // 关闭SDK
    ShutdownSdk();
}