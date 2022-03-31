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
    int num;//��һ�ζ�ȡ�����ݳ���
    bucket(int fd_) :fd(fd_), num(0)
    {
        memset(buff, 0, sizeof(buff));//���
    }
    bucket() {

        memset(buff, 0, sizeof(buff));//���
    }

    //memset(buff,0,sizeof(buff));//���


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
        //����׽��ֵ�epollģ��
        struct epoll_event event;
        //����¼�
        event.events = opov;
        //�����׽��ֲ���Ҫdata������
        if (sock == lsock) {
            event.data.ptr = NULL;
        }
        else {
            //��sock���buff������
            //����ļ���������bucket
            event.data.ptr = new struct bucket(sock);
        }
        epoll_ctl(mepoll, EPOLL_CTL_ADD, sock, &event);
    }
    void start() {
        Addevents(lsock, EPOLLIN);//����������ʱ�Ƚ������׽��ֶ�Ӧ���¼���ӵ�Ҫ�����¼����н�������ʱ���¼���������¼�ΪEPOLLIN
        while (1) {
            struct epoll_event events[1024];//���صľ����¼���
            //���õ��ľ����¼��������ã���һ��û��ȫ��װ�£�ʣ�µ��´λ�һ����
            //�����¼�����±�Ϊ0��ʼ���
            int timeout = 2000;
            int num = epoll_wait(mepoll, events, 1024, timeout);
            //����ֵ���Ѿ��������¼��ĸ���

            switch (num) {
            case 0:
                cout << "�ȴ���ʱ" << endl;
                break;
            case -1:
                cout << "�ȴ�����" << endl;
                break;
            default:
                //ִ���¼�
                cout << "�ȴ��ɹ�" << endl;
                service(events, num);
                break;
            }
        }

    }
    void service(struct epoll_event events[], int num) {
        for (int i = 0;i < num;i++) {
            uint32_t ev = events[i].events;//��ȡ�����¼�
            if (ev & EPOLLIN) {
                //���¼�����
                //��Ϊ����Ҳ�Ƕ��¼�����ҲҪ����ǲ��Ǽ����׽���
                //��Ϊ�����׽��ֵ�ptr����ΪNILL��
                if (events[i].data.ptr == NULL) {
                    //�����¼�
                    struct sockaddr_in addr;
                    socklen_t len = sizeof(addr);

                    int sock = accept(lsock, (struct sockaddr*)&addr, &len);
                    if (sock > 0) {
                        cout << "get a new link......." << endl;
                        Addevents(sock, EPOLLIN);

                    }

                }
                else if (events[i].data.ptr != NULL) {
                    //��ͨ�¼�
                    //�õ�����¼���Ӧ��bucket
                    bucket* b = (bucket*)events[i].data.ptr;
                    //����һ��recv��ȡ�Ļ�����������ݺ���recv�������ݱ�֤��������
                    int s = recv(b->fd, b->buff + b->num, sizeof(b->buff) - b->num/*��������ȥ��һ�ο����������������ݳ��� */, 0);
                    if (s > 0) {
                        b->num += s;//������ζ�ȡ���ݵĳ�����һ�δ��⿪ʼ���

                        cout << "client" << b->buff << endl;
                        //��ӡ��buff�����ݻ�Խ��Խ��
                        if (b->num >= sizeof(b->buff)) {
                            //�����Ѿ��õ���������requst
                            //��Ҫ����response
                            //�޸Ķ��¼�Ϊд�¼�
                            struct epoll_event event;
                            event.data.ptr = events[i].data.ptr;
                            event.events = EPOLLOUT;
                            epoll_ctl(mepoll, EPOLL_CTL_MOD, b->fd, &event);
                        }
                    }
                    else if (s == 0) {
                        close(b->fd);
                        epoll_ctl(mepoll, EPOLL_CTL_DEL, b->fd, NULL);//��epolģ��lɾ���رյ��ļ���������
                        cout << "client have quited" << endl;
                        //����Ӧ��bucket�������ͷ�
                        delete b;
                    }

                }
            }
            else if (ev & EPOLLOUT) {
                //д�¼�����

                bucket* b = (bucket*)events[i].data.ptr;
                int s = send(b->fd, b->buff, sizeof(b->buff), 0);



                close(b->fd);
                epoll_ctl(mepoll, EPOLL_CTL_DEL, b->fd, NULL);//��epolģ��lɾ���رյ��ļ���������
                delete b;


            }
            else {
                //�����¼�
            }

        }
    }
    ~EpollServer() {
        close(lsock);
        close(mepoll);//�ر�epollģ��
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
