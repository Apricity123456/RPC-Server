
#include "rpc.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

static void signup_done(RespSignup *response, srpc::RPCContext *context)
{
	//处理 rpc调用的返回值
	int code = response->code();
	std::string message = response->message();
	printf("code = %d, message = %s\n", code, message.c_str());
}

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	const char *ip = "127.0.0.1";
	unsigned short port = 1412;

	UserService::SRPCClient client(ip, port);//client本质是server的代理

	ReqSignup signup_req;//准备函数rpc的参数
	signup_req.set_username("admin1");
	signup_req.set_password("123");
	//调用rpc
	client.Signup(&signup_req, signup_done);
	// signup_done本质是一个回调函数，当服务端回复消息以后调用

	wait_group.wait();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
