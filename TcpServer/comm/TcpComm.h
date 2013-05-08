/**
 *  @file   TcpComm.h
 *  @brief   公用头文件，数据结构、常量等
 *  @author   stanjiang
 *  @date   2012-03-29
*/
#ifndef _ZONE_TCP_COMM_H_
#define _ZONE_TCP_COMM_H_

// 系统头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

// glog头文件
#include <glog/logging.h>

// 通信协议
#include "cs_proto/role.pb.h"


#define Trace(format, args...) \
    printf("%s:%u(%s): \n\t"format"\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define FLUSH() \
    base::GetLogger(INFO)->Flush();\
    base::GetLogger(ERROR)->Flush();


/***系统类型定义 ***/
typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef unsigned long long ULLONG;


/***系统宏定义***/
#define INVALID_SOCKET              -1                        /*无效socket句柄*/
#define IP_LENGTH                   20                        /*IP地址长度*/
#define RECV_BUF_LEN                (16 * 1024)                /*接收客户端信息的缓冲区*/ 

/***系统常量参数定义***/

// 连接IP和Port
const char CONNECT_IP[] = "10.20.206.61";
const unsigned short CONNECT_PORT = 8888;

// tcpclient测试连接数量
const int MAX_CLIENT_CONN = 20000;

// tcpsvr处理最大连接数量
const int MAX_SOCKET_NUM = 31000;

const int SOCK_RECV_BUFFER = 512*1024;
const int SOCK_SEND_BUFFER = 512*1024;
const int STR_COMM_LEN = 128;
const int LISTEN_BACKLOG = 512;

const char TCPSVR_PIPE_FILE[] = "tcppipefile";

// 客户端占用共享内存 key
const int CLIENT_SHM_KEY = 1110;

// 存储socket连接的共享内存key
const int SOCKET_SHM_KEY = 1111;

// tcpsvr->gamesvr消息队列key
const int CS_MQ_SHMKEY = SOCKET_SHM_KEY+1;
// gamesvr->tcpsvr消息队列key
const int SC_MQ_SHMKEY = CS_MQ_SHMKEY+1;

// gamesvr的共享内存key
const int GAMESVR_SHM_KEY = SC_MQ_SHMKEY+1;

// tcpsvr<->gamesvr消息队列长度
const int MQ_SIZE = 1024*1024*10;


// CS通讯包包头中长度字段占用的大小
const int PKGHEAD_FIELD_SIZE = sizeof(int); 
// CS通信包最小长度
const int MIN_CSPKG_LEN = 10;
// CS通信包最大长度
const int MAX_CSPKG_LEN = RECV_BUF_LEN;
// CS打包临时缓冲区大小
const int CSPKG_OPT_BUFFSIZE = RECV_BUF_LEN*2;

// tcpsvr一次从消息队列中取得的最大消息包数,即一次最多发送给client的消息回包数
const int MAX_SEND_PKGNUM = 512;

/***系统公共数据结构定义 ***/

// 创建共享内存的模式
enum ENMShmMode
{
    ESM_None = -1,
    ESM_Init = 0,   //全新，即原来没有共享内存新创建
    ESM_Resume = 1,   //恢复，即共享原来已经存在，恢复使用
    ESM_MAX
};

// 消息队列类型
enum MQType
{
    MQ_CS = 1,
    MQ_SC = 2,
    MQ_MAX
};

// 通信服务器运行类型
enum SvrRunFlag
{
    reloadcfg = 1,
    tcpexit
};

// 网络连接出错码定义
enum ENMSocketErr
{
    Err_Succeed            =      0,    /*处理正常*/
    Err_ClientClose        =      -1,  /*客户端关闭*/         
    Err_ClientTimeout    =      -2,  /*客户端超时*/    
    Err_WriteBuffOver    =      -3,  /*Tcp写缓冲区已满*/
    Err_ReadBuffEmpty  =      -4,  /*Tcp读缓冲区为空*/
    Err_PacketError        =      -5, /*客户端发送的包错误*/
};

// tcpsvr和客户端通讯的socket结构
struct SocketConnInfo
{
    int socket_fd; // socket句柄
    ULONG uin; //玩家账号
    int recv_bytes; // 接收的字节数
    char recv_buf[RECV_BUF_LEN];  //接收到的client请求包   
    ULONG client_ip; // 客户端IP地址
    time_t  create_Time; // socket的创建时间
    time_t  recv_data_time; // 接收到数据包的时间戳
};

// tcpsvr与gamesvr通讯的包头
struct CSPkgHead
{
    int pkg_size; // cs总包长
    int fd; // cs通信socket
    ULONG client_ip; // 客户端IP地址,以便gamesvr运营时作一些统计用
};

// 统计间隔时间
const int STAT_TIME = 20;

// 统计信息
struct StatInfo
{
    int fd;
    int count;
};

#endif

