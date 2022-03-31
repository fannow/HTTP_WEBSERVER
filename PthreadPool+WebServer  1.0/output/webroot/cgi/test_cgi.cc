#include<iostream>
#include<strings.h>
#include<unistd.h>
#include<stdlib.h>
using namespace std;
int main(){
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
    string ss="";
    char c;
   while(s){
      read(0,&c,1);
      ss.push_back(c);
      s--;
    }
   cout<<"cgi"<<ss<<endl;
  }
  cout<<"cgi"<<query<<endl;
  return 0;
}
