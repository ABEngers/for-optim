#include <SoftwareSerial.h>
#include <PS4USB.h>

#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USB usb; //USB宣言
PS4USB DS4(&usb);

SoftwareSerial s_Serial(7, 6); //SoftwareSerial宣言

enum { LX, LY, RX, RY, L_2, R_2, BA1, BA2, SUM }; //データ位置の定義
#define BAUDRATE   38400 //serial baudrate
#define DATA_N     9     //送信するデータ数 number of data
#define SEND_DELAY 8     //送信ディレイ時間 ms

void setup() {
  Serial.begin(BAUDRATE);   //HardwareSerial baudrate設定
  s_Serial.begin(BAUDRATE); //SoftwareSerial baudrate設定

  //サンプルからほぼそのまま移植
  #if !defined(__MIPSEL__)
  while (!Serial);
  #endif
  if (usb.Init() == -1) {
    while (1); //halt
  }
}

void loop() {
  unsigned char data[DATA_N] = {}; //データ格納配列
  usb.Task(); //USB処理開始(呪文)
  
  if(DS4.connected()) { //DualShock4接続確認
    
    data[LX]  = DS4.getAnalogHat(LeftHatX);  //LスティックX軸
    data[LY]  = DS4.getAnalogHat(LeftHatY);  //LスティックY軸
    data[RX]  = DS4.getAnalogHat(RightHatX); //RスティックX軸
    data[RY]  = DS4.getAnalogHat(RightHatY); //RスティックY軸
    data[L_2] = DS4.getAnalogButton(L2);     //L2ボタン
    data[R_2] = DS4.getAnalogButton(R2);     //R2ボタン

    //ButtonAssign1 値検出
    if(DS4.getButtonPress(TRIANGLE)) data[BA1] |= 0x01; //三角ボタン
    if(DS4.getButtonPress(CIRCLE))   data[BA1] |= 0x02; //丸ボタン
    if(DS4.getButtonPress(CROSS))    data[BA1] |= 0x04; //バツボタン
    if(DS4.getButtonPress(SQUARE))   data[BA1] |= 0x08; //四角ボタン
    if(DS4.getButtonPress(UP))       data[BA1] |= 0x10; //十字キー上
    if(DS4.getButtonPress(RIGHT))    data[BA1] |= 0x20; //十字キー右
    if(DS4.getButtonPress(DOWN))     data[BA1] |= 0x40; //十字キー下
    if(DS4.getButtonPress(LEFT))     data[BA1] |= 0x80; //十字キー左

    //ButtonAssign2 値検出
    if(DS4.getButtonPress(L1))     data[BA2] |= 0x01; //L1
    if(DS4.getButtonPress(L3))     data[BA2] |= 0x02; //L3(左スティック押しこみ)
    if(DS4.getButtonPress(R1))     data[BA2] |= 0x04; //R1
    if(DS4.getButtonPress(R3))     data[BA2] |= 0x08; //R3(右スティック押しこみ)
    if(DS4.getButtonPress(SELECT)) data[BA2] |= 0x10; //SELECTボタン
    if(DS4.getButtonPress(START))  data[BA2] |= 0x20; //STARTボタン
    if(DS4.getButtonPress(PS))     data[BA2] |= 0x40; //PSボタン

    for(int i=0; i<DATA_N-1; ++i) data[SUM] += (int)data[i]; //CheckSUM加算
    
    s_Serial.write(0xAF); //先頭データ送信
    for(int i=0; i<DATA_N; ++i) s_Serial.write(data[i]); //データ送信
    s_Serial.write(0xED); //終端データ送信
    s_Serial.write('\r'); //CR
    s_Serial.write('\n'); //LF
  }
  delay(SEND_DELAY); //送信バッファの圧迫回避
}


/*Data Assign
Serial
|-------|---------------|----------------------
| Byte  |     Data      | Information
|-------|---------------|----------------------
| 0Byte |     0xAF      | Head of data
| 1Byte | unsigned char | LStick X
| 2Byte | unsigned char | LStick Y
| 3Byte | unsigned char | RStick X
| 4Byte | unsigned char | RStick Y
| 5Byte | unsigned char | L2
| 6Byte | unsigned char | R2
| 7Byte | unsigned char | ButtonAssign1
| 8Byte | unsigned char | ButtonAssign2
| 9Byte | unsigned char | CheckSUM
|10Byte |     0xED      | End of data
|-------|---------------|----------------------

7Byte ButtonAssign1
|-------|--------|-------------
|  Bit  |  Data  | Information
|-------|--------|-------------
| 0Bit  |  i/o   |  TRIANGLE
| 1Bit  |  i/o   |  CIRCLE
| 2Bit  |  i/o   |  CROSS
| 3Bit  |  i/o   |  SQUARE
| 4Bit  |  i/o   |  UP
| 5Bit  |  i/o   |  RIGHT
| 6Bit  |  i/o   |  DOWN
| 7Bit  |  i/o   |  LEFT
|-------|--------|-------------

8Byte ButtonAssign2
|-------|--------|-------------
|  Bit  |  Data  | Information
|-------|--------|-------------
| 0Bit  |  i/o   |  L1
| 1Bit  |  i/o   |  L3
| 2Bit  |  i/o   |  R1
| 3Bit  |  i/o   |  R3
| 4Bit  |  i/o   |  SELECT
| 5Bit  |  i/o   |  START
| 6Bit  |  i/o   |  PS
| 7Bit  |  i/o   |  N/A
|-------|--------|-------------
*/