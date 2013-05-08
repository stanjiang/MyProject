#include "TcpConnectMgr.h"
#include "MyShmMgr.h"
#include "TcpCode.h"
#include "MsgQueue.h"

using namespace google;

char* CTcpConnectMgr::current_shmptr_;

CTcpConnectMgr* CTcpConnectMgr::CreateInstance(void)
{
    int shm_key = SOCKET_SHM_KEY;
    int shm_size = CountSize();
    int assign_size = shm_size;
    current_shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(shm_key, shm_size, assign_size);

    CTcpConnectMgr* obj = new CTcpConnectMgr();

    return obj;
}

int CTcpConnectMgr::CountSize(void)
{
    int size = sizeof(CTcpConnectMgr);

    return size;
}

void* CTcpConnectMgr::operator new(size_t size)
{
    return (void*)current_shmptr_;
}

void CTcpConnectMgr::operator delete(void* mem)
{
}

int CTcpConnectMgr::Init(void)
{
    errno = 0;
    send_pkg_count_ = 0;
    recv_pkg_count_ = 0;
    laststat_time_ = 0;

    // 初始化服务器监听socket
    if(InitListenSocket(CONNECT_IP, CONNECT_PORT) != 0)
    {
        return -1;
    }    

    // 初始化socket列表数据
    for(int i = 0; i < MAX_SOCKET_NUM; ++i)
    {        
        memset(&client_sockconn_list_[i], 0, sizeof(SocketConnInfo));
        client_sockconn_list_[i].socket_fd = INVALID_SOCKET;
    }

    maxfds_ = listen_fd_+1;
    epoll_timeout_ = 20;
    cur_conn_num_ = 0;

    if(CreateMQ(MQ_CS) != 0)
    {
        return -1;
    }

    if(CreateMQ(MQ_SC) != 0)
    {
        return -1;
    }

    DLOG(INFO) << "init tcpsvr ok! ";
    return 0;
}

void CTcpConnectMgr::Run(void)
{
    while(1)
    {
        if(tcpexit == run_flag_)
        {
            LOG(INFO) << "tcpsvrd exit!";
            return;
        }
        if(reloadcfg == run_flag_)
        {
            LOG(INFO) << "reload tcpsvrd config file ok!";
            run_flag_ = 0;
        }

        // 读取客户端请求
        GetClientMessage();

        // 读取tcpsvr中待发送的数据
        CheckWaitSendData(); 

        // 检测通讯超时的socket
        CheckTimeOut();      
    }
}

int CTcpConnectMgr::InitListenSocket(const char* ip, unsigned short port)
{
    listen_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listen_fd_ < 0)
    {
        LOG(ERROR) << "socket failed, fd=" << listen_fd_ << ", ip=" << ip << ", port=" << port;
        return -1;
    }

    SetNonBlock(listen_fd_);    
    SetSocketOpt(listen_fd_);
    
    struct sockaddr_in addr;
    SetAddress(ip, port, &addr);
    
    int ret = bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr));
    if(ret != 0)
    {
        LOG(ERROR) << "bind socket error, ret=" << ret;
        return -1;
    }

    ret = listen(listen_fd_, LISTEN_BACKLOG); 
    if(ret != 0)
    {
        LOG(ERROR) << "listen socket error, ret=" << ret;
        return -1;
    }
    LOG(INFO) << "create server socket ok, ip=" << ip << "port=" << port << ", fd=" << listen_fd_;

    // 创建服务器epoll句柄
    epoll_fd_ = epoll_create(MAX_SOCKET_NUM);
    if(epoll_fd_ < 0)
    {
        LOG(ERROR) << "create server epoll fd error, fd=" << epoll_fd_ << ", errno=" << errno;
        return -1;    
    }

    // 将服务器侦听socket加入epoll事件中
    struct epoll_event ev;
    ev.events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP;
    ev.data.fd = listen_fd_;

    ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
    if(ret < 0)
    {
        LOG(ERROR) << "mod epoll event error, fd=" << listen_fd_ << ", ret=" << ret << ", err=" << errno << ", errstr=" << strerror(errno);
        return -1;
    }

    LOG(INFO) << "add listen socket to epoll ok, fd=" << listen_fd_;

    return 0;
}

int CTcpConnectMgr::SetNonBlock(int fd)
{
    int flags = 1;
    int ret = ioctl(fd, FIONBIO, &flags);
    if(ret != 0)
    {
        LOG(ERROR) << "ioctl opt error, ret=" << ret;
        return -1;
    }

    flags = fcntl(fd, F_GETFL); 
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret < 0)
    {
        LOG(ERROR) << "fcntl opt error, ret=" << ret;
        return -1;
    }

    return 0;
}


