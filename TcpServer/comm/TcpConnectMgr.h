/**
 *  @file   TcpConnectMgr.h
 *  @brief   tcp连接管理器
 *  @author   stanjiang
 *  @date   2012-03-29
*/
#ifndef _ZONE_TCPCONNECT_MGR_H_
#define _ZONE_TCPCONNECT_MGR_H_

#include "TcpComm.h"

class CMsgQueue;

class CTcpConnectMgr
{
public:
    CTcpConnectMgr(){}
    ~CTcpConnectMgr(){close(epoll_fd_); close(listen_fd_);}
    
    /***
     *  @brief   创建tcp连接管理器实例,即内存托管在共享内存中
     *  @param   无
     *  @return   实例指针
     ***/
    static CTcpConnectMgr*  CreateInstance(void);

    /***
     *  @brief   计算连接器所需内存大小
     *  @param   无
     *  @return   内存大小
     ***/
    static int CountSize(void);

    /***
     *  @brief   重载new与delete操作,使其对象在共享内存中分配与回收空间
     *  @return   void
     ***/
    static void* operator new(size_t size);
    static void operator delete(void* mem);

    /***
     *  @brief   设置运行标志
     *  @param   flag: 通信服务器运行标志
     *  @return   void
     ***/
    void SetRunFlag(int flag){run_flag_ = flag;}

    /***
     *  @brief   初始化tcp连接管理器
     *  @param   无
     *  @return   0: ok , -1: error
     ***/
    int Init(void);

    /***
     *  @brief   运行tcp连接管理器,即进入tcp连接处理主循环
     *  @param   无
     *  @return   void
     ***/
    void Run(void);

    /***
     *  @brief  基于Tcp连接的发送数据接口
     *  @param   fd: 连接socket
     *  @param   databuf: 待发送数据buf
     *  @param   len: 待发送数据长度
     *  @return   0: ok , -1: error
     ***/
    static int TcpSendData(int fd, const char* databuf, int len);

    /***
     *  @brief  基于Tcp连接的接收数据接口
     *  @param   fd: 连接socket
     *  @param   databuf: 待接收数据buf
     *  @param   len: 接收到数据的实际长度
     *  @return   0: ok , -1: error
     ***/
    static int TcpRecvData(int fd, char* databuf, int& len);

    /***
     *  @brief   设置套接字为非阻塞状态
     *  @param   fd: 套接字
     *  @return   0: ok , -1: error
     ***/
    static int SetNonBlock(int fd);

    /***
     *  @brief   设置套接字各特定选项,如:是否重用,send/recv buffer等
     *  @param   fd: 套接字
     *  @return   0: ok , -1: error
     ***/
    static int SetSocketOpt(int fd);

    /***
     *  @brief   设置套接字地址信息
     *  @param   ip: ip字符串
     *  @param   port: 端口地址
     *  @param   addr: 套接字地址
     *  @return    fd
     ***/
    static void SetAddress(const char* ip, unsigned short port, struct sockaddr_in* addr);

private:
    
    /***
     *  @brief   读取客户端请求
     *  @return   void
     ***/
    void GetClientMessage(void);

    /***
     *  @brief   读取tcpsvr<-->gamesvr的消息队列中收取待发送至client的数据
     *  @return   void
     ***/
    void CheckWaitSendData(void);

    /***
     *  @brief   检测通讯超时的socket
     *  @return   void
     ***/
    void CheckTimeOut(void);

private:

    /***
     *  @brief   初始化tcpsvr监听socket
     *  @param   ip: ip字符串
     *  @param   port: 端口地址
     *  @return   0: ok , -1: error
     ***/
    int InitListenSocket(const char* ip, unsigned short port);

    /***
     *  @brief   因通信错误而清除socket相关信息
     *  @param   fd: socket句柄
     *  @param   type: 错误码
     *  @return   void
     ***/
    void ClearSocketInfo(int fd, ENMSocketErr type);

    /***
     *  @brief   接收客户端连接的数据
     *  @param   fd: accept生成的socket
     *  @return   0: ok , -1: error
     ***/
    int RecvClientData(int fd);

    /***
     *  @brief   增加/减少连接数量
     *  @return   
     ***/	
    void IncSockConn(void){++cur_conn_num_;}
    void DecSockConn(void){--cur_conn_num_;}

    /***
     *  @brief   创建消息队列
     *  @param   type : 消息队列类型
     *  @return   0: ok , -1: error
     ***/
    int CreateMQ(MQType type);
    
private:
    static char* current_shmptr_; // attath共享内存地址
   
    int run_flag_; // 运行标志
    SocketConnInfo client_sockconn_list_[MAX_SOCKET_NUM]; // 客户端socket连接信息列表
    char send_client_buf_[SOCK_SEND_BUFFER]; // 发送消息至client的buf

    struct epoll_event  events_[MAX_SOCKET_NUM]; // epoll事件集合
    
    int epoll_fd_; // 服务器epoll创建的句柄    
    int listen_fd_; // 监听socket句柄
    int maxfds_; // 最大sockt句柄数
    int epoll_timeout_; // epoll wait超时时间
    int cur_conn_num_; // 当前连接数    

    CMsgQueue* mq_cs_; // tcpsvr->gamesvr消息队列
    CMsgQueue* mq_sc_; // gamesvr->tcp消息队列

    int send_pkg_count_;
    int recv_pkg_count_;
    int laststat_time_;
};





#endif



