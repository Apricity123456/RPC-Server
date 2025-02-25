#include <workflow/WFFacilities.h>
#include <workflow/MySQLResult.h>
#include <csignal>
#include <wfrest/HttpServer.h>
#include "Hash.h"
#include "Token.h"
#include <wfrest/json.hpp>
#include <alibabacloud/oss/OssClient.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "rpc.srpc.h"
#include "workflow/WFFacilities.h"
using namespace srpc;
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
static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
    wait_group.done();
}

int main()
{
    signal(SIGINT, sig_handler);
    wfrest::HttpServer server;
    // GET /file/upload
    server.GET("/file/upload", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp) {
        resp->File("static/view/index.html");
    });
    // POST /file/upload
    server.POST("/file/upload", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series) {
        auto userInfo = req->query_list();
        // 提取出用户名和token
        std::string username = userInfo["username"];
        // 解析form-data类型的报文
        auto &formData = req->form();
        // 提取出文件的名字和文件的内容
        std::string filename = formData["file"].first;
        std::string filecontent = formData["file"].second; 
        // 保存文件 open write close or IOtask
        std::string filepath = "./tmp/" + filename;
        int fd = open(filepath.c_str(),O_RDWR|O_CREAT,0666);
        write(fd,filecontent.c_str(),filecontent.size());
        close(fd);
        FileHash hashObject(filepath);
        std::string filehash = hashObject.m_sha1;
        MqInfo mqinfo;
        if(mqinfo.CurrentStoreType == StoreType::OSS && mqinfo.IsAsyncTransferEnable == false){
            // 将文件备份到OSS当中
            // 初始化SDK
            InitializeSdk();
            // 创建一个OSS属性的对象
            ClientConfiguration conf; // 默认属性
            OSSInfo info;
            OssClient client(info.EndPoint, info.AccessKeyID, info.AccessKeySecret, conf);
            std::string osspath = "oss/" + filehash;
            auto outcome = client.PutObject(info.Bucket, osspath, filepath);
            if (outcome.isSuccess() == false){
                fprintf(stderr, "Fail, code = %s, msg = %s, requestID = %s\n",
                        outcome.error().Code().c_str(),
                        outcome.error().Message().c_str(),
                        outcome.error().RequestId().c_str());
            }
        }
        else if(mqinfo.CurrentStoreType == StoreType::OSS && mqinfo.IsAsyncTransferEnable == true){
            Json fileJson;
            fileJson["filehash"] = filehash;
            fileJson["filepath"] = filepath;
            Channel::ptr_t channel = Channel::Create();
            BasicMessage::ptr_t message = BasicMessage::Create(fileJson.dump());
            channel->BasicPublish(mqinfo.transExchange,mqinfo.transRoutingKey,message);
        }
        

        std::string filesizestr = std::to_string(filecontent.size());
        // 将文件的信息写入到数据库当中 tbl_file
        // 创建一个mysql任务
        auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0, [resp](WFMySQLTask *mysqlTask){
            if(mysqlTask->get_state() != WFT_STATE_SUCCESS){
                fprintf(stderr,"error msg:%s\n",WFGlobal::get_error_string(mysqlTask->get_state(),mysqlTask->get_error()));
                resp->set_status_code("500");
                return;
            }
            auto respMysql = mysqlTask->get_resp();
            if(respMysql->get_packet_type() == MYSQL_PACKET_ERROR){
                fprintf(stderr,"errorcode = %d, msg = %s\n",respMysql->get_error_code(), respMysql->get_error_msg().c_str());
                resp->set_status_code("500");
                return;
            }
            protocol::MySQLResultCursor cursor(respMysql);
            if(cursor.get_affected_rows() != 1){
                resp->set_status_code("500");
                return;
            }
            else{
                // 设计一个成功的页面，重定向过去即可
                resp->set_status_code("302");
                resp->headers["Location"] = "/file/upload/success";
            }
        });
        // 给mysql任务设置属性
        std::string sql = "INSERT INTO mycloud.tbl_file (file_sha1,file_name,file_size,file_addr,status) VALUES (";
        sql += "'" + filehash + "','" + filename + "'," + filesizestr + ",'" + filepath + "', 0);";
        sql += "INSERT INTO mycloud.tbl_user_file (user_name,file_sha1,file_name,file_size,status) VALUES('";
        sql += username + "','" + filehash + "','" + filename + "'," + filesizestr + ",0)";
        mysqlTask->get_req()->set_query(sql);
        // 将mysql任务加入到序列当中
        series->push_back(mysqlTask);
    });
    // GET /file/upload/success
    server.GET("/file/upload/success", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp) {
        resp->String("Upload success!");
    });
    // GET /file/download?filename=2.txt&filehash=40bd001563085fc35165329ea1ff5c5ecbdbbeef&filesize=3
    server.GET("/file/download", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp) {
        // //解析参数
        // auto fileInfo = req->query_list();
        // std::string filename = fileInfo["filename"];
        // std::string filehash = fileInfo["filehash"];
        // std::string filesizestr = fileInfo["filesize"];
        // int filesize = std::stoi(filesizestr);
        // //打开文件 open read or IOtask
        // std::string filepath = "./tmp/" + filename;
        // int fd = open(filepath.c_str(),O_RDONLY);
        // char *buf = new char[filesize];
        // read(fd,buf,filesize);
        // //将文件的内容写入响应报文的报文体
        // resp->append_output_body(buf,filesize);
        // //设置首部字段，触发浏览器的下载行为
        // resp->headers["Content-Type"] = "application/octet-stream";
        // resp->headers["Content-Disposition"] = "attachment;filename=" + filename;
        // delete []buf;
    
        // 解析参数
        auto fileInfo = req->query_list();
        std::string filename = fileInfo["filename"];
        // 通过filename 生成在实际文件系统的路径
        resp->headers["Location"] = "http://192.168.118.128:1235/" + filename;
        // 重定向
        resp->set_status_code("301");
    });
    
    // GET /user/signup 注册
    server.GET("/user/signup",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->File("static/view/signup.html");
    });
    // POST /user/signup 注册
    server.POST("/user/signup",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series){
        // 解析报文主体
        auto formMap = req->form_kv();
        std::string username = formMap["username"];
        std::string password = formMap["password"];
        // // 加密得到密文
        // std::string salt = "$6$SxIxUNPLMPUaGNdK$";
        // std::string encryptedPassword = crypt(password.c_str(),salt.c_str());
        // // 插入到数据库当中
        // auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0, [resp](WFMySQLTask *mysqlTask){
        //     // insert into 之后的逻辑
        //     if(mysqlTask->get_state() != WFT_STATE_SUCCESS){
        //         fprintf(stderr,"error msg:%s\n",WFGlobal::get_error_string(mysqlTask->get_state(),mysqlTask->get_error()));
        //         resp->set_status_code("500");
        //         return;
        //     }
        //     auto respMysql = mysqlTask->get_resp();
        //     if(respMysql->get_packet_type() == MYSQL_PACKET_ERROR){
        //         fprintf(stderr,"errorcode = %d, msg = %s\n",respMysql->get_error_code(), respMysql->get_error_msg().c_str());
        //         resp->set_status_code("500");
        //         return;
        //     }
        //     protocol::MySQLResultCursor cursor(respMysql);
        //     if(cursor.get_affected_rows() != 1){
        //         resp->set_status_code("500");
        //         return;
        //     }
        //     else{
        //         resp->String("SUCCESS");
        //         return;
        //     }
        // });
        // // sql语句
        // std::string sql = "INSERT INTO mycloud.tbl_user (user_name, user_pwd, status) VALUES ('";
        // sql += username + "','" + encryptedPassword +"',0);";
        // // 设置sql任务的属性
        // mysqlTask->get_req()->set_query(sql);
        // // 把sql任务加入到序列当中
        // series->push_back(mysqlTask);

        // server的作用是一个api网关
        // 先访问注册中心 得到signup服务的ip和端口
        auto httpTask = WFTaskFactory::create_http_task("http://192.168.118.128:8500/v1/agent/services",10,0,[=](WFHttpTask *httpTask){
            // 拿到了响应的内容
            const void * body;
            size_t size;
            httpTask->get_resp()->get_parsed_body(&body,&size);
            Json serviceInfo = Json::parse((char *)body);
            std::string ip = serviceInfo["SignupService1"]["Address"];
            unsigned short port = serviceInfo["SignupService1"]["Port"];
            fprintf(stderr,"ip = %s, port = %d\n", ip.c_str(), port);
            UserService::SRPCClient client(ip.c_str(), port);
            ReqSignup rpcReq;
            rpcReq.set_username(username);
            rpcReq.set_password(password);
            auto rpcTask = client.create_Signup_task([=](RespSignup *rpcResp, srpc::RPCContext *ctx){
                if(rpcResp->code() != 0){
                    fprintf(stderr,"error msg = %s\n", rpcResp->message().c_str());
                    resp->set_status_code("500");
                }
                else{
                    resp->String("SUCCESS");
                }
            });
            rpcTask->serialize_input(&rpcReq);
            series_of(httpTask)->push_back(rpcTask);
        });
        series->push_back(httpTask);
        // 使用rpc调用signup服务

    });
    // GET /static/view/signin.html
    server.GET("/static/view/signin.html",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series){
        resp->File("static/view/signin.html");
    });
    // POST /user/signin
    server.POST("/user/signin",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series){
        //解析报文内容
        auto formMap = req->form_kv();
        std::string username = formMap["username"];
        std::string password = formMap["password"];
        std::string salt = "$6$SxIxUNPLMPUaGNdK$";
        std::string encryptedPassword = crypt(password.c_str(),salt.c_str());
        //查询数据库，提取用户名所对应的密文密码
        auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0, [resp,username,encryptedPassword,series](WFMySQLTask *mysqlTask){
            //对比密码
            if (mysqlTask->get_state() != WFT_STATE_SUCCESS){
                fprintf(stderr, "error msg:%s\n", WFGlobal::get_error_string(mysqlTask->get_state(), mysqlTask->get_error()));
                resp->set_status_code("500");
                return;
            }
            auto respMysql = mysqlTask->get_resp();
            if (respMysql->get_packet_type() == MYSQL_PACKET_ERROR){
                fprintf(stderr, "errorcode = %d, msg = %s\n", respMysql->get_error_code(), respMysql->get_error_msg().c_str());
                resp->set_status_code("500");
                return;
            }
            protocol::MySQLResultCursor cursor(respMysql);
            std::vector<std::vector<protocol::MySQLCell>> rows;
            cursor.fetch_all(rows);
            if(rows.size() != 1){
                resp->set_status_code("500");
                return;
            }
            else{
                if(rows[0][0].is_string()){
                    std::string mysqlResult = rows[0][0].as_string();
                    if(mysqlResult == encryptedPassword){
                        //生成token
                        Token token(username,"12345678");
                        // 将生成的token写入表当中
                        auto newTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0,NULL);
                        std::string sql = "REPLACE INTO mycloud.tbl_user_token (user_name, user_token) VALUES('" +
                                           username + "','" + token.m_token + "');";
                        newTask->get_req()->set_query(sql);
                        series->push_back(newTask);
                        // 生成对客户端响应
                        Json respMsg;
                        Json data;
                        data["Token"] = token.m_token;
                        data["Username"] = username;
                        data["Location"] = "/static/view/home.html";
                        respMsg["data"] = data;
                        respMsg["code"] = 0;
                        respMsg["msg"] = "OK";
                        resp->String(respMsg.dump());
                    }
                }
            }
        });
        std::string sql = "SELECT user_pwd FROM mycloud.tbl_user WHERE user_name = '" + username + "' LIMIT 1;";
        mysqlTask->get_req()->set_query(sql);
        series->push_back(mysqlTask);
    });
    server.GET("/static/view/home.html",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->File("static/view/home.html");
    });
    server.GET("/static/js/auth.js",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->File("static/js/auth.js");
    });
    server.GET("/static/img/avatar.jpeg",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp){
        resp->File("static/img/avatar.jpeg");
    });
    
    server.POST("/user/info", [](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series) {
        //解析查询参数
        auto userInfo = req->query_list();
        std::string username = userInfo["username"];
        //校验token
        //查询MYSQL
        auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0,[resp,username](WFMySQLTask *mysqlTask){
            //回复响应
            if (mysqlTask->get_state() != WFT_STATE_SUCCESS){
                fprintf(stderr, "error msg:%s\n", WFGlobal::get_error_string(mysqlTask->get_state(), mysqlTask->get_error()));
                resp->set_status_code("500");
                return;
            }
            auto respMysql = mysqlTask->get_resp();
            if (respMysql->get_packet_type() == MYSQL_PACKET_ERROR){
                fprintf(stderr, "errorcode = %d, msg = %s\n", respMysql->get_error_code(), respMysql->get_error_msg().c_str());
                resp->set_status_code("500");
                return;
            }
            protocol::MySQLResultCursor cursor(respMysql);
            std::vector<std::vector<protocol::MySQLCell>> rows;
            cursor.fetch_all(rows);
            if(rows.size() != 1){
                resp->set_status_code("500");
                return;
            }
            else{
                std::string signupat = rows[0][0].as_string();
                Json respMsg;
                Json data;
                data["Username"] = username;
                data["SignupAt"] = signupat;
                respMsg["data"] = data;
                resp->String(respMsg.dump());
            }
        });
        std::string sql = "SELECT signup_at FROM mycloud.tbl_user WHERE user_name = '"+ username + "';";
        mysqlTask->get_req()->set_query(sql);
        series->push_back(mysqlTask);

    });
    server.POST("/file/query",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series){
        // 提取客户端的信息
        auto userInfo = req->query_list();
        std::string username = userInfo["username"];
        auto formMap = req->form_kv();
        std::string limit = formMap["limit"];
        // 查询操作
        auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:123@localhost",0,[resp](WFMySQLTask *mysqlTask){
            //回复响应
            if (mysqlTask->get_state() != WFT_STATE_SUCCESS){
                fprintf(stderr, "error msg:%s\n", WFGlobal::get_error_string(mysqlTask->get_state(), mysqlTask->get_error()));
                resp->set_status_code("500");
                return;
            }
            auto respMysql = mysqlTask->get_resp();
            if (respMysql->get_packet_type() == MYSQL_PACKET_ERROR){
                fprintf(stderr, "errorcode = %d, msg = %s\n", respMysql->get_error_code(), respMysql->get_error_msg().c_str());
                resp->set_status_code("500");
                return;
            }
            protocol::MySQLResultCursor cursor(respMysql);
            std::vector<std::vector<protocol::MySQLCell>> rows;
            cursor.fetch_all(rows);
            Json respMsg;
            for(auto &row:rows){
                // row[0] id  
                // row[1] user_name
                // row[2] file_sha1
                Json fileMsg;
                fileMsg["FileHash"] = row[2].as_string();
                // row[3] file_size
                fileMsg["FileSize"] = row[3].as_ulonglong();
                // row[4] file_name
                fileMsg["FileName"] = row[4].as_string();
                // row[5] upload_at  
                fileMsg["UploadAt"] = row[5].as_string();
                // row[6] last_update 
                fileMsg["LastUpdated"] = row[6].as_string();
                respMsg.push_back(fileMsg);
            }
            resp->String(respMsg.dump());
        });
        std::string sql = "SELECT * FROM mycloud.tbl_user_file WHERE user_name = '" + username + "' AND status = 0 LIMIT " + limit + ";";
        mysqlTask->get_req()->set_query(sql);
        series->push_back(mysqlTask);
    });
    server.POST("/file/downloadurl",[](const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series){
        auto userInfo = req->query_list();
        std::string filename = userInfo["filename"];
        std::string url = "http://192.168.118.128:1235/" + filename;
        resp->String(url);
    });
    if (server.track().start(1234) == 0)
    {
        server.list_routes();
        wait_group.wait();
        server.stop();
    }
    else
    {
        fprintf(stderr, "Cannot start server");
        exit(1);
    }
    return 0;
}
