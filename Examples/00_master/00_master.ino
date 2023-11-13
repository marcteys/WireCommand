#define WIRE_SERIAL_DEBUG 1
//#undef WIRE_SERIAL_DEBUG

#define MAXSERIALCOMMANDS  15
#include <WireCommand.h>
WireCommand wireCommand;

#define ADDRESS 0x00
#define SLAVE_ADDRESS 0x10
#define OTHER_SLAVE_ADDRESS 0x20

void setup() {
  Serial.begin(115200);
  wireCommand.begin(ADDRESS);
  wireCommand.addCommand("command", ReceiveCommandStatus);
  wireCommand.addCommand("send", SendStuff);
}


// this should happened only once
void SendStuff() {
  wireCommand.sendMessage(0x10, "sendStuff", 32, 255);
}

// this should happened only once
void ReceiveCommandStatus() {
  char *arg;
  arg = wireCommand.next();
  if (arg != NULL)  {
    Serial.print("Receive argument");
  }  else {
    Serial.println("ReceiveBoardStatus:this should not happend");
  }
}

void ScanAll()  {
  wireCommand.scanAllPorts();
}

void loop() {
  ForceRequest();
  wireCommand.update();
}

void ForceRequest() {
  wireCommand.request(0x10);
}


void Error(String error) { // todo : changer ça, ici on créé une fonction un peu pour rien et qui peut faire de la confusion
  Error(error.c_str());
}

void Error(char * error) {
  Serial.print("Error : ");
  Serial.println(error);

}
