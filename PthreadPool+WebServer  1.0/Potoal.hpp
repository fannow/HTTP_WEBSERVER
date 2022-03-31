#include"Sock.hpp"
#include<algorithm>
#include<cstring>
#include<sys/stat.h>
#include<vector>
#include<unistd.h>
#include<unordered_map>
#include<sys/sendfile.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/wait.h>
#include"Until.hpp"
#define WEBROOT "webroot"
#define HOMEPATH "index.html"
#define VERSION "HTTP/1.0"
class Httprequest{
private:
  string request_line;//请求行
  vector<string>request_headr;//请求头
  string request_space;//请求空行
  string request_body;//请求体
private:
  string method;//请求方法
  string uri;
  string version;//http版本号



  //key:value形式存放请求报头
  unordered_map<string,string> headrkv;

  //请求体长度
  ssize_t content_length;
  string path;//资源存放路径
  string query_string;//存放的是浏览器发给server发的参数


  bool cgi;//标记可执行文件
  ssize_t path_file_size;
  string suff;//请求文件后缀
public:
    Httprequest():request_space("\n"),content_length(-1),path(WEBROOT) \
                  ,cgi(false),suff("text/html")
  {}
    ssize_t GetContentLeng(){
      return content_length;
    }
    void MakeSuff(){
       size_t pos=path.rfind('.');

     if(pos!=string::npos){
        suff=path.substr(pos+1);
      }
    }
    string GetSuff(){
      return suff;
    }
  void SetRequestLine(string &line){
    request_line=line;
  }
  void  SetFileSize(ssize_t size){
    path_file_size=size;
  }
  ssize_t GetFileSize(){
    return path_file_size;
  }
  string GetMethod(){
    return method;
  }
  void RequestLineParse(){
    Util::StringParse(request_line,method,uri,version);//请求行分离
    Log(NOTICE,request_line);
    Log(NOTICE,method);
    Log(NOTICE,uri);
    Log(NOTICE,version);
  }
  void InsertHeadrLine(string &line){
  request_headr.push_back(line);
  Log(NOTICE,line);
  }
  void RequestHeadrParse(){

    for(auto &it:request_headr){
      string k,v;
   //   cout<<k<<":"<<v<<endl;
      Util::MakeStringKv(it,k,v);
      headrkv.insert({k,v});
      if(strcasecmp(k.c_str(),"Content-Length")==0){
        content_length=atoi(v.c_str());
   //     cout<<"headparse2"<<content_length<<endl;
      }
  //    cout<<k<<":"<<v<<endl;
    }
  }
  bool IsNeedRecvHeadr(){
    //post POST Post ->统一大小写
    //统一大写
   // transform(method.begin(),method.end(),method.begin(),::toupper);
    //strcaseacmp(method.c_str,"POST");忽略大小写比较
    if((strcasecmp(method.c_str(),"post")==0)&&content_length>0){
      //再报头中找Contxt-length--请求体长度不为-1
      //post方法传参参数在请求体中
      //要执行cgi
      cgi=true;
     return true;
    }
  return  false;
  }
  ssize_t ContentLength(){
    return content_length;
  }
  void SetRequestBody(string&str){
    request_body=str;
  }
  bool IsMethod(){
    //忽略大小写比较
    if(strcasecmp(method.c_str(),"post")==0&&strcasecmp(method.c_str(),"get")==0){
          return true;
    }
    return false;
  }
  bool IsGet(){
    if(strcasecmp(method.c_str(),"get")==0){
          return true;
    }
    return false;
}
  bool IsPost(){
    if(strcasecmp(method.c_str(),"post")==0){
          return true;
    }
    return false;
  }
  void UriPare(){
    //存在
  size_t pos=uri.find('?');
  if(pos!=string::npos){
    path+=uri.substr(0,pos);
    query_string=uri.substr(pos+1);
    //get方法uri后边跟着参数
   // 执行cgi
   cgi=true;
  }else{
       path+=uri;
      }
 // cout<<path<<endl;
  }
  void SetUripath(){
    path+=uri;
//    cout<<path<<endl;
  }
  void IsAddIndex(){
    //以/结尾/a/b/结尾都要添加
    //
    
    if(path[path.size()-1]=='/'){
//      cout<<"添加成功"<<endl;
        path+=HOMEPATH;
    }
  }
  string GetPath(){
    return path;
  }
  void SetPath(string&_path){
      path=_path;
  }
  void SetCgi(){
    cgi=true;
  }
  bool GetCgi(){
    return cgi;
  }
  string GetQuery(){
    return query_string;
  }
  string GetrequestBody(){
    return request_body;
  }
  ~Httprequest(){}
};
class Httpresponse{
private:
  string response_line;//响应行
  vector<string>response_headr;//响应头
  string response_space;//响应空行
  string response_body;//响应体
public:
  Httpresponse():response_space("\r\n"){
  }

