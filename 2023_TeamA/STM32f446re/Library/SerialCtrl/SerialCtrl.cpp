#include <SerialCtrl.h>
#include "mbed.h"

SerialCtrl::SerialCtrl(Serial *pts, Timer *ptt) {
    _Serial = pts;
    _Timer = ptt;
    TIMEOUT_MS = 8;
    RETRY_MAX = RETRY_DEF;
}
    
void SerialCtrl::param(unsigned char header, unsigned char footer) {
    STX = header;
    ETX = footer;
}

bool SerialCtrl::get() {
    int retry = 0;
    
    do {
        //judge start of text
        if( !(input() == STX) ) {
            ++retry;
        } else {
            unsigned char sum = 0;
            
            //get data
            for(int i=0; i<DATA_N;   ++i) data[i] = input();
            for(int i=0; i<DATA_N-1; ++i) sum += data[i];
    
            //judge checksum and end of text
            if( (sum == data[SUM]) && (input() == ETX) ) {
                return true;
            } else {
                ++retry;
            }
        }
    } while(retry < RETRY_MAX);
    return false;
}

void SerialCtrl::setFailMax(int n) {
    if(n >= 0) {
        RETRY_MAX = n;
    } else {
        return;
    }
}
void SerialCtrl::setFailMax() {
    RETRY_MAX = RETRY_DEF;
}

unsigned char SerialCtrl::input() {
    _Timer->reset();
    while(_Timer->read_ms() < TIMEOUT_MS) {
        if( _Serial->readable() ) {
            return _Serial->getc();
        }
    }
    return NUL;
}
