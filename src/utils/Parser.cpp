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
  insert_data>>this->info.serial;
  insert_data>>this->info_name.group_name;
  insert_data>>this->info.group;
  insert_data>>this->info_name.device_name;
  insert_data>>this->info.device;
  insert_data>>this->info_name.ip_address;
  insert_data>>this->info.ip;
  insert_data>>this->info_name.light_status;
  insert_data>>this->info.status;
  insert_data>>this->info_name.light_level;
  insert_data>>this->info.level;
  insert_data>>this->info_name.command;
  insert_data>>this->info.command_s;
  //data_map[serial][&this->info_name]= &this->info; //commented out because I don't think we need a map
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