  //制作响应报头
  void MakeResposeLine(int code){
    response_line+=VERSION;
    response_line+=" ";
    response_line+=to_string(code);
    response_line+=" ";
    response_line+=CodetoStatus(code);//code转换为状态码
    response_line+="\r\n";
  }
  void AddHeadr(string sud){
    response_headr.push_back(sud);
  }
  string CodetoStatus(int code){
    string status="";
    switch(code){
        case 400:
          status="fork you";
          break;
        case 404:
          status="NOT FOUND";
          break; 
        case 200:
          status="OK";
          break;
        default:
          status="OK";
          break;

    }
    return status;
  }
  string GetresponseLine(){
    return response_line;
  }
  vector<string> &GetrespinseHeadr(){
    response_headr.push_back(response_space);
    return response_headr;
  }
~Httpresponse(){}
};
class EHttp{
private:
  int sock;
  Httprequest hrq;
  Httpresponse hrp;
  string path;

private:
  void GetrequestLine(){
    string line;
  Sock::getline(sock,line);
  hrq.SetRequestLine(line);//设置请求头
  hrq.RequestLineParse();//分析请求头
  }
  void GetRequestheadr(){
    string strheadr;
    //循环读取请求报头
    //直到读到空行位置
    while(1){
    strheadr="";
    Sock::getline(sock,strheadr);
    if(strheadr.empty()){
      break;
    }
      hrq.InsertHeadrLine(strheadr);//添加请求报头到队列
     }
    hrq.RequestHeadrParse();
   // cout<<"读取时hear:"<<hrq.GetContentLeng()<<endl;
  }
  void GetRequestBody(){
    ssize_t len=hrq.ContentLength();
    char ch;
    string str="";
    while(len){
      recv(sock,&ch,1,0);
        str+=ch;
        len--;
    }
   // cout<<"set requst:"<<str<<endl;
    hrq.SetRequestBody(str);//设置请求体
  }
  void MakeResponseLineParse(int code){
      hrp.MakeResposeLine(code);
  }
  void Makersponseheadr(){
    hrq.MakeSuff();
    string suff=hrq.GetSuff();
    string suff_str=Suff(suff);
    string suffparse="Content-Type: ";
    suffparse+=suff_str;
    suffparse+="\r\n";
    hrp.AddHeadr(suffparse);
    //cout<<suffparse<<endl;
  }
  string Suff(string suff){
   string su="text/html";
      if(suff=="css"){
        su="application/x-csi";
      }
      else if(suff=="html"){
        su="text/html";
      }
    else  if(suff=="jpg"){
        su="image/jpeg";
      }
     else if(suff=="jpe"){
        su="image/jpeg";
      }
      else if(suff=="js"){
        su="application/x-jpg";
      }
      
   return su;

  }
public:

