#include "Parser.h"
//#include "Parser.cpp"
using namespace std;

int main(){
  Parser* parse = new Parser("{\"cmd\":status,\"uuid\":123,\"Serial\":987654321,\"data\":{\"Device\":noob,\"IP\":120.84.1474,\"light_status\":1,\"light_level\":5,\"Group\":living_room_3000}}");
  // cout<<"true or false: "<<parse->getValidation()<<endl;
  cout<<"Command: "<<parse->getCommand()<<endl;
  cout<<"uuid: "<<parse->getUUID()<<endl;
  cout<<"Serial number: "<<parse->getSerial()<<endl;
  cout<<"Group name: "<<parse->getGroupName()<<endl;
  cout<<"Device name: "<<parse->getDeviceName()<<endl;
  cout<<"IP: "<<parse->getIP()<<endl;
  cout<<"Light status (1 or 0): "<<parse->getLight_S()<<endl;
  cout<<"Light level (1-10): "<<parse->getLight_L()<<endl;
  return 0;
}
