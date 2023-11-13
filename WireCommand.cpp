

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <string.h>

#include<Wire.h>


#include "WireCommand.h"


WireCommand * WireCommand::_instance = NULL;

WireCommand::WireCommand() {
  WireCommand::_instance = this;
  //Serial
  strncpy(delim, "/", MAXDELIMETER);
  term = '\r';
  numCommand = 0;
  clearBuffer();
}

void WireCommand::begin(int addr) {
  if (addr == 0) {
    Wire.begin(addr); // Master
  }
  else Wire.begin(addr); // Slave

  Wire.setClock(1000000);

  Wire.onReceive(WireCommand::_instance->recieveEvent); // Message recevied fro Master to Slave
  Wire.onRequest(WireCommand::_instance->requestEvent); // Message recevied from Slave to Master
}


// When master send request. This should not happend
void WireCommand::requestEvent()
{

  for (int i = 0; i < SERIALCOMMANDBUFFER; i++ ) {
//    WireCommand::_instance->outCommingMessagesBuffer[i] = '\0'; // reset the string
  }

  for (int i = 0; i < MAX_BUFFER_MESSAGES; i++) {
    if (WireCommand::_instance->outCommingMessagesBuffer[i][0] != '\0') {
      Wire.write(WireCommand::_instance->outCommingMessagesBuffer[i]);

#ifdef WIRE_SERIAL_DEBUG
      Serial.print("Wire message ");
      Serial.print(WireCommand::_instance->outCommingMessagesBuffer[i]);
      Serial.print(" at ");
      Serial.print(i);
      Serial.println(" sent from buffer for Master ");
#endif
      WireCommand::_instance->outCommingMessagesBuffer[i][0] = '\0';
      break;
    }
  }


  // }

}


void WireCommand::recieveEvent() {

  /*
    int receiveIndex  = 0;
    char serialInputString[SERIALCOMMANDBUFFER];

    while (Wire.available()) {
    char inChar = (char)Wire.read();
    serialInputString[receiveIndex] = inChar;
    receiveIndex++;
    }
    WireCommand::_instance->ParseIncommingMessage(serialInputString);

  */

  for (int i = 0; i < SERIALCOMMANDBUFFER; i++ ) {
    WireCommand::_instance->receivedDataBuffer[i] = '\0'; // reset the string
  }
  int i = 0;
  while ( Wire.available()) {
    WireCommand::_instance->receivedDataBuffer[i] = (char)Wire.read();
    i++;
  }
  WireCommand::_instance->receivedDataBuffer[i] = 0;
  WireCommand::_instance->ParseIncommingMessage(WireCommand::_instance->receivedDataBuffer);
  WireCommand::_instance->clearBuffer();


}


void WireCommand::setMessageData(int numOfData, int first, int second, int third, int fourth, int fifth) {
  messageData[0] = numOfData;
  messageData[1] = first;
  messageData[2] = second;
  messageData[3] = third;
  messageData[4] = fourth;
  messageData[5] = fifth;
}

int WireCommand::sendMessage(byte targetAddr, char * message, int value) {
  messageData[0] = 1;
  messageData[1] = value;
  return sendMessage(targetAddr, message);
}



int WireCommand::sendMessage(byte targetAddr, char * message) {

  char newMessageOutput[SERIALCOMMANDBUFFER];
  memset(&newMessageOutput[0], 0, sizeof(newMessageOutput));
  int index = 0;
  for (index = 0; message[index] != '\0'; index++) {
    newMessageOutput[index] = message[index];
  }


  // build message string
  if (messageData[0] > 0) {
    for (int ix = 1; ix <= messageData[0]; ix++) {  // For each parameter
      sprintf(newMessageOutput + strlen(newMessageOutput), "/%d", messageData[ix] );
    }
  }

  // todo : vérifier que messageOutputData.c_str() st plus petit que SERIALCOMMANDBUFFER  , sinon elle ne sera pas parsée de l'autre coté


  int state = 4;
  if (targetAddr == 0) {  // Store to Buffer

    bool storeInBuffer = true;
    // Do not store message if already in buffer
    for (int buff = 0; buff < MAX_BUFFER_MESSAGES; buff++ ) {
      if (strcmp(outCommingMessagesBuffer[buff], newMessageOutput) == 0) {
#ifdef WIRE_SERIAL_DEBUG
        Serial.print("Not storing message already present ");
        Serial.println(newMessageOutput);
#endif
        storeInBuffer = false;
      }
    }
    if (storeInBuffer) {
      // Copy in the first closest
      for (int i = 0; i < MAX_BUFFER_MESSAGES; i++) {
        if (outCommingMessagesBuffer[i][0] == '\0') {
          strncpy(outCommingMessagesBuffer[i], newMessageOutput, SERIALCOMMANDBUFFER);
          break;
        } else if (i == MAX_BUFFER_MESSAGES - 1) { // always copy in the last one
          strncpy(outCommingMessagesBuffer[i], newMessageOutput, SERIALCOMMANDBUFFER);
        }
      }

#ifdef WIRE_SERIAL_DEBUG
      Serial.print("Wire message ");
      Serial.print(newMessageOutput);
      Serial.println(" was stored to buffer for  Master");
#endif
    }
    state = 0;
  } else {  // SendToSlave
    Wire.beginTransmission(targetAddr);
    Wire.write(newMessageOutput);
    Wire.requestFrom((uint8_t)targetAddr, (uint8_t)1);
    state = Wire.endTransmission();

#ifdef WIRE_SERIAL_DEBUG
    Serial.print("Wire message ");
    Serial.print(newMessageOutput);
    Serial.print(" sent to ");
    Serial.print(targetAddr);
    Serial.print(", state: ");
    Serial.println(state);
#endif
  }



  // Clear data
  for (int i = 0; i < 6; i++)
    messageData[i] = 0;

  // TODO : catcherror here
  // if(state !=0) screen.displaySend(state.c_str());
  return state;
}




