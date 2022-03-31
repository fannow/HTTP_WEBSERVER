#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<queue>
#include"Potoal.hpp"
using namespace std;
typedef void (*handler_t)(int);
class Tesk {
  int sock;
  handler_t handler;
  public:
  Tesk(){}
  void Run(){
    handler(sock);
  }
  Tesk(int sock_):sock(sock_),handler(Enpty::EnptyHttp)
  {}
};
class PthreadPool {
private:
	queue<Tesk*> qt;
	int num; //线程池数量
  pthread_mutex_t lock;
  pthread_cond_t cond;
public:
	PthreadPool(int _num) :
		num(_num)
	{}
	bool Empty() {
		return qt.size() == 0;
	}
  void Lock(){
  pthread_mutex_lock(&lock);
  }
  void Unlock(){
    pthread_mutex_unlock(&lock);
  }
  void Cond(){
    pthread_cond_wait(&cond,&lock);
  }
  void Uncond(){
    pthread_cond_signal(&cond);
  }
	static void* Runthread(void *rid) {
		PthreadPool* pp = (PthreadPool*)rid;
    
      //临界资源加锁
      pp->Lock();
	    	while (pp->Empty()) {
	          //任务队列为空直接阻塞自己
            pp->Cond();
	    	}
    Tesk *t=pp->Out();//获取任务队列中的任务
    //  pp->Out(*t);//提取任务；
     // t->Run();
      pp->Unlock();
      t->Run();
      delete  t;
  }
	void Init() {

     pthread_mutex_init(&lock,NULL); 
     pthread_cond_init(&cond,NULL);
		pthread_t tid;
		for (int i = 0;i < num;i++) {
			pthread_create(&tid, NULL, Runthread, this);
			pthread_detach(tid);

		}
	}
  void push(Tesk &in){
    //临界资源直接加锁
    Lock();

    qt.push(&in);
    Unlock();
    //任务队列中由任务了直接唤醒来做任务的线程
    Uncond();
  }
  Tesk* Out(){
    Tesk *t=qt.front();//获取任务队列中的任务
    
     //将去走的任务删除
     qt.pop();
  return t;
  }
  ~PthreadPool(){
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
  }
};