int CTcpConnectMgr::SetSocketOpt(int fd)
{
    // 设置套接字重用
    int reuse_addr_ok = 1;
    setsockopt(fd,SOL_SOCKET, SO_REUSEADDR, &reuse_addr_ok, sizeof(reuse_addr_ok));

    // 设置接收&发送buffer
    int recv_buf = SOCK_RECV_BUFFER;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&recv_buf, sizeof(recv_buf));
    int send_buf = SOCK_SEND_BUFFER;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&send_buf, sizeof(send_buf));

    int flags = 1;
    struct linger ling = {0, 0};
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
    setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    
    return 0;
}

void CTcpConnectMgr::SetAddress(const char* ip, unsigned short port, struct sockaddr_in* addr)
{
    bzero(addr, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(ip);
    addr->sin_port = htons(port);
}

void CTcpConnectMgr::ClearSocketInfo(int fd, ENMSocketErr type)
{
    if(Err_ClientClose == type)
    {
        close(fd);
        DecSockConn();
    }
}

void CTcpConnectMgr::GetClientMessage(void)
{
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));    
    struct epoll_event ev;
    
    int fd_num = epoll_wait(epoll_fd_, events_, MAX_SOCKET_NUM, epoll_timeout_);
    for(int i = 0; i < fd_num; ++i)
    {
        if(events_[i].data.fd <= 0)
        {
            LOG(ERROR) << "invalid socket fd, fd=" << events_[i].data.fd;
            continue;
        }

        if(events_[i].events & EPOLLERR)
        {
            LOG(ERROR) << "socket generate error event";
            ClearSocketInfo(events_[i].data.fd, Err_ClientClose);
            continue;
        }

        // 如果监测到一个SOCKET用户连接到了绑定的SOCKET端口，则建立新的连接
        if(events_[i].data.fd == listen_fd_)
        {
            while(1)
            {
                socklen_t client_len = sizeof(sockaddr_in);
                int conn_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_len);
                if(conn_fd <= 0)
                {                         
                    if(EINTR == errno)
                    {
                        LOG(INFO) << "accept conn continue, fd=" << conn_fd;
                        continue;
                    }

                    if(EAGAIN == errno)
                    {
                        // 当前尚未有新连接上来
                        DLOG(INFO) << "current no client connect accept, fd=" << conn_fd << ", err=" << errno << ", errstring=" << strerror(errno);
                        break;
                    }
                    
                    // 其他情况出错则表示客户端连接上来以后又立即关闭了
                    LOG(INFO) << "accept socket conn error, fd=" << conn_fd << ", err=" << errno << ", errstring=" << strerror(errno);
                    break;
                }
                else
                {
                    IncSockConn();
                    // 检查连接数是否过载
                    if(cur_conn_num_ >= MAX_SOCKET_NUM || conn_fd >= MAX_SOCKET_NUM)
                    {
                        LOG(ERROR) << "accept a invalid conn, fd=" << conn_fd;
                        close(conn_fd);
                        DecSockConn();
                        continue;
                    }

                    if(conn_fd > maxfds_)
                    {
                        maxfds_ = conn_fd;
                    }

                    // 添加新的client连接至epoll事件集合中
                    SetNonBlock(conn_fd);
                    ev.data.fd = conn_fd;
                    ev.events = EPOLLIN|EPOLLET;            
                    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, conn_fd, &ev);
                    if(ret < 0)
                    {
                        LOG(ERROR) << "epoll_ctl add client conn error, ret=" << ret << ", err=" << errno << ", errstring=" << strerror(errno);
                        continue;
                    }

                    // 分配新连接存储空间,以socket为索引
                    client_sockconn_list_[conn_fd].socket_fd = conn_fd;
                    client_sockconn_list_[conn_fd].client_ip = client_addr.sin_addr.s_addr;
                    time(&(client_sockconn_list_[conn_fd].create_Time));

                    client_sockconn_list_[conn_fd].recv_bytes = 0;
                    client_sockconn_list_[conn_fd].recv_data_time = 0;
                    client_sockconn_list_[conn_fd].uin = 0;
                    
                    DLOG(INFO) << "accept a client socket ok, client=" << inet_ntoa(client_addr.sin_addr) 
                        << ", fd=" << conn_fd << ", connnum=" << cur_conn_num_;
                }                    
            }               
        }
        else if(events_[i].events & EPOLLIN)
        {
            int sockfd = events_[i].data.fd;
            if(sockfd < 0)
            {
                LOG(ERROR) << "invalid socket, fd=" << sockfd;
                continue;
            }

            DLOG(INFO) << "process client reading, fd=" << sockfd;
            // 从socket读取client数据,并发送至tcpsvr-->gamesvr的消息队列中
            RecvClientData(sockfd);
        }
    }
}

