#include<iostream>
#include<string>

#include<sstream>
using namespace std;
class Util{
  public:
    static void StringParse(string &line,string &methon,string &uri,string &version){
    stringstream ss(line);
    ss>>methon>>uri>>version;
    }
    static void MakeStringKv(string StringKv,string &k,string &v){
      size_t f =StringKv.find(':');
      if(f!=string::npos){
      k=StringKv.substr(0,f);
      v=StringKv.substr(f+2);

      }
    }
    static int StringToInt(string str){
      return atoi(str.c_str());
    }
};
