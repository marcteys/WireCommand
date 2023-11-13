
#include <WireCommand.h>
WireCommand wireCommand;

#define SLAVE_ADDRESS 0x10

void setup() {
  Serial.begin(115200);
  wireCommand.begin(SLAVE_ADDRESS);
  wireCommand.addCommand("sendSlave", ReceiveStuff);
}

// this should happened only once
void ReceiveStuff() {
  char *arg;
  arg = wireCommand.next();
  if (arg != NULL)  {
    int firstParm = atoi(arg);
    Serial.print("The first variable is ");
    Serial.println(firstParm);
  }  else {
    Serial.println("Error parsing.");
  }
}


void loop() {
  wireCommand.update();
  wireCommand.delay(10);
}
