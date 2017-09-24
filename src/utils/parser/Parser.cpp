#include "Parser.h"
using namespace std;

/*
Parser::Parser accepts a file name and extracts the entire file into a string.
After getting a single string of information, it stringstream it into a struct of data_name
and its data in order.
*/
Parser::Parser(std::string jfile){
  ifstream file;                          //Basic file reading start
  file.open(jfile);
  int count = 0;
  char fchar;
  string sdata = "";                      //Single string that contains all data and data_name
  while(!file.eof()){                     //While loop to extract charcters only within " "
    fchar = file.get();
    if(fchar == '"' && count == 0){
      count = 1;
    }
    else if(fchar == '"' && count == 1){
      count = 0;
      sdata = sdata + ' ';
    }
    if(count == 1 && fchar!='"'){
      sdata = sdata + fchar;
    }
  };                                      //Basic file reading end
  stringstream insert_data(sdata);        //Assigning data to their own field
  string input;
  while(insert_data>>input){
    if(input == "Serial"){
      this->info_name.serial_num = input;
      insert_data>>input;
      this->info.serial = input;
    }
    else if(input == "Command"){
      this->info_name.command = input;
      insert_data>>input;
      this->info.command_s = input;
    }
    else if(input == "Group"){
      this->info_name.group_name = input;
      insert_data>>input;
      this->info.group = input;
    }
    else if(input == "Device"){
      this->info_name.device_name = input;
      insert_data>>input;
      this->info.device = input;
    }
    else if (input == "IP"){
      this->info_name.ip_address = input;
      insert_data>>input;
      this->info.ip = input;
    }
    else if (input == "Status"){
      this->info_name.light_status = input;
      int intput;
      insert_data>>intput;
      this->info.status = intput;
    }
    else if (input == "Level"){
      this->info_name.light_level = input;
      int intput;
      insert_data>>intput;
      this->info.level = intput;
    }
    else{cout<<"error"<<endl;}
  }
};
string Parser::getSerial()const{
  return info.serial;
}
string Parser::getCommand()const{
  return info.command_s;
}
int Parser::getLight_S()const{
  return info.status;
}
int Parser::getLight_L()const{
  return info.level;
}
string Parser::getIP()const{
  return info.ip;
}
string Parser::getGroupName()const{
  return info.group;
}
string Parser::getDeviceName()const{
  return info.device;
}
void Parser::setIP(string newIP){
  this->info.ip = newIP;
}
void Parser::setLight_S(int newStatus){
  this->info.status = newStatus;
}
void Parser::setLight_L(int newLevel){
  this->info.level = newLevel;
}
void Parser::setGroupName(string newGroupN){
  this->info.group = newGroupN;
}
void Parser::setDeviceName(string newDeviceN){
  this->info.group = newDeviceN;
}
