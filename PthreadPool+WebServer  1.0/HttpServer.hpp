#include"PthreadPoo.hpp"
class HttpServer{
private:
  int port;
  int lsock;
  PthreadPool* pp;
  static HttpServer*httpserver;
  static pthread_mutex_t lock;
  HttpServer(int _port):port(_port),lsock(-1),pp(NULL)
  {}
public:
  //HttpServer(int _port):port(_port),lsock(-1),pp(NULL)
 // {}
  void Init(){
    pp=new PthreadPool(10);
    lsock=Sock::SocKet();
    Sock::SetSockOpt(lsock);//端口复用
    Sock::Bind(lsock,port);
    Sock::Listen(lsock);
    pp->Init();
  //  cout<<"线程初始化"<<endl;
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
  //daemon(1,1);
     while(1){
      int sock=Sock::Accept(lsock);
      if(sock<0){
          continue;
      }
   //   cout<<"get a new link"<<endl;
        Tesk *t=new Tesk(sock);
        pp->push(*t);

      //demo
     // pthread_t tid;
      //int *newsock=new int(sock);//多线程之间资源共享防止一个改变sock，导致其他的线程sock该百年
     // pthread_create(&tid,NULL,Enpty::EnptyHttp,newsock);
     // pthread_detach(tid);
     }
  }
  ~HttpServer(){
    if(lsock>-1){
      close(lsock);
    }
  }

};
HttpServer*HttpServer::httpserver=NULL;
pthread_mutex_t HttpServer::lock=PTHREAD_MUTEX_INITIALIZER;

