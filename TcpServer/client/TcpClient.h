/**
 *  @file   TcpClient.h
 *  @brief   tcp通讯客户端
 *  @author   stanjiang
 *  @date   2012-04-04
*/
#ifndef _ZONE_TCPCLIENT_H_
#define _ZONE_TCPCLIENT_H_

#include "TcpComm.h"
#include <string>

struct ClientOptInfo
{
    int fd; // socket
    bool send_flag; // 数据发送标志,true:可以发送数据,false:不可以发送
    bool recv_flag; // 数据接收标志
    int recv_bytes; // 接收的字节数
    char recv_buf[RECV_BUF_LEN];  //接收到的client请求包       
    int uin;
};

class CTcpClient
{
public:
    CTcpClient(){}
    ~CTcpClient(){Destroy();}

    /***
     *  @brief   创建实例,即内存托管在共享内存中
     *  @param   无
     *  @return   实例指针
     ***/
    static CTcpClient*  CreateInstance(void);

    /***
     *  @brief   重载new与delete操作,使其对象在共享内存中分配与回收空间
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);


    /***
     *  @brief   初始化
     *  @param   port: 端口地址
     *  @param   addr: 套接字地址     
     *  @return    void
     ***/
    int Init(const char* ip, unsigned short port);

    /***
     *  @brief   进入客户端事件循环
     *  @return    void
     ***/
    void Run(void);

private:
    /***
     *  @brief   创建客户端套接字
     *  @param   port: 端口地址
     *  @param   addr: 套接字地址
     *  @return    fd
     ***/
    int CreateSocket(const char* ip, unsigned short port);

    /***
     *  @brief   预处理IO事件
     *  @return    void
     ***/
    void PrepareIOEvent(void);

    /***
     *  @brief   定时处理
     *  @return    void
     ***/
    void TickHandle(void);
    
    /***
     *  @brief   发送CS消息
     *  @return    void
     ***/
    void SendDataToServera(void);

    /***
     *  @brief   发送CS消息
     *  @return    void
     ***/
    void RecvDataFromServer(void);

    /***
     *  @brief   打印消息包内容
     *  @param   addr: 套接字地址
     *  @return    void
     ***/
    void PrintMsg(const std::string buf);

    /***
     *  @brief   发送AccountLoginReq消息
     *  @param   fd: client套接字
     *  @param   uin: client 账号
     *  @return    void
     ***/
    int SendAccountLoginReq(int fd, unsigned int uin);

    /***
     *  @brief   接收AccountLoginRes消息
     *  @param   fd: client套接字
     *  @return    void
     ***/
    int RecvAccountLoginRes(int fd, const char* pkg, int len);

    /***
     *  @brief   进程退出前清理数据
     *  @return    void
     ***/
    void Destroy(void);
    
private:
    static char* current_shmptr_; // 客户端共享内存地址
    
    int client_conn_[MAX_CLIENT_CONN];
    int maxfd_;
    ClientOptInfo client_opt_[MAX_CLIENT_CONN+1]; 
    int client_epoll_fd_;  // 客户端epoll创建的句柄
    struct epoll_event client_events_[MAX_CLIENT_CONN];    
    int cur_conn_num_;

    int send_pkg_count_;
    int recv_pkg_count_;
    time_t laststat_time_;
};



#endif
