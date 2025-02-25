#include <47func.h>
#include <workflow/MySQLResult.h>
#include "rpc.srpc.h"
#include "workflow/WFFacilities.h"
#include <ppconsul/consul.h>
#include <ppconsul/agent.h>
using namespace srpc;
using namespace ppconsul;
static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class UserServiceServiceImpl : public UserService::Service
{
public:

	void Signup(ReqSignup *request, RespSignup *response, srpc::RPCContext *ctx) override
	{
		// rpc的被调函数
		// request 请求的参数
		// response 将要返回给客户端的返回值
		// ctx 一些连接的信息和其他补充的内容
		std::string username = request->username();
		std::string password = request->password();
		std::string salt = "$6$SxIxUNPLMPUaGNdK$";
        std::string encryptedPassword = crypt(password.c_str(),salt.c_str());
		auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0, [response](WFMySQLTask *mysqlTask){
            // insert into 之后的逻辑
            if(mysqlTask->get_state() != WFT_STATE_SUCCESS){
                fprintf(stderr,"error msg:%s\n",WFGlobal::get_error_string(mysqlTask->get_state(),mysqlTask->get_error()));
				response->set_code(-1);
				response->set_message("mysql connect error!");
                return;
            }
            auto respMysql = mysqlTask->get_resp();
            if(respMysql->get_packet_type() == MYSQL_PACKET_ERROR){
                fprintf(stderr,"errorcode = %d, msg = %s\n",respMysql->get_error_code(), respMysql->get_error_msg().c_str());
				response->set_code(-1);
				response->set_message("SQL error!");
                return;
            }
            protocol::MySQLResultCursor cursor(respMysql);
            if(cursor.get_affected_rows() != 1){
				response->set_code(-1);
				response->set_message("INSERT INTO error!");
                return;
            }
            else{
				response->set_code(0);
				response->set_message("success!");
                return;
            }
        });
        // sql语句
        std::string sql = "INSERT INTO mycloud.tbl_user (user_name, user_pwd, status) VALUES ('";
        sql += username + "','" + encryptedPassword +"',0);";
        // 设置sql任务的属性
        mysqlTask->get_req()->set_query(sql);
        // 把sql任务加入到序列当中
        ctx->get_series()->push_back(mysqlTask);
	}
};
void timerCallback(WFTimerTask *timerTask){
	agent::Agent *pagent = (agent::Agent *)timerTask->user_data;
	pagent->servicePass("SignupService1");
	auto nextTask = WFTaskFactory::create_timer_task(5*1000*1000,timerCallback);
	nextTask->user_data = pagent;
	series_of(timerTask)->push_back(nextTask);
}
int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	unsigned short port = 1412;
	SRPCServer server; // server本质上是client的代理
	//server运作逻辑，非常类似于workflow的httpserver的运作逻辑
	// 当有客户端/主调方 调用rpc时，server会创建一个序列，当序列执行完成之后，回复消息给客户端
	UserServiceServiceImpl userservice_impl;
	// 把样例代码当中的抽象类，先继承并重写，实例化一个对象
	server.add_service(&userservice_impl);
	// 把被调函数加入到server当中

	//创建一个Consul对象
	Consul consul("127.0.0.1:8500",kw::dc = "dc1");
	//创建一个agent
	agent::Agent agent(consul);
	//注册本服务
	agent.registerService(
		agent::kw::name = "SignupService1",
		agent::kw::address = "127.0.0.1",
		agent::kw::id = "SignupService1",
		agent::kw::port = 1412,
		agent::kw::check = agent::TtlCheck(std::chrono::seconds(10))
	);
	//提示注册中心，本服务存活
	agent.servicePass("SignupService1");
	// 设计一个永远存在的定时任务
	auto timerTask = WFTaskFactory::create_timer_task(5*1000*1000,timerCallback);
	timerTask->user_data = &agent;
	timerTask->start();
	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
