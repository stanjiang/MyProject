




#include <errno.h>
#include <string.h>


#include "ev.h"
#include "common.h"



static void do_connected(struct ev_loop* reactor,ev_io* w,int events)
{
//    close(w->fd);
    if(events & EV_WRITE)
    {
        cm_printf("ev write, fd: %d;", w->fd);
        int send_len = 0;
        char buf[32] = "nihao";
        int len = strlen(buf);

        send_len = send(w->fd, buf, len, 0);
        if(send_len == -1)
        {
            perror("echo send");
            cm_printf("echo send errno: %d, %s", errno, strerror(errno));
            //to do ???
            return;
        }
        if(send_len != len)
        {
            cm_printf("send len is not equal: %d, %d", send_len, len);
            //to do ??
        }

        ev_io_stop(reactor, w);
        ev_io_set(w, w->fd, EV_READ);
        ev_io_start(reactor, w);

    }
    else if(events & EV_READ)
    {
        cm_printf("ev read, fd: %d;", w->fd);
        int len = 0;
        char buf[32] = {};
        len = recv(w->fd, buf, 32, 0);
        if(len <= 0)
        {
            perror("echo recv");
            cm_printf("len: %d; errno: %d, %s;", len, errno, strerror(errno));
            ev_break(reactor,EVBREAK_ALL);
        }
        else
        {
            cm_printf("buf: %s; len: %d;", buf, len);
        }

    }

//    ev_break(reactor,EVBREAK_ALL);
}

int main()
{
    struct ev_loop* reactor=ev_loop_new(EVFLAG_AUTO);
//    int fd=common::new_tcp_client("127.0.0.1",34567);
    int fd=common::new_tcp_client("192.168.0.253",34567);
    cm_printf("socket %d;\n", fd);
    ev_io io;
    ev_io_init(&io,&do_connected,fd,EV_WRITE);
    ev_io_start(reactor,&io);
    cm_printf("ev run begin!\n");
    ev_run(reactor,0);
    cm_printf("close socket %d;\n", fd);
    close(fd);
    ev_loop_destroy(reactor);
    return 0;
}













