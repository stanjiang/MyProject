

#include "ev.h"

#include "common.h"

static void do_accept(struct ev_loop* reactor,ev_io* w,int events)
{
    struct sockaddr_in addr;
    socklen_t addr_size=sizeof(addr);
    int conn=accept(w->fd,(struct sockaddr*)&addr,&addr_size);
    std::string r=common::address_to_string(&addr);
    fprintf(stderr,"accept %s\n",r.c_str());

    int len = 0;
    char buf[32] = {};
    len = recv(conn, buf, 32, 0);

    if(len <= 0)
    {

    }
    else
    {
        cm_printf("buf: %s; len: %d;", buf, len);
        send(conn, buf, len, 0);
        sleep(10);
        send(conn, buf, len, 0);
    }

//    close(conn);
}

int main()
{
    struct ev_loop* reactor=ev_loop_new(EVFLAG_AUTO);
    int fd=common::new_tcp_server(34567);
    ev_io w;
    ev_io_init(&w,do_accept,fd,EV_READ);
    ev_io_start(reactor,&w);
    cm_printf("ev run!\n");
    ev_run(reactor,0);
    cm_printf("close socket %d;\n", fd);
    close(fd);
    ev_loop_destroy(reactor);
}




