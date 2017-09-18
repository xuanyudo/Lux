#include "Parser.h"
//#include "Parser.cpp"
using namespace std;

int main(){
  Parser* parse = new Parser("Data.json");
  cout<<"Serial number: "<<parse->getSerial()<<endl;
  cout<<"Command: "<<parse->getCommand()<<endl;
  cout<<"Group name: "<<parse->getGroupName()<<endl;
  cout<<"Device name: "<<parse->getDeviceName()<<endl;
  cout<<"IP: "<<parse->getIP()<<endl;
  cout<<"Light status (1 or 0): "<<parse->getLight_S()<<endl;
  cout<<"Light level (1-10): "<<parse->getLight_L()<<endl;
  parse->setIP("120.84.1455");
  parse->setGroupName("living_room");
  parse->setGroupName("living_room_1");
  parse->setLight_S(0);
  parse->setLight_L(6);
  cout<<"--------------------------------"<<endl;
  cout<<"New "<<"IP: "<<parse->getIP()<<endl;
  cout<<"New "<<"Group name: "<<parse->getGroupName()<<endl;
  cout<<"New "<<"Device name: "<<parse->getDeviceName()<<endl;
  cout<<"New "<<"Light status (1 or 0): "<<parse->getLight_S()<<endl;
  cout<<"New "<<"Light level (1-10): "<<parse->getLight_L()<<endl;
  return 0;
}
