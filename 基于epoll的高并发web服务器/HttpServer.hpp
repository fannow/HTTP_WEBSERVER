#include"Potoal.hpp"

//缓冲区结构体
struct  buter {
    int sock;
    EHttp* eh;//事务类处理request和包装response;
   
    buter() {
  

    }
    buter(int sock_):sock(sock_),eh(NULL) {
   

    }

};
class HttpServer{
private:
  int port;
  int lsock;
  int mepoll;



  static HttpServer*httpserver;
  static pthread_mutex_t lock;
  HttpServer(int _port):port(_port),lsock(-1)
  {}
public:
  
  void Init(){
    
 
    lsock=Sock::SocKet();
    Sock::SetSockOpt(lsock);//端口复用
    Sock::Bind(lsock,port);
    Sock::Listen(lsock);
   //创建epoll模型
   mepoll =epoll_create(128);
   if(mepoll<0){
     cout<<"epoll创建失败"<<endl;
     exit(5);
   }
  }
  //单例模式
  static HttpServer*GetInstance(int sk){
    if(httpserver==NULL){
     pthread_mutex_lock(&lock);
     if(NULL==httpserver){
     httpserver=new HttpServer(sk);
     pthread_mutex_unlock(&lock);

     }
    }
    return httpserver;
  }

  void Start(){

      //将监听套接字对应的写事件添加到对应的epoll模型中
     AddEpoll(lsock,EPOLLIN);
     while(1){
         struct epoll_event eve[100];//就绪事件集合,有epoll_create返回就绪事件集合
         int num = epoll_wait(mepoll, eve, 100, 0);
         switch (num) {
                case 0:
 //                   cout << "等待超时" << endl;
                   // sleep(2);
                    break;
                case -1:
                    cout << "等待出错" << endl;
                    break;
                default:
              
                    Service(eve, num); 
         }
   
     }
  }

  void Service(struct epoll_event event[], int num) {
        cout<<"循环取出就绪事件"<<endl;
      for (int i = 0;i < num;i++) {
          uint32_t en = event[i].events;
              struct buter* bt= (struct buter*)event[i].data.ptr;
              //EHttp* eh = bt->eh;
         
              //事件的recv和senfile
              if (en & EPOLLIN) {
                  if (event[i].data.ptr == NULL) {
                      //链接事件就绪建立连接
                          //建立连接事件
                        //建立连接也是写时建
                      //首先判断是不是建立连接事件

                //建立连接事件的缓冲区被设置为NULL直接这样这样判断
                cout<<"建立连接"<<endl;
                      int sock = Sock::Accept(lsock);

                      //将新建套接字以写时建添加到epoll模型中
                      AddEpoll(sock, EPOLLIN);

                  }
                  else if (event[i].data.ptr != NULL) {
                       
                         //普通读事件
              EHttp* eh = bt->eh;
                       //request的收取
                        //传入epoll模型如果读取失败就把该套接字从mepoll删除
                      eh->Getrequest(&mepoll);
                      //事件读取完就需要将读事件改为写事件
                      struct epoll_event event1;
                      event1.data.ptr = event[i].data.ptr;
                      event1.events = EPOLLOUT;
                      epoll_ctl(mepoll, EPOLL_CTL_MOD, bt->sock, &event1);
                      cout << "读事件完成" << endl;
                  }
              }
              else if (en & EPOLLOUT) {
              EHttp* eh = bt->eh;
                        cout<<"写事件"<<endl;
                     //response的制作  
                     //response发送response;
                     //sendfine发送文件
                  eh->MakeResponse();
                  cout<<"响应制作完成"<<endl;
                  eh->SendREquest();
                  close(bt->sock);
                  //删除文件描述符
                  epoll_ctl(mepoll, EPOLL_CTL_DEL, bt->sock, NULL);


              }
              else {
              //其他事件
              
              }

          }
          
  
  }

  //将套接字添加到epoll模型中
void  AddEpoll(int sock,uint32_t opev) {
    //套接字对应的事件添加到epoll模型中
      struct epoll_event   event;
      //设置是按对应的字段；
      event.events = opev;

      //监听套接字只需要负责连接事件所以不需要缓冲区将缓冲区设置为null
      if (sock == lsock) {
          event.data.ptr = NULL;
      
      }
      else {
        //其他套接字设置缓冲区
          struct buter *bt= new struct buter(sock);
          bt->eh = new EHttp(sock);
          event.data.ptr = bt;
            

          
      
      }
      epoll_ctl(mepoll, EPOLL_CTL_ADD, sock, &event);
  }
  ~HttpServer(){
    if(lsock>-1){
      close(lsock);
    }
    close(mepoll);
  }

};
HttpServer*HttpServer::httpserver=NULL;
pthread_mutex_t HttpServer::lock=PTHREAD_MUTEX_INITIALIZER;

