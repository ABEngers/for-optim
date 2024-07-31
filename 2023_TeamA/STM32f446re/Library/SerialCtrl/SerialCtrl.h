#ifndef SERIALCTRL
#define SERIALCTRL

#include "mbed.h"

#define DATA_N 9
enum {LX, LY, RX, RY, L2, R2, B1, B2, SUM};
#define TRIANGLE 0x01
#define CIRCLE   0x02
#define CROSS    0x04
#define SQUARE   0x08
#define UP       0x10
#define RIGHT    0x20
#define DOWN     0x40
#define LEFT     0x80

#define L1       0x01
#define L3       0x02
#define R1       0x04
#define R3       0x08
#define SELECT   0x10
#define START    0x20
#define PS       0x40

class SerialCtrl {
public:
    SerialCtrl(Serial*, Timer*);
    unsigned char data[DATA_N];
    
    void param(unsigned char header, unsigned char footer);
    bool get();
    void setFailMax(int n);
    void setFailMax();

private:
    unsigned char input();
    Serial *_Serial;
    Timer *_Timer;
    
    unsigned char STX; //header data code
    unsigned char ETX; //footer data code
    int TIMEOUT_MS;    //timeout limit time
    int RETRY_MAX;     //retry limit
    
    #define NUL       0  //dummy data
    #define RETRY_DEF 15 //fail max default
    
};

#endif //SERIALCTRL