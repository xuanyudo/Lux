#include "encode.h"
//#include "Parser.cpp"
using namespace std;

int main(){
  Encode* encode = new Encode();
  encode->setSerialN("987654321");
  encode->setIP("120.84.1474");
  encode->setGroupName("living_room");
  encode->setDeviceName("noob");
  encode->setLight_S(1);
  encode->setLight_L(5);
  encode->setCommand("status");
  encode->setUuid(123);
   encode->setGroupName("living_room_3000");
  string json = encode->stringfy();
  cout<<json<<endl;
}
