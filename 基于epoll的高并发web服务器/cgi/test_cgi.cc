#include<iostream>
#include<strings.h>
#include<stdio.h>
#include<string>
#include<unistd.h>
#include<stdlib.h>
using namespace std;
void fun(string post){
  size_t pos=post.find('&');
  string port1;
    string port2;
  if(pos!=string::npos){
      port1=post.substr(0,pos);
      port2=post.substr(pos+1);      
  }
    size_t pos1=port1.find('=');
    size_t pos2=port2.find('=');
    int a=0;
    int b=0;
    if(pos1!=string::npos){
      a=atoi(port1.substr(pos1+1).c_str());
    }
    if(pos2!=string::npos){
       b=atoi(port2.substr(pos2+1).c_str()); 
    }
cout<<a<<"+"<<b<<"="<<a+b<<endl;
cout<<a<<"-"<<b<<"="<<a-b<<endl;
cout<<a<<"/"<<b<<"="<<a/b<<endl;
cout<<a<<"*"<<b<<"="<<a*b<<endl;



}
int main(){
  cout<<"<html>\
    <h1>你好</h1>\
</html>"<<endl;
  cout<<"hello"<<endl;
  string method="";
  string query="";
  if(getenv("METHOD")){
    method=getenv("METHOD");

  }
  else{
  return 1;  
  }
  if(strcasecmp(method.c_str(),"get")==0){
    query=getenv("QUERY_STRING");
  }else if(strcasecmp(method.c_str(),"post")==0){
    size_t s=atoi(getenv("CONTENT-LENGTH"));
    
    char c;
   while(s){
      read(0,&c,1);
      query.push_back(c);
      s--;
    }
   cout<<"cgi"<<query<<endl;

  }
  cout<<query<<":::cgi"<<endl;
  fun(query);
  return 0;
}
