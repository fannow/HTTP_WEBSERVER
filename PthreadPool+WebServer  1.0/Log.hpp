#include<iostream>
#include<string>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/time.h>
using namespace std;
#define NOTICE 1
#define WARNING 2
#define ERROR 3
#define FATAL 4




#define Log(level,message) \
log(#level,message,__FILE__/*自动获取调用的文件名*/,__LINE__/*自动获取对应的行号*/)

void log(string leve,string message,string file,int line){
  
  //int s=open("log.txt",O_WRONLY | O_CREAT | O_APPEND);
  
 // dup2(s,1);//将文件log.txt重定向到stdout  cout直接输入到文件中
  struct timeval te;
  gettimeofday(&te,NULL);//获取时间
  //cout<<"["<<leve<<"]["<<message<<"]["<<file<<"]["<<line<<"]["<<te.tv_sec<<"]"<<endl;
//  string str
//close(s);
//  str+="[";
 // str+=leve;
  //str+="][";
 // str+=message;
 // str+="][";
 // str+=file;
// str+="][";
 // str+=line;
 // str+="][";
  //  str+=te.tv_sec;
  //str+="]";
  
 // write(1,str.c_str(),str.size());
}
