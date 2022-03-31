#include<iostream>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
#include<stdlib.h>
#include<sys/epoll.h>
#include<sys/types.h>
using namespace std;
struct bucket {
    char buff[10];
    int fd;
    int num;//上一次读取的数据长度
    bucket(int fd_) :fd(fd_), num(0)
    {
        memset(buff, 0, sizeof(buff));//清空
    }
    bucket() {

        memset(buff, 0, sizeof(buff));//清空
    }

    //memset(buff,0,sizeof(buff));//清空


};
class EpollServer {
private:
    int lsock;
    int post;
    int mepoll;

public:
    EpollServer(int _post) {
        post = _post;
    }
    void Init() {
        lsock = socket(AF_INET, SOCK_STREAM, 0);
        if (lsock < 0) {
            cout << "socket error" << endl;
            exit(1);
        }
        struct sockaddr_in in_addr;
        in_addr.sin_family = AF_INET;
        in_addr.sin_port = htons(post);
        in_addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(lsock, (struct sockaddr*)&in_addr, sizeof(in_addr)) < 0) {
            cout << "bind error" << endl;
            exit(0);
        }
        if (listen(lsock, 5) < 0) {
            cout << "listen error" << endl;
            exit(4);
        }
        mepoll = epoll_create(128);
        if (mepoll < 0) {
            cout << "epoll_create error" << endl;
            exit(3);
        }
    }
    void Addevents(int sock, uint32_t opov) {
        //添加套接字到epoll模型
        struct epoll_event event;
        //添加事件
        event.events = opov;
        //监听套接字不需要data缓冲区
        if (sock == lsock) {
            event.data.ptr = NULL;
        }
        else {
            //给sock添加buff缓冲区
            //添加文件描述符到bucket
            event.data.ptr = new struct bucket(sock);
        }
        epoll_ctl(mepoll, EPOLL_CTL_ADD, sock, &event);
    }
    void start() {
        Addevents(lsock, EPOLLIN);//服务器启动时先将监听套接字对应的事件添加到要检测的事件集中建立链接时读事件所以添加事件为EPOLLIN
        while (1) {
            struct epoll_event events[1024];//返回的就绪事件集
            //不用担心就绪事件集不够用，这一次没有全部装下，剩下的下次会一起处理
            //就绪事件会从下标为0开始添加
            int timeout = 2000;
            int num = epoll_wait(mepoll, events, 1024, timeout);
            //返回值是已经就绪的事件的个数

            switch (num) {
            case 0:
                cout << "等待超时" << endl;
                break;
            case -1:
                cout << "等待出错" << endl;
                break;
            default:
                //执行事件
                cout << "等待成功" << endl;
                service(events, num);
                break;
            }
        }

    }
    void service(struct epoll_event events[], int num) {
        for (int i = 0;i < num;i++) {
            uint32_t ev = events[i].events;//获取就绪事件
            if (ev & EPOLLIN) {
                //读事件就绪
                //因为链接也是读事件所以也要检测是不是监听套接字
                //因为监听套接字的ptr设置为NILL了
                if (events[i].data.ptr == NULL) {
                    //链接事件
                    struct sockaddr_in addr;
                    socklen_t len = sizeof(addr);

                    int sock = accept(lsock, (struct sockaddr*)&addr, &len);
                    if (sock > 0) {
                        cout << "get a new link......." << endl;
                        Addevents(sock, EPOLLIN);

                    }

                }
                else if (events[i].data.ptr != NULL) {
                    //普通事件
                    //拿到这个事件对应的bucket
                    bucket* b = (bucket*)events[i].data.ptr;
                    //自上一次recv读取的缓冲区添加数据后再recv拷贝数据保证数据完整
                    int s = recv(b->fd, b->buff + b->num, sizeof(b->buff) - b->num/*总容量减去上一次拷贝到缓冲区的数据长度 */, 0);
                    if (s > 0) {
                        b->num += s;//保存这次读取数据的长度下一次从这开始添加

                        cout << "client" << b->buff << endl;
                        //打印的buff中数据会越来越多
                        if (b->num >= sizeof(b->buff)) {
                            //假如已经拿到完整定长requst
                            //就要发送response
                            //修改读事件为写事件
                            struct epoll_event event;
                            event.data.ptr = events[i].data.ptr;
                            event.events = EPOLLOUT;
                            epoll_ctl(mepoll, EPOLL_CTL_MOD, b->fd, &event);
                        }
                    }
                    else if (s == 0) {
                        close(b->fd);
                        epoll_ctl(mepoll, EPOLL_CTL_DEL, b->fd, NULL);//从epol模型l删除关闭的文件描述符号
                        cout << "client have quited" << endl;
                        //将对应的bucket缓冲区释放
                        delete b;
                    }

                }
            }
            else if (ev & EPOLLOUT) {
                //写事件就绪

                bucket* b = (bucket*)events[i].data.ptr;
                int s = send(b->fd, b->buff, sizeof(b->buff), 0);



                close(b->fd);
                epoll_ctl(mepoll, EPOLL_CTL_DEL, b->fd, NULL);//从epol模型l删除关闭的文件描述符号
                delete b;


            }
            else {
                //其他事件
            }

        }
    }
    ~EpollServer() {
        close(lsock);
        close(mepoll);//关闭epoll模型
    }
};
void openov(string op) {
    cout << op << endl;
}
int main(int argc, char* argv[]) {
    if (argc != 2) {
        openov(argv[0]);
        exit(0);
    }
    EpollServer* es = new EpollServer(atoi(argv[1]));
    es->Init();
    es->start();
    return 0;
}
