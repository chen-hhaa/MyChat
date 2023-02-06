#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <iconv.h>

#include <map>
#include <iostream>
#include <regex>

#include "ConfigReader.h"
#include "UserManager.h"

using namespace std;

// 最大连接数
const int MAX_CONN = 1024;

// 用于保存客户端信息的结构体
struct Client {
    int sockfd;
    std::string name;
};

UserManager* user_manager;
// 保存客户端信息的 map
map<int, Client> clients;

UserManager* db_connect(){
    ConfigReader cfg_reader("../config/mysql.conf");
    cfg_reader.readConfig();
    unordered_map<string, string> cfg_dic = cfg_reader.get_config_dic();
    UserManager* userInfo_manager = UserManager::GetInstance(cfg_dic);
    return userInfo_manager;
}

bool Gbk2Utf(string const& src_str, string& dst_str, 
string const& src_encoding = "gbk", string const& dst_encoding = "utf8")
{
    iconv_t cd;
    std::size_t src_len = src_str.size();
    std::size_t dst_len = src_len * 2;
    dst_str.resize(dst_len);
    cd = iconv_open(dst_encoding.c_str(), src_encoding.c_str());
    if (cd == 0)
    return false;
    char* src_data = (char*)(src_str.data());
    char* dst_data = (char*)(dst_str.data());
    std::size_t left_len {dst_len};
    if (iconv(cd, &src_data, &src_len, &dst_data, &left_len) == -1)
    return false;
    iconv_close(cd);
    dst_str.resize(dst_len-left_len);
    return true;
}

//登录信息处理,返回服务器处理结果
string loginInfoProc(string & message, int sockfd){
    cout << message << endl;
    // 提取用户名和密码信息
    string gbk_user_name, user_name, user_pwd;
    regex p("\\[\\d{1}\\]\\[(.*)\\]\\[(.*)\\]"); // C++11 需要使用两个"\\"符号
    smatch matchRes;

    if(regex_match(message, matchRes, p)){
        gbk_user_name = matchRes[1].str();
        Gbk2Utf(gbk_user_name, user_name);
        cout << "user_name:" << user_name << endl;
        user_pwd = matchRes[2].str();
        cout << "user_pwd:" << user_pwd << endl; 
    }
    else {
        cout << "Error: 用户名和密码解析失败！" << endl;
    }

    // 查询用户信息数据库，验证信息
    string msg = "";
    // 如果密码正确, 返回客户端ok
    if(user_pwd == user_manager->get_password(user_name)){
        msg = "ok";
        clients[sockfd].name=gbk_user_name;
    }
    else if (user_manager->get_password(user_name) == ""){
        msg = "user does not exist!";
    }
    else {
        msg = "Incorrect passowrd!";
    }
    write(sockfd, msg.c_str(), msg.size());
    return msg;
}

int main() {
    //数据库连接
    user_manager = db_connect();
    
    // 创建监听 socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // 处理错误
    }

    // 绑定地址和端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        // 处理错误
    }

    // 设置 socket 为监听状态
    ret = listen(sockfd, 5);
    if (ret < 0) {
        // 处理错误
    }

    // 创建 epoll 实例
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        // 处理错误
    }

    // 将监听 socket 加入 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    if (ret < 0) {
        // 处理错误
    }
    

    // 循环监听
    while (true) {
        struct epoll_event events[MAX_CONN];
        int n = epoll_wait(epfd, events, MAX_CONN, -1);
        if (n < 0) {
            // 处理错误
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            // 如果是监听 socket，则接受连接
            if (fd == sockfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
                if (client_sockfd < 0) {
                    // 处理错误
                }

                // 将新连接的 socket 加入 epoll
                ev.events = EPOLLIN;
                ev.data.fd = client_sockfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
                if (ret < 0) {
                    // 处理错误
                }

                // 将新连接的 socket 加入 map
                Client client;
                client.sockfd = client_sockfd;
                client.name = "";
                clients[client_sockfd] = client;
            }
            else {
                // 如果是已连接的 socket，则读取数据
                char buffer[1024];
                int n = read(fd, buffer, 1024);
                if (n < 0) {
                    // 处理错误
                }
                else if (n == 0) {
                    // 客户端断开连接
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    clients.erase(fd);
                }
                else {
                    // 处理读取到的数据
                    std::string message(buffer, n);
                    
                    //解析从服务端读取到的数据(三种信息：1登录信息，2注册信息，3聊天信息)
                    if (message[1] == '1') //登录信息
                    {
                        loginInfoProc(message, fd);
                    }
                    else if(message[1] == '2') // 注册信息
                    {
                        // TODO
                    }
                    else if(message[1] == '3') // 聊天信息
                    {
                        // TODO
                        message = message.substr(3);
                        string name = clients[fd].name;
                        // 广播消息
                        for (auto& c : clients) {
                            if (c.first != fd) {
                                write(c.first, ('[' + name +']' + ": " + message).c_str(), message.size() + name.size() + 4);
                            }
                        }
                    }


                    // if (clients[fd].name.empty()) {
                    //     // 如果客户端还没有设置名称，则认为是设置名称的消息
                    //     clients[fd].name = message;
                    //     // 广播新用户加入消息
                    //     std::string msg = "-----[" + message + ']' + "joined the chat room!-----";
                    //     for (auto& c : clients) {
                    //         write(c.first, msg.c_str(), msg.size());
                    //     }
                    // }
                    // else {
                    //     // 否则认为是聊天消息
                        // std::string name = clients[fd].name;
                        // // 广播消息
                        // for (auto& c : clients) {
                        //     if (c.first != fd) {
                        //         write(c.first, ('[' + name +']' + ": " + message).c_str(), message.size() + name.size() + 4);
                        //     }
                        // }
                    // }
                }
            }
        }
    }
    // 关闭 epoll 实例
    close(epfd);
    // 关闭监听 socket
    close(sockfd);
    return 0;
}


