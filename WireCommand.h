
#ifndef WireCommand_h
#define WireCommand_h


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



#include<Wire.h>

// Commands parser
#ifndef SERIALCOMMANDBUFFER
#define SERIALCOMMANDBUFFER 30
#endif

#ifndef MAXSERIALCOMMANDS
#define MAXSERIALCOMMANDS  6
#endif

#define MAXDELIMETER 2

#ifndef MAX_BUFFER_MESSAGES
#define MAX_BUFFER_MESSAGES 3
#endif


#define WIRE_SERIAL_DEBUG 1


class WireCommand {

  public:
    static WireCommand * _instance;

    WireCommand();
    void begin(int addr);
    int sendMessage(byte targetAddr, char * message);
    int sendMessage(byte targetAddr, char * message, int value);
    void setMessageData(int numOfData, int first, int second, int third, int fourth, int fifth);

    void scanAllPorts();
    static void receiveEvent();
    void ParseIncommingMessage(char * incommingMessage);

    // Serial Commands
    char *next();         // returns pointer to next token found in command buffer (for getting arguments to commands)
    void update();
    void request(int addr);
    void delay(unsigned int duration);
    void clearBuffer();

    void addCommand(const char *, void(*)());   // Add commands to processing dictionary

    static void recieveEvent(int numBytes);
    static void requestEvent();
    void ReadSerial();
    void ReadWire();

    char outCommingMessagesBuffer[MAX_BUFFER_MESSAGES][SERIALCOMMANDBUFFER];
    char receivedDataBuffer[SERIALCOMMANDBUFFER];

  private:

    //Serial Command
    int  bufPos;                        // Current position in the buffer
    char delim[MAXDELIMETER];           // null-terminated list of character to be used as delimeters for tokenizing (default " ")
    char term;                          // Character that signals end of command (default '\r')
    char *token;                        // Returned token from the command buffer as returned by strtok_r
    char *last;                         // State variable used by strtok_r during processing
    typedef struct _callback {
      char command[12];
      void (*function)();
    } SerialCommandCallback;            // Data structure to hold Command/Handler function key-value pairs
    int numCommand;
    SerialCommandCallback CommandList[MAXSERIALCOMMANDS];   // Actual definition for command/handler array
    // void (*defaultHandler)();           // Pointer to the default handler function

    char messageOutputData[SERIALCOMMANDBUFFER];
    int messageData[6];

};

#endif