void WireCommand::ParseIncommingMessage(char * incommingMessage) {
  int i;
  boolean matched;


#ifdef SERIAL_DEBUG
  Serial.print("Received: ");
  Serial.println(incommingMessage);
#endif
  bufPos = 0;         // Reset to start of buffer
  token = strtok_r(incommingMessage, delim, &last); // Search for command at start of buffer
  if (token == NULL) return;
  matched = false;
  for (i = 0; i < numCommand; i++) {
#ifdef SERIAL_DEBUG
    Serial.print("Comparing [");
    Serial.print(token);
    Serial.print("] to [");
    Serial.print(CommandList[i].command);
    Serial.println("]");
#endif
    // Compare the found command against the list of known commands for a match
    if (strncmp(token, CommandList[i].command, SERIALCOMMANDBUFFER) == 0)
    {
#ifdef SERIAL_DEBUG
      Serial.print("Matched Command: ");
      Serial.println(token);
#endif
      // Execute the stored handler function for the command
      (*CommandList[i].function)();
      clearBuffer();
      matched = true;
      break;
    }

  }
}



void WireCommand::ReadSerial() {
  int receiveIndex  = 0;
  char serialInputString[SERIALCOMMANDBUFFER];
  while (Serial.available()) {
    char inChar = Serial.read();
    serialInputString[receiveIndex] = '\0';
    if (inChar != '\r' && inChar != '\n') {
      serialInputString[receiveIndex] = inChar;
      receiveIndex++;
    }
    if (inChar == '\n' && receiveIndex != 0) {
      strcpy(receivedDataBuffer, serialInputString);
      ParseIncommingMessage(receivedDataBuffer);
    }
  }
  clearBuffer();
}


void WireCommand::ReadWire() {
  int receiveIndex  = 0;
  // char serialInputString[SERIALCOMMANDBUFFER];
  //serialInputString[0] = '\0';

/*  for (int i = 0; i < SERIALCOMMANDBUFFER; i++ ) {
    receivedDataBuffer[i] = '\0'; // reset the string
  }
  */
  
  bool hasNewData = false;
  while (Wire.available()) {
    hasNewData = true;
    char inChar = (char)Wire.read();
    if (inChar != -1 ) {
      receivedDataBuffer[receiveIndex] = inChar;
      receiveIndex++;
    }
  }
  if (hasNewData && receivedDataBuffer[0] != '\0') {
    ParseIncommingMessage(receivedDataBuffer);
    clearBuffer();
  }
}



//
// Initialize the command buffer being processed to all null characters
//
void WireCommand::clearBuffer()
{
  for (int i = 0; i < SERIALCOMMANDBUFFER; i++)
  {
    receivedDataBuffer[i] = '\0';
  }
  bufPos = 0;
}


// Retrieve the next token ("word" or "argument") from the Command buffer.
// returns a NULL if no more tokens exist.
char *WireCommand::next()
{
  char *nextToken;
  nextToken = strtok_r(NULL, delim, &last);
  return nextToken;
}


void WireCommand::update()
{
  ReadSerial();
}

void WireCommand::request(int addr)
{
  // delay(10);
  Wire.requestFrom(addr, SERIALCOMMANDBUFFER);
    delay(1);
  ReadWire();
}



void WireCommand::delay(unsigned int duration) {
  unsigned long time_now = millis();
  while (millis() < time_now + duration) {
    WireCommand::update();
  }
}



void WireCommand::addCommand(const char *command, void (*function)()) {

  if (numCommand < MAXSERIALCOMMANDS - 1) {
#ifdef SERIAL_DEBUG
    Serial.print(numCommand);
    Serial.print("-");
    Serial.print("Adding command for ");
    Serial.println(command);
#endif
    strncpy(CommandList[numCommand].command, command, SERIALCOMMANDBUFFER);
    CommandList[numCommand].function = function;
    numCommand++;
  } else {
#ifdef SERIAL_DEBUG
    Serial.println("Error, too much commands!");
#endif
  }
}




void WireCommand::scanAllPorts() {
  byte error, address; int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for (address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX); Serial.println();
      nDevices++;
    }
    else if (error == 4)    {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found\n");
  else Serial.println("Scan complete.\n");
}