  EHttp(int sock_):sock(sock_)
  {}
  //分析请求
  void Getrequest(){
    //分析http请求行
    GetrequestLine();
    //分析http请求头
    GetRequestheadr();

    //是否要读取请求体
    //首先请求类型get不需要
    //还有请求报头中的字段有没有请求体的长度
   // cout<<"GetContentLeng:"<<hrq.GetContentLeng()<<endl;
    if(hrq.IsNeedRecvHeadr()){
      GetRequestBody();
   //   cout<<"GetRequestheadr："<<hrq.GetrequestBody()<<endl;
    }
    //已经读完所有的请求
  }
  //制作响应
  void MakeResponse(){
  
    //只处理post get请求
    int code=200;
    if(hrq.IsMethod()){
        Log(WARNING,"method  was requested ");
      goto end;
    }

    //开始解析请求
    //get方式有uri
    if(hrq.IsGet()){
      //uri存在
      //先分析uri
    hrq.UriPare();
    }
   else  if(hrq.IsPost()){
      hrq.SetUripath();
    }
    //get方法且query_string没有->资源存放在path中
    //get方法且query_string有->资源在path，给path传递的参数在query_string中
    //post方式资源在path中传递的参数在请求体中
    



   //是否添加主页以/结尾添加以其他形式不添加
   hrq.IsAddIndex();


   //检测path是否有效-->path访问的文件在web目录中存在
  
    path=hrq.GetPath();
    struct stat st;
    //检测path有效返回st.st_mode的的文件对应的模式，文件或目录
    if(stat(path.c_str(),&st)<0){
      //文件不存在路径
        Log(WARNIG,"path is not html 404");
        code=404;
    } else{
      //路径存在
      if(S_ISDIR(st.st_mode)){
        //检测返回的是不是是不是路经
        //是以/a/b结尾的但是b是一个目录
        path+="/";
     //   cout<<"添加/"<<endl;
        hrq.SetPath(path);
        hrq.IsAddIndex();
       //设置为/a/b/index.html 
      // cout<<"path"<<hrq.GetPath()<<endl;
      }else{ 
        if(S_IXUSR&st.st_mode || st.st_mode&S_IXOTH || st.st_mode&S_IXGRP){
          //检查是不是可执行文件
            //执行文件返回
            //cgi处理
       //     cout<<"kzhicode cgi"<<endl;
            hrq.SetCgi();
        }else{
          //正常的html文件
        }

       }

      if(!hrq.GetCgi()){
        //不需要返回结果
         hrq.SetFileSize(st.st_size);

       }
      
      // cout<<"path:"<<hrq.GetPath()<<endl;
    }
end:
    //根据request制作response;
    //首先制作响应行
    MakeResponseLineParse(code);
    //制作响应头
    //MakeResponseHeader();
    //制作响应体
   // MakeResponsebody();
   Makersponseheadr();
  }
  //请求参数获取
  void EXcalCgi(){
    //get方法的请求参数在uri中
   // string query;
   // if(hrq.IsGet()){

   // query=hrq.GetQuery();
   // }else{
   //   query=hrq.GetrequestBody();
   // }
   //
   int pip_in[2]={0};
   int pip_out[2]={0};
   pipe(pip_in);
   pipe(pip_out);
   string path=hrq.GetPath();
   string method=hrq.GetMethod();
   string method_env="METHOD=";//通过环境变量来
   method_env+=method;
   string query_string;
   string query_env;
   string content_length_env;
  string body;
   putenv((char*)method_env.c_str());//将method_env请求方式导入到系统环境变量，方便子进程获取
     pid_t tid=fork();

      if(tid==0){
       // cout<<"zijcng"<<endl;
 //       子进程通过只写不读
        close(pip_in[1]);
        close(pip_out[0]);
        dup2(pip_in[0],0);//重定向到stdin读取
        dup2(pip_out[1],1);//重定向到stdout写入
        //传递请求方法到子进程
          //一种是通过环境变量给子进程  ->GET方法由父进程给子进程 ->uri的请求参数query_string 因为数量有限
        if(hrq.IsGet()){
             query_string=hrq.GetQuery();
             query_env="QUERY_STRING=";//通过环境变量来
             query_env+=query_string;
             cout<<"query::"<<query_string<<endl;
             putenv((char*)query_env.c_str());//将请求参数导入的系统环境变量中方便子

        }else if(hrq.IsPost()){
          content_length_env="CONTENT-LENGTH=";
          content_length_env+=to_string(hrq.GetContentLeng());
       //  cout<<"cont-ength_env"<<content_length_env<<endl;
         putenv((char*)content_length_env.c_str());
        }else{

        }
        //执行程序将结果返回给父进程
        //程序替换
        execl(path.c_str(),path.c_str(),NULL);
        exit(0);
      }
      close(pip_in[0]);
      close(pip_out[1]);
       //另一种是通过管道传递给子进程 ->POST由父进程传递给子进程 ->  请求体
      if(hrq.IsPost()){
        
          body=hrq.GetrequestBody();
      //    ssize_t size=body.size();
      //    while(size){
      //      write(pip_in[1],&c,1);
       //     size--;
        //  }
     //     cout<<"ispost()"<<body<<endl;
          char ch='X';
          
       //   cout<<"post body"<<body<<endl;
          for(size_t i=0;i<body.size();i++){
            ch=body[i];
          write(pip_in[1],&ch,1);
            
       }
      }
      char ch=0;
      ssize_t s=0;
      do{
         s=read(pip_out[0],&ch,1);//读取子进程返回的结果
           if(s>0){
           send(sock,&ch,1,0);//直接发送子进程返回的结果
    }
      }while(s>0);
      //进程等待
      waitpid(tid,NULL,0);
  }

  //发送响应
  void SendREquest(){
    //发送响应行
    send(sock,hrp.GetresponseLine().c_str(),strlen(hrp.GetresponseLine().c_str()),0);
    vector<string> responseheadr=hrp.GetrespinseHeadr();
    for(string it:responseheadr){
    send(sock,it.c_str(),strlen(it.c_str()),0);
    }
    //检测CGI是否有可执行文件并返回结果
    if(hrq.GetCgi()){
      //返回请求结果比如传送的表单的执行结果
 //     cout<<"get:"<<"getqingqiu"<<endl;
      EXcalCgi();
    }else{
      //请求没有参数
      //不用返回结果
      //直接返回请求资源
      SendFile();
    }

  }

  void SendFile(){
    //先打开文件
    string path=hrq.GetPath();
    cout<<"path:"<<hrq.GetPath()<<endl;
    int fd=open(path.c_str(),O_RDONLY);//只读方式打开文件
    if(fd<0){
      Log(ERROR,"open file is bug!!");
      return;
    }
    sendfile(sock,fd,NULL,hrq.GetFileSize());

  
    close(fd);
  }

};
class Enpty{
  public:
    static void EnptyHttp(int sock){
   //   int sock=*(int *)arg;
   //   delete (int*)arg;
#ifdef _DEBUG  
      char buff[10240];
      int c=recv(sock,buff,sizeof(buff),0);
      if(c>0){
      buff[c]=0;
      cout<<buff<<endl;     
    
#else
  
      EHttp *eh=new EHttp(sock);
      eh->Getrequest();
      eh->MakeResponse();
      eh->SendREquest();
      close(sock);
#endif
      
    }
};
