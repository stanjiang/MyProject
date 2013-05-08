#include "TcpClient.h"
#include "TcpConnectMgr.h"
#include "TcpCode.h"
#include "MyShmMgr.h"

using namespace google;

// 日志文件名
const char* LOGFILE_INFO = "../log/tcpclient_info.log";
const char* LOGFILE_ERROR = "../log/tcpclient_error.log";

char* CTcpClient::current_shmptr_;

CTcpClient* CTcpClient::CreateInstance(void)
{
    int shm_key = CLIENT_SHM_KEY;
    int shm_size = sizeof(CTcpClient);
    int assign_size = shm_size;
    current_shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(shm_key, shm_size, assign_size);

    CTcpClient* obj = new CTcpClient();
    return obj;
}

void* CTcpClient::operator new(size_t size)
{
    return (void*)current_shmptr_;
}

void CTcpClient::operator delete(void* mem)
{
}

int CTcpClient::Init(const char* ip, unsigned short port)
{
    memset(&client_conn_, 0, sizeof(client_conn_));
    memset(&client_opt_, 0, sizeof(client_opt_));
    client_epoll_fd_ = 0;
    maxfd_ = 0;
    cur_conn_num_ = 0;
    memset(&client_events_, 0, sizeof(client_events_));    
    send_pkg_count_ = 0;
    recv_pkg_count_ = 0;
    laststat_time_ = 0;

    return CreateSocket(ip, port);
}

void CTcpClient::Run(void)
{
    for(; ;)
    {
        PrepareIOEvent();
        TickHandle();
    }

}

