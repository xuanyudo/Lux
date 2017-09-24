#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Encode
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
  std::string serial_num;
  std::string group_name;
  std::string device_name;
  std::string ip_address;
  std::string light_status;
  std::string light_level;
  std::string command;
};
private:
  data info;              //Making our struct data private
  data_name info_name;    //Making out struct data_name private
  std::string file_name;
public:
  Encode(std::string file);
  ~Encode();
  void setCommand(std::string com);
  void setSerialN(std::string serial_n);
  void setGroupName(std::string groupN);
  void setDeviceName(std::string deviceN);
  void setIP(std::string newIP);
  void setLight_S(int newStatus);
  void setLight_L(int newLevel);
  void stringfy();
};
