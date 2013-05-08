/**
 *  @file   TcpComm.h
 *  @brief   ����ͷ�ļ������ݽṹ��������
 *  @author   stanjiang
 *  @date   2012-03-29
*/
#ifndef _ZONE_TCP_COMM_H_
#define _ZONE_TCP_COMM_H_

// ϵͳͷ�ļ�
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

// glogͷ�ļ�
#include <glog/logging.h>

// ͨ��Э��
#include "cs_proto/role.pb.h"


#define Trace(format, args...) \
    printf("%s:%u(%s): \n\t"format"\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define FLUSH() \
    base::GetLogger(INFO)->Flush();\
    base::GetLogger(ERROR)->Flush();


/***ϵͳ���Ͷ��� ***/
typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef unsigned long long ULLONG;


/***ϵͳ�궨��***/
#define INVALID_SOCKET              -1                        /*��Чsocket���*/
#define IP_LENGTH                   20                        /*IP��ַ����*/
#define RECV_BUF_LEN                (16 * 1024)                /*���տͻ�����Ϣ�Ļ�����*/ 

/***ϵͳ������������***/

// ����IP��Port
const char CONNECT_IP[] = "10.20.206.61";
const unsigned short CONNECT_PORT = 8888;

// tcpclient������������
const int MAX_CLIENT_CONN = 20000;

// tcpsvr���������������
const int MAX_SOCKET_NUM = 31000;

const int SOCK_RECV_BUFFER = 512*1024;
const int SOCK_SEND_BUFFER = 512*1024;
const int STR_COMM_LEN = 128;
const int LISTEN_BACKLOG = 512;

const char TCPSVR_PIPE_FILE[] = "tcppipefile";

// �ͻ���ռ�ù����ڴ� key
const int CLIENT_SHM_KEY = 1110;

// �洢socket���ӵĹ����ڴ�key
const int SOCKET_SHM_KEY = 1111;

// tcpsvr->gamesvr��Ϣ����key
const int CS_MQ_SHMKEY = SOCKET_SHM_KEY+1;
// gamesvr->tcpsvr��Ϣ����key
const int SC_MQ_SHMKEY = CS_MQ_SHMKEY+1;

// gamesvr�Ĺ����ڴ�key
const int GAMESVR_SHM_KEY = SC_MQ_SHMKEY+1;

// tcpsvr<->gamesvr��Ϣ���г���
const int MQ_SIZE = 1024*1024*10;


// CSͨѶ����ͷ�г����ֶ�ռ�õĴ�С
const int PKGHEAD_FIELD_SIZE = sizeof(int); 
// CSͨ�Ű���С����
const int MIN_CSPKG_LEN = 10;
// CSͨ�Ű���󳤶�
const int MAX_CSPKG_LEN = RECV_BUF_LEN;
// CS�����ʱ��������С
const int CSPKG_OPT_BUFFSIZE = RECV_BUF_LEN*2;

// tcpsvrһ�δ���Ϣ������ȡ�õ������Ϣ����,��һ����෢�͸�client����Ϣ�ذ���
const int MAX_SEND_PKGNUM = 512;

/***ϵͳ�������ݽṹ���� ***/

// ���������ڴ��ģʽ
enum ENMShmMode
{
    ESM_None = -1,
    ESM_Init = 0,   //ȫ�£���ԭ��û�й����ڴ��´���
    ESM_Resume = 1,   //�ָ���������ԭ���Ѿ����ڣ��ָ�ʹ��
    ESM_MAX
};

// ��Ϣ��������
enum MQType
{
    MQ_CS = 1,
    MQ_SC = 2,
    MQ_MAX
};

// ͨ�ŷ�������������
enum SvrRunFlag
{
    reloadcfg = 1,
    tcpexit
};

// �������ӳ����붨��
enum ENMSocketErr
{
    Err_Succeed            =      0,    /*��������*/
    Err_ClientClose        =      -1,  /*�ͻ��˹ر�*/         
    Err_ClientTimeout    =      -2,  /*�ͻ��˳�ʱ*/    
    Err_WriteBuffOver    =      -3,  /*Tcpд����������*/
    Err_ReadBuffEmpty  =      -4,  /*Tcp��������Ϊ��*/
    Err_PacketError        =      -5, /*�ͻ��˷��͵İ�����*/
};

// tcpsvr�Ϳͻ���ͨѶ��socket�ṹ
struct SocketConnInfo
{
    int socket_fd; // socket���
    ULONG uin; //����˺�
    int recv_bytes; // ���յ��ֽ���
    char recv_buf[RECV_BUF_LEN];  //���յ���client�����   
    ULONG client_ip; // �ͻ���IP��ַ
    time_t  create_Time; // socket�Ĵ���ʱ��
    time_t  recv_data_time; // ���յ����ݰ���ʱ���
};

// tcpsvr��gamesvrͨѶ�İ�ͷ
struct CSPkgHead
{
    int pkg_size; // cs�ܰ���
    int fd; // csͨ��socket
    ULONG client_ip; // �ͻ���IP��ַ,�Ա�gamesvr��Ӫʱ��һЩͳ����
};

// ͳ�Ƽ��ʱ��
const int STAT_TIME = 20;

// ͳ����Ϣ
struct StatInfo
{
    int fd;
    int count;
};

#endif