void CTcpConnectMgr::CheckWaitSendData(void)
{
    for(int i = 0; i < MAX_SEND_PKGNUM; ++i)
    {
        int size = 0;
        int ret = mq_sc_->Pop(send_client_buf_, size);
        if(ret != 0)
        {
            return;
        }

        char* recv_data = &send_client_buf_[0];
        CSPkgHead* head = (CSPkgHead*)recv_data;
        int fd = ntohl(head->fd);
        if(fd > MAX_SOCKET_NUM)
        {
            LOG(ERROR) << "recv invalid client fd, fd=" << fd;
            return;
        }

        ULONG client_ip = ntohl(head->client_ip);
        assert(client_sockconn_list_[fd].socket_fd == fd);
        assert(client_sockconn_list_[fd].client_ip == client_ip);
        
        const char* body = recv_data + sizeof(CSPkgHead);
        int pkg_body_size = CTcpCode::ConvertInt32(body);

        ret = TcpSendData(fd, body, pkg_body_size);
        if(ret != 0)
        {
            LOG(ERROR) << "send to client data error, ret=" << ret << ", fd=" << fd << ", size=" << pkg_body_size;
        }
        else
        {
            DLOG(INFO) << "send to client data ok, ret=" << ret << ", fd=" << fd << ", size=" << pkg_body_size;
            ++send_pkg_count_;
        }
    
    }
}

void CTcpConnectMgr::CheckTimeOut(void)
{
     /***加入统计信息***/
    time_t curtime = time(NULL);
    if(curtime >= laststat_time_)
    {       
        send_pkg_count_ /= STAT_TIME;
        recv_pkg_count_ /= STAT_TIME;
        LOG(INFO) << "process pkg info, sendpkg=" << send_pkg_count_ << ", recvpkg=" << recv_pkg_count_;
        
        send_pkg_count_ = 0;
        recv_pkg_count_ = 0;
        laststat_time_ = curtime + STAT_TIME;
    }

    FLUSH();
}

int CTcpConnectMgr::RecvClientData(int fd)
{
    SocketConnInfo& cur_conn = client_sockconn_list_[fd];
    assert(fd == cur_conn.socket_fd);
    
    // 检查socket合法性    
    if(fd != cur_conn.socket_fd)
    {
        LOG(ERROR) << "invalid socket, fd=" << fd << "cur_fd=" << cur_conn.socket_fd;
        return -1;
    }

    /* 由于采用的是ET触发,故每次read数据时,均把当前socket tcp buf中的数据读取完毕,
        即最大限度地读取数据*/    

    int offset = cur_conn.recv_bytes;
    int len = sizeof(cur_conn.recv_buf) - offset;
    int ret = TcpRecvData(fd, cur_conn.recv_buf + offset, len);
    if(ret == Err_ClientClose)
    {
        LOG(ERROR) << "recv client data error, ret=" << ret;
        //ClearSocketInfo(fd, Err_ClientClose);
        cur_conn.recv_bytes = 0;
        return -1;
    }
    else if(Err_ReadBuffEmpty == ret && 0 == len)
    {
        return 0;
    }

    if(len <= MIN_CSPKG_LEN || len > MAX_CSPKG_LEN)
    {
        LOG(ERROR) << "recv invalid client pkg, fd=" << fd << ", len=" << len;
        return -1;
    }
    
    cur_conn.recv_bytes += len; // 累加当前socket已接收的数据        
    time(&cur_conn.recv_data_time); // 记录接收数据时间
    int cur_pkg_len = CTcpCode::ConvertInt32(cur_conn.recv_buf); // 当前请求包原始长度
    if(cur_pkg_len <= 0)
    {
        LOG(ERROR) << "recv client data ok, fd=" << fd << ", offset=" << offset << ", cur_recv_len=" << len 
            << ", totle_recv_len=" << cur_conn.recv_bytes << ", cur_pkg_len=" << cur_pkg_len;
        return -1;
    }

    DLOG(INFO) << "recv client data ok, fd=" << fd << ", offset=" << offset << ", cur_recv_len=" << len 
        << ", totle_recv_len=" << cur_conn.recv_bytes << ", cur_pkg_len=" << cur_pkg_len;

    // 只有当前已接受的数据大于原始请求包长度时,才作处理,否则继续接受数据
    while(cur_conn.recv_bytes >= cur_pkg_len)
    {
        static char msg_buf[CSPKG_OPT_BUFFSIZE];
        char* msg_ptr = msg_buf;
        CSPkgHead* pkg_head = (CSPkgHead*)msg_ptr;
        pkg_head->pkg_size = htonl(cur_pkg_len + sizeof(CSPkgHead));
        pkg_head->client_ip = htonl(cur_conn.client_ip);
        pkg_head->fd = htonl(fd);

        msg_ptr += sizeof(CSPkgHead);
        memcpy(msg_ptr, cur_conn.recv_buf, cur_pkg_len);

        // 加入了tcpsvr-->gamesvr的消息队列中
        mq_cs_->Push(msg_buf, cur_pkg_len + sizeof(CSPkgHead));

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
            cur_conn.recv_bytes = 0;
            DLOG(INFO) << "current client pkg process over, fd=" << fd;

            ++recv_pkg_count_;
        }
    }
    
    return 0;
}