int CTcpClient::CreateSocket(const char* ip, unsigned short port)
{
    client_epoll_fd_ = epoll_create(MAX_CLIENT_CONN);
    if(client_epoll_fd_ < 0)
    {
        LOG(ERROR) << "create client epoll fd error, fd=" << client_epoll_fd_ << ", errno=" << errno;
        return -1;
    }
    
    for(int i = 0; i < MAX_CLIENT_CONN; ++i)
    {
        client_conn_[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(client_conn_[i] < 0)
        {
            LOG(ERROR) << "socket failed, fd=" << client_conn_[i] << ", ip="  << ip << ", port=" << port;
            return -1;
        }

        if(client_conn_[i] > maxfd_)
        {
            maxfd_ = client_conn_[i];
        }

        if(maxfd_ > MAX_CLIENT_CONN)
        {
            LOG(ERROR) << "beyond max fd, maxfd=" << maxfd_;
            break;
        }

        CTcpConnectMgr::SetNonBlock(client_conn_[i]);    
        CTcpConnectMgr::SetSocketOpt(client_conn_[i]);

        struct sockaddr_in addr;
        CTcpConnectMgr::SetAddress(ip, port, &addr);

        errno = 0;
        int ret = connect(client_conn_[i], (struct sockaddr*)(&addr), sizeof(addr));
        if(ret != 0)
        {
            if(errno != EINPROGRESS)
            {
                LOG(ERROR) << "connect server error, ret=" << ret << ", ip=" << ip << ", port=" << port 
                    << ", errno=" << errno << ", errstr=" << strerror(errno);
                return -1;
            }
        }

        // 在当前连接中注册epoll事件和连接fd
        struct epoll_event ev;
        ev.events = EPOLLIN|EPOLLOUT|EPOLLET;
        ev.data.fd = client_conn_[i];

        ret = epoll_ctl(client_epoll_fd_, EPOLL_CTL_ADD, client_conn_[i], &ev);
        if(ret < 0)
        {
            LOG(ERROR) << "mod epoll event error, ret=" << ret << ", err=" << errno << ", errstr=" << strerror(errno);
            return -1;
        }

        ++cur_conn_num_;        
        DLOG(INFO) << "init client socket ok, fd=" << client_conn_[i] << ", connnum=" << cur_conn_num_;

    }
   
    return 0;
}


void CTcpClient::PrepareIOEvent(void)
{
    int fd_num = epoll_wait(client_epoll_fd_, client_events_, MAX_CLIENT_CONN, 50);
    for(int i = 0; i < fd_num; ++i)
    {
        int fd = client_events_[i].data.fd;
        client_opt_[fd].fd = fd;
               
        // 若有可写fd,表明连接已经正常建立,可以发送数据了
        if(client_events_[i].events & EPOLLOUT)
        {
            if(fd <= 0 || fd > MAX_CLIENT_CONN)
            {
                continue;
            }
            client_opt_[fd].send_flag = true;
        }

        if(client_events_[i].events & EPOLLIN)
        {
            if(fd <= 0 || fd > MAX_CLIENT_CONN)
            {
                continue;
            }

            // 可以接收数据了
            client_opt_[fd].recv_flag = true;            
        }            
    }
}

void CTcpClient::TickHandle(void)
{
    SendDataToServera();

    int count = 2;
    for(int i = 0; i < count; ++i)
    {
        RecvDataFromServer();
    }

    time_t curtime = time(NULL);
    if(curtime >= laststat_time_)
    {
        LOG(INFO) << "send pkg=" << send_pkg_count_/STAT_TIME << ", recv pkg=" << recv_pkg_count_/STAT_TIME;
        send_pkg_count_ = 0;
        recv_pkg_count_ = 0;
        laststat_time_ = curtime + STAT_TIME;
    }

    usleep(1000*100);
}

void CTcpClient::SendDataToServera(void)
{
    for(int i = 0; i < MAX_CLIENT_CONN; ++i)
    {
        if(client_opt_[i].send_flag)
        {
            int fd = client_opt_[i].fd;           
            assert(client_opt_[fd].fd == fd);

            int uin = (i+1)*10000;
            client_opt_[fd].uin = uin;
                
            int ret = SendAccountLoginReq(fd, uin);
            if(0 == ret)
            {
                ++send_pkg_count_;
                DLOG(INFO) << "send pkg ok, fd=" << fd << ", uin=" << uin;
            }
            else
            {
                LOG(ERROR) << "send account req error, ret=" << ret;
            }
        }
    }
}

void CTcpClient::RecvDataFromServer(void)
{
    for(int i = 0; i < MAX_CLIENT_CONN; ++i)
    {
        if(client_opt_[i].recv_flag)
        {
            int fd = client_opt_[i].fd;
            if(0 == fd)
            {
                continue;
            }
            
            ClientOptInfo& cur_conn = client_opt_[fd];
            assert(fd == cur_conn.fd);

            DLOG(INFO) << "recv data from server, fd=" << fd << ", uin=" << cur_conn.uin;

            int offset = cur_conn.recv_bytes;
            int len = sizeof(cur_conn.recv_buf) - offset;
            int ret = CTcpConnectMgr::TcpRecvData(fd, cur_conn.recv_buf + offset, len);
            if(Err_ClientClose == ret)
            {
                DLOG(ERROR) << "recv server data error, ret=" << ret;                
                cur_conn.recv_bytes = 0;
                //close(fd);
                continue;
            }
            else if(Err_ReadBuffEmpty == ret && 0 == len)
            {
                continue;
            }

            if(len <= MIN_CSPKG_LEN || len >= MAX_CSPKG_LEN)
            {
                LOG(ERROR) << "recv invalid server pkg, fd=" << fd << ", len=" << len << ", ret=" << ret;
                cur_conn.recv_bytes = 0;
                continue;
            }
            
            cur_conn.recv_bytes += len; // 累加当前socket已接收的数据        
            int cur_pkg_len = CTcpCode::ConvertInt32(cur_conn.recv_buf); // 当前请求包原始长度

            // 只有当前已接受的数据大于原始请求包长度时,才作处理,否则继续接受数据
            while(cur_conn.recv_bytes >= cur_pkg_len)
            {
                // 检查本次接受的数据是否还有剩余数据还没有处理
                int recv_restdata_len = cur_conn.recv_bytes - cur_pkg_len;
                if(recv_restdata_len > 0)
                {
                    memmove(cur_conn.recv_buf, cur_conn.recv_buf + cur_pkg_len, recv_restdata_len);
                    cur_conn.recv_bytes -= cur_pkg_len;
                    cur_pkg_len = CTcpCode::ConvertInt32(cur_conn.recv_buf); // 下一个请求包原始长度
                    
                    DLOG(INFO) << "there are rest data to been process after, restlen=" << recv_restdata_len 
                        << ", fd=" << fd << "next pkg len=" << cur_pkg_len;
                }
                else
                {
                    RecvAccountLoginRes(fd, cur_conn.recv_buf, cur_pkg_len);
                    cur_conn.recv_bytes = 0;
                    ++recv_pkg_count_;
                }
            }            
        }
    }    
}

void CTcpClient::PrintMsg(const std::string buf)
{
    DLOG(INFO) << "encoded to bytes, size=" << buf.size();
    for (size_t i = 0; i < buf.size(); ++i)
    {
        DLOG(INFO) << i << ": " << buf[i] << "\n";
    }
}

int CTcpClient::SendAccountLoginReq(int fd, unsigned int uin)
{
    cspkg::AccountLoginReq acc_login_req;
    acc_login_req.set_account(uin);
    static char session[128];
    snprintf(session, sizeof(session), "Dream what you want to dream; go where you want to go; be what you want to be; \
        because you have only one life and one chance to do all the things you want to do. ");
    acc_login_req.set_session_key(session);

    std::string pkg = CTcpCode::Encode(acc_login_req);
    if(pkg.size() == 0)
    {
        return -1;
    }
    
    int pkg_head = 0;
    std::copy(pkg.begin(), pkg.begin() + sizeof(pkg_head), reinterpret_cast<char*>(&pkg_head));
    int len = ::ntohl(pkg_head);
    assert(len == pkg.size());

    std::string buf = pkg.substr(PKGHEAD_FIELD_SIZE);
    assert(len == buf.size()+PKGHEAD_FIELD_SIZE);

    DLOG(INFO) << "encode pkg ok, len=" << len << ", fd=" << fd << ", uin=" << uin;
    
    return CTcpConnectMgr::TcpSendData(fd, pkg.c_str(), pkg.size());
}


int CTcpClient::RecvAccountLoginRes(int fd, const char* pkg, int len)
{
    std::string buf;
    buf.assign(pkg, len);
    google::protobuf::Message* msg = CTcpCode::Decode(buf);
    assert(NULL != msg);
    cspkg::AccountLoginRes* acc_login_res = dynamic_cast<cspkg::AccountLoginRes*>(msg);
    assert(acc_login_res != NULL);
    assert(acc_login_res->account() == client_opt_[fd].uin);
    
    
    DLOG(INFO) << "recv accout login res to process over, fd=" << fd;
                    
    return 0;
}

void CTcpClient::Destroy(void)
{
    for(int i = 0; i < MAX_CLIENT_CONN; ++i)
    {
        close(client_conn_[i]);
    }
}

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, LOGFILE_INFO);
    google::SetLogDestination(google::ERROR, LOGFILE_ERROR);
//    google::SetStderrLogging(google::ERROR); // 错误日志同时输出到console
    
//    google::FlushLogFiles(ERROR);

    CTcpClient* client = CTcpClient::CreateInstance();
    if(NULL == client)
    {
        printf("create client inst error!\n");
        return -1;
    }
    
    int ret = client->Init(CONNECT_IP, CONNECT_PORT);
    if(ret != 0)
    {
        LOG(ERROR) << "init client error";
        Trace("init client error!");
        return -1;
    }

    client->Run();

    google::ShutdownGoogleLogging();
    
    return 0;
}


