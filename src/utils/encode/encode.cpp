#include "encode.h"
using namespace std;

/*

*/
Encode::Encode(std::string jfile){
  this->file_name = jfile;
  this->info_name.serial_num = "Serial";
  this->info_name.group_name = "Group";
  this->info_name.device_name = "Device";
  this->info_name.ip_address = "IP";
  this->info_name.light_status = "Status";
  this->info_name.light_level = "Level";
  this->info_name.command = "Command";
};
void Encode::stringfy(){
  ofstream json;
  json.open(file_name);
  json<<"{ "<<'"'<<info_name.serial_num<<'"'<<':'<<'"'<<info.serial<<'"'<<endl;
  json<<"\t"<<'{'<<endl;
  json<<"\t\t"<<'"'<<info_name.command<<'"'<<": "<<'"'<<info.command_s<<'"'<<endl;
  json<<"\t\t"<<'{'<<endl;
  if(info.command_s == "on"){
    json<<"\t\t\t"<<'"'<<info_name.device_name<<'"'<<": "<<'"'<<info.device<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.ip_address<<'"'<<": "<<'"'<<info.ip<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.light_status<<'"'<<": "<<'"'<<info.status<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.light_level<<'"'<<": "<<'"'<<info.level<<'"'<<','<<endl;
  }
  else if(info.command_s == "off"){
    json<<"\t\t\t"<<'"'<<info_name.device_name<<'"'<<": "<<'"'<<info.device<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.ip_address<<'"'<<": "<<'"'<<info.ip<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.light_status<<'"'<<": "<<'"'<<info.status<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.light_level<<'"'<<": "<<'"'<<info.level<<'"'<<','<<endl;
  }
  else if(info.command_s == "test"){
    json<<"\t\t\t"<<'"'<<info_name.ip_address<<'"'<<": "<<'"'<<info.ip<<'"'<<','<<endl;
  }
  else if(info.command_s == "status"){
    json<<"\t\t\t"<<'"'<<info_name.device_name<<'"'<<": "<<'"'<<info.device<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.ip_address<<'"'<<": "<<'"'<<info.ip<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.light_status<<'"'<<": "<<'"'<<info.status<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.light_level<<'"'<<": "<<'"'<<info.level<<'"'<<','<<endl;
    json<<"\t\t\t"<<'"'<<info_name.group_name<<'"'<<": "<<'"'<<info.group<<'"'<<','<<endl;
  }
  else if(info.command_s == "exit"){
    json<<"\t\t\t"<<'"'<<info_name.ip_address<<'"'<<": "<<'"'<<info.ip<<'"'<<','<<endl;
  }
  json<<"\t\t"<<'}'<<endl;
  json<<"\t"<<'}'<<endl;
  json<<'}'<<endl;
}
void Encode::setIP(string newIP){
  this->info.ip = newIP;
}
void Encode::setLight_S(int newStatus){
  this->info.status = newStatus;
}
void Encode::setLight_L(int newLevel){
  this->info.level = newLevel;
}
void Encode::setGroupName(string newGroupN){
  this->info.group = newGroupN;
}
void Encode::setDeviceName(string newDeviceN){
  this->info.device = newDeviceN;
}
void Encode::setCommand(string command_string){
  this->info.command_s = command_string;
}
void Encode::setSerialN(string serial_input){
  this->info.serial = serial_input;
}
