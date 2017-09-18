#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

class Parser
{
struct data{
  std::string serial;
  std::string group;
  std::string device;
  std::string ip;
  int status;
  int level;
  std::string command_s;
};
struct data_name{
  std::string group_name;
  std::string device_name;
  std::string ip_address;
  std::string light_status;
  std::string light_level;
  std::string command;
};
private:
  //std::map<std::string,std::map<data_name*, data*>> data_map;   //Map was not use
  data info;              //Making our struct data private
  data_name info_name;    //Making out struct data_name private
  //std::string serial;
public:
  Parser(std::string data);
  ~Parser();
  std::string getIP()const;
  std::string getSerial()const;
  int getLight_S()const;
  int getLight_L()const;
  std::string getGroupName()const;
  std::string getDeviceName()const;
  void setGroupName(std::string groupN);
  void setDeviceName(std::string deviceN);
  std::string getCommand()const;
  void setIP(std::string newIP);
  void setLight_S(int newStatus);
  void setLight_L(int newLevel);
  //void make_data_key(std::string ip_key, std::string lightS_key, std::string lightL_key, std::string com_key);
  //void make_data(std::string serial_d,std::string ip_d,int lightS_d, int lightL_d,std::string com_d);
};