int CTcpConnectMgr::TcpSendData(int fd, const char* databuf, int len)
{
    int left = len;
    while(left > 0)
    {
        int ret = write(fd, databuf + len - left, left);
        if(ret <= 0)
        {
            // 忽略EINTR 信号
            if(errno == EINTR)
            {
                LOG(INFO) << "recv EINTR signal while send data, fd=" << fd;
                continue;
            }

             // 当socket是非阻塞时,如返回EAGAIN,表示写缓冲队列已满
            if(errno == EAGAIN)
            {
                LOG(INFO) << "tcp send buffer is full, fd=" << fd << ", left=" << left << ", len=" << len;
                //下次继续写
                return Err_WriteBuffOver;  
            }

            LOG(ERROR) << "tcp connect error, errno=" << errno << ", errstr=" << strerror(errno);
            //连接出错,需要关闭连接
            return Err_ClientClose;
        }

        left -= ret;
        DLOG(INFO) << "send data ok, fd=" << fd << ", left=" << left << ", len=" << len;
    }

    DLOG(INFO) << "send data over, fd=" << fd;
    return Err_Succeed;
}

int CTcpConnectMgr::TcpRecvData(int fd, char* databuf, int& len)
{
    int read_len = 0;
    int left = len;
    while(left > 0)
    {
        int ret = read(fd, databuf + len - left, left);
        if(ret <= 0)
        {
            // 忽略EINTR 信号
            if(errno == EINTR)
            {
                LOG(INFO) << "recv EINTR signal while recv data, fd=" << fd;
                continue;
            }

            if(0 == ret)
            {
                //表示client已关闭连接
                len = 0;
                LOG(INFO) << "the client connect has closed, ret=" << ret << ", errno=" << errno 
                    << ", errstr=" << strerror(errno) << ", fd=" << fd;
                return Err_ClientClose;            
            }
            
            // 当socket是非阻塞时,如返回EAGAIN,表示读缓冲队列为空
            if(errno == EAGAIN)
            {
                len = read_len;
                DLOG(INFO) << "tcp recv buffer is empty, ret=" << ret << ", fd=" << fd << ", left=" << left 
                    << ", len=" << len << ", readlen=" << read_len;
                // 下次继续读
                return Err_ReadBuffEmpty;
            }

            // 其它出错
            len = 0;
            LOG(ERROR) << "tcp connect error, ret=" << ret << ", errno=" << errno
                << ", errstr=" << strerror(errno) << ", fd=" << fd;
            return Err_ClientClose;
        }

        left -= ret;
        read_len += ret;
        DLOG(INFO) << "recv data ok, fd=" << fd << ", left=" << left << ", len=" << len;
    }

    DLOG(INFO) << "recv data over, fd=" << fd;
    len = read_len;
    return Err_Succeed;
}

int CTcpConnectMgr::CreateMQ(MQType type)
{
    char* shmptr_ = NULL;
    if(MQ_CS == type)
    {
        shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(CS_MQ_SHMKEY, MQ_SIZE, MQ_SIZE);
        CMsgQueue::InitShmAttr(shmptr_);
        mq_cs_ = new CMsgQueue;
        if(NULL == mq_cs_)
        {
            LOG(ERROR) << "new cs msg queue error, key=" << CS_MQ_SHMKEY << ", size=" << MQ_SIZE;
            return -1;
        }
        ENMShmMode mode = CMyShmMgr::Instance().GetShmMode(CS_MQ_SHMKEY);
        mq_cs_->Init(MQ_SIZE, mode);
        LOG(INFO) << "create cs mq ok, key=" << CS_MQ_SHMKEY << ", size=" << MQ_SIZE;
    }
    else if(MQ_SC == type)
    {
        shmptr_ = (char*)CMyShmMgr::Instance().CreateShm(SC_MQ_SHMKEY, MQ_SIZE, MQ_SIZE);
        CMsgQueue::InitShmAttr(shmptr_);
        mq_sc_ = new CMsgQueue;
        if(NULL == mq_sc_)
        {
            LOG(ERROR) << "new sc msg queue error, key=" << SC_MQ_SHMKEY << ", size=" << MQ_SIZE;
            return -1;
        }
        ENMShmMode mode = CMyShmMgr::Instance().GetShmMode(SC_MQ_SHMKEY);
        mq_sc_->Init(MQ_SIZE, mode);
        LOG(INFO) << "create sc mq ok, key=" << SC_MQ_SHMKEY << ", size=" << MQ_SIZE;
    }

    return 0;
}


