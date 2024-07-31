//全方位移動ライブラリを使ったロボットの足回りを動かすプログラム
//※OmniMoveライブラリやSerialCtrlライブラリの使用を前提としています
#include "mbed.h"
#include "math.h"
#include "OmniMove.h"   //全方向移動ライブラリ
#include "SerialCtrl.h" //シリアル通信ライブラリ

#define dv 0.05

CAN can(PA_11,PA_12); // CANの宣言(RD, TD)
CANMessage msg;       // CAN送信用のバッファの宣言(初期値 8byte : char型の要素数8個の配列)

//回路:2セルだから7.4v モーター:14.8v
//全方位移動ライブラリの宣言
OmniMove omni;

//モータ(Digital)(モータ正転か逆転足回り機構)
DigitalOut move_dig0(PB_8), // motor[0]足回り
           move_dig1(PB_15),       // motor[1]
           move_dig2(PA_6),        // motor[2]
           move_dig3(PC_5);        // motor[3]
/*
//モータ(Digital)(モータ正転か逆転射出機構)
DigitalOut launch_dig4(PA_7),
           launch_dig5(PB_14),
           launch_dig6(PA_9),
           launch_dig7(PC_4);
*/
//モータ(Digital)(モータ正転か逆転か昇降機構)
//DigitalOut upd_dig8(PB_13);
DigitalOut upd_dig8(PA_7);

//モータ(Digital)(モータ正転か逆転か装填機構)
//DigitalOut load_dig9(PC_2),
//           load_dig10(PC_3);
DigitalOut load_dig9(PA_7),
           load_dig10(PA_9);

//button
int  triangle = 0, circle = 0, cross = 0, square = 0, start = 0, select = 0;

//モータ(PWM)(モータ出力足回り機構)
PwmOut move_pwm0(PC_9_ALT0), // motor[0]
       move_pwm1(PC_8),    // motor[1] 
       move_pwm2(PB_9_ALT1),    // motor[2]
       move_pwm3(PC_6_ALT0);    // motor[3}
/*
//モータ(pwm)(モータ出力射出機構)
PwmOut launch_pwm4(PC_7_ALT0),
       launch_pwm5(PB_10),
       launch_pwm6(PB_4),
       launch_pwm7(PB_5);
*/
//モータ(pwm)(モータ出力昇降機構)
//PwmOut upd_pwm8(PB_2);
PwmOut upd_pwm8(PC_7_ALT0);

//モータ(pwm)(モータ出力装填機構)
//PwmOut load_pwm9(PA_8),
//       load_pwm10(PB_1_ALT0);
PwmOut load_pwm9(PC_7_ALT0),
       load_pwm10(PB_4);

//基板led
DigitalOut led1(PA_15),
           led2(PA_13);

//緊急停止ボタン
DigitalIn stop(PB_12);

//リミットスイッチ
DigitalIn high(PC_1),
          low(PB_0);

//PCデバック用
Serial pc(USBTX, USBRX, 9600);

//シリアル通信(PS3)
Serial PS3(PB_6, PB_7);       // PS3のシリアル(tx:送信,rx:受信)
Timer stim;                   //シリアルのタイマー設定
SerialCtrl wire(&PS3, &stim); //(読みたいコントローラーのアドレス,タイマーのアドレス)

//テープLED
PwmOut pwmR(PA_0),
       pwmG(PA_10),
       pwmB(PA_1);

//インターバル用タイマー
Timer j; //launcher用
Timer k; //load用
Timer l; //updown用
Timer m; //system用

//緊急停止FSM
typedef enum{
  S1 = 0U,//初期設定
  E1, //光度上昇
  E2 //光度下降
} EMERGENCY_STATUS;

//装填機構FSM
typedef enum{
    LD_STANDBY = 0U, //待機
    LOAD //装填
} LOAD_STATUS;

//射出機構FSM
typedef enum{
    LH_STANDBY = 0U, //待機
    LAUNCH //射出
} LAUNCH_STATUS;

//昇降機構FSM
typedef enum{
    UD_STANDBY = 0U,//待機
    UPDOWN //昇降
} UPDOWN_STATUS;

//テープledFSM
typedef enum{
    s1 = 0U,//r_
    s2,
    s3,
    s4,
    s5,
    s6,
    s7,
    s8,
    s9,
    s10
} RAINBOW_STATUS;

double clamp(double v, double low, double high){
    return min(max(v, low), high);
}

//ボタン判定簡略用関数
void input(){
    if(wire.get()){
    //if(1){
    //wire.data[6] = 0x03;
    triangle = (wire.data[B1] & 0x01);
    circle = (wire.data[B1] & 0x02);
    cross = (wire.data[B1] & 0x04);
    square = (wire.data[B1] & 0x08);
    select = (wire.data[B2] & 0x10);
    start = (wire.data[B2] & 0x20);
    /*
        return 1;
    }
    else{
        return -1;
    }
    */
    }
}

//昇降機構用関数
void updown(UPDOWN_STATUS *upd_status, unsigned int *b){
    switch(*upd_status){
        case UD_STANDBY:
            upd_dig8 = 0;
            upd_pwm8 = 0.0;
            if(((triangle && (double)(l.read())) > 0.3)||((cross && (double)(l.read())) > 0.3)){
                *upd_status = UPDOWN;
            }
            break;
        case UPDOWN:
            *b = 3;
            if(square){
                *b = 0;
                l.stop();  //インターバルをストップ
                l.reset(); //インターバルをリセット
                l.start(); //インターバルをスタート
            }
            if(high==1) if(triangle) *b = 1;
            if(low==1) if(cross) *b = 2;
            switch(*b){
                case 0:
                    *upd_status = UD_STANDBY;
                    break;
                case 1:
                    upd_dig8 = 1;
                    upd_pwm8 = 0.7;
                    break;
                case 2:
                    upd_dig8 = 0;
                    upd_pwm8 = 0.7;
                    break;
                case 3:
                    upd_dig8 = 0;
                    upd_pwm8 = 0;
                    break;
            }
            break;
    }
}

//装填機構用関数
void load(LOAD_STATUS *load_status){
    switch(*load_status){
        case LD_STANDBY:
            load_dig9 = 1;
            load_dig10 = 0;
            load_pwm9 = (float)1.0;
            load_pwm10 = (float)0.0;
            if(circle && (double)(k.read()) > 0.3){
                k.stop();  //インターバルをストップ
                k.reset(); //インターバルをリセット
                k.start(); //インターバルをスタート
                *load_status = LOAD;
            }
            break;
        case LOAD:
            load_pwm9 = (float)0.5;
            load_pwm10 = (float)0.5;
            if(circle && (double)(k.read()) > 0.3){
                k.stop();  //インターバルをストップ
                k.reset(); //インターバルをリセット
                k.start(); //インターバルをスタート
                *load_status = LD_STANDBY;
            }
            break;
    }
}

//テープLED(pullup)用関数
void rainbow(RAINBOW_STATUS *led_status,unsigned int *a,unsigned int *tmp){
    switch(*led_status){
        case s1:
            *tmp = *a%150;
            pwmG = (double)(1.0-(*tmp/25.0));
            if(25<(*a%150)){
                *led_status = s2;
            }
            break;
        case s2:
            *tmp = *a%150 - 25;
            pwmR = (double)(*tmp/25.0);
            if(50<(*a%150)){
                *led_status = s3;
            }
            break;
        case s3:
            *tmp = *a%150 - 50;
            pwmB = (double)(1.0-(*tmp/25.0));
            if(75<(*a%150)){
                *led_status = s4;
            }
            break;
        case s4:
            *tmp = *a%150 - 75;
            pwmG = (double)(*tmp/25.0);
            if(100<(*a%150)){
                *led_status = s5;
            }
            break;
        case s5:
            *tmp = *a%150 - 100;
            pwmR = (double)(1.0-(*tmp/25.0));
            if(125<(*a%150)){
                *led_status = s6;
            }
            break;
        case s6:
            *tmp = *a%150 - 125;
            pwmB = (double)(*tmp/25.0);
            if(0==(*a%150)){
                *led_status = s1;
            }
            break;
    }
    (*a)++;
}

//射出機構用関数
void launcher(LAUNCH_STATUS *launch_status,char *can_msg){
    static double speed;
    switch (*launch_status){
        case LH_STANDBY:
            can_msg[1] |= (char)0x00;
            msg.data[1] = can_msg[1];
            speed = 0.0;
            /*
            launch_dig4 = 0; //正転(1)
            launch_dig5 = 0; //正転(2)
            //launch_dig5 = 1; //逆転(1)
            launch_dig6 = 0; //正転(2)
            launch_dig7 = 1; //逆転(2)
            
            launch_pwm4 = launch_pwm6 = 0.0;
            launch_pwm5 = 0.0;
            //launch_pwm5 = 1.0; //逆転
            launch_pwm7 = 1.0;
            */

            //0.1秒後にステータス変更
            if((double)(j.read()) > 0.1){
                j.stop();  //インターバルをストップ
                j.reset(); //インターバルをリセット
                j.start(); //インターバルを開始
                *launch_status = LAUNCH;
            }
            break;
        case LAUNCH:
            led2 = 1;
            can_msg[1] |= (char)0x08;
            if((wire.data[B1] & UP)&&((double)(j.read()) > 0.2)){
                speed += 0.05;
                j.stop();  //インターバルをストップ
                j.reset(); //インターバルをリセット
                j.start(); //インターバルを開始
            }
            if((wire.data[B1] & DOWN)&&((double)(j.read()) > 0.2)){
                speed -= 0.05;
                j.stop();  //インターバルをストップ
                j.reset(); //インターバルをリセット
                j.start(); //インターバルを開始
            }
            speed = clamp(speed, 0.0, 1.0);
            can_msg[2] = (unsigned char)(speed*255);
            //can_msg[2] = (unsigned char)speed;
            //can_msg[2] = (unsigned char)5;
            for(int i=0;i<3;i++){
                msg.data[i] = can_msg[i];
            }
            /*
            launch_pwm4 = launch_pwm6 = speed;
            launch_pwm5 = speed;
            //launch_pwm5 = 1.0 - speed; //逆転
            launch_pwm7 = 1.0 - speed;
            */
            break;
    }
}

//足回り機構用関数
void movement(float *motor_out,double *x,double *y,float *Vroll,float *cur_speed){//,float *r,float *theta){
    
    //      アナログスティックの座標 (DUALSHOCK3 and DUALSHOCK4)
    //                       0
    //                       |
    //                       |
    //                       | 
    //             0 ----------------> 255 LX or RX
    //                       |
    //                       | 
    //                       |
    //                       V
    //                      255
    //                     LY or RY
            
    //マシンの動き(LX)
    // wire.data[0] → LX
    if(wire.data[0] >= 117 && wire.data[0] <= 137) { //ニュートラルの設定
        *x = 0;
    } else {
        *x = -1*((float)(wire.data[0]) - 127) / 128; //x軸のモータ出力設定
    }

    //マシンの動き(LY)
    // wire.data[1] → LY
    if(wire.data[1] >= 117 && wire.data[1] <= 137) { //ニュートラルの設定
        *y = 0;
    } else {
        *y = (float(wire.data[1]) - 128) * (-1) / 128; //y軸のモータ出力設定
    }

    // マシンの回転速度(Vroll) アナログスティック(Rスティック)ver.
    // wire.data[2] → RX
    if(wire.data[2]>=117 && wire.data[2]<=137) { //ニュートラルの設定
        *Vroll = 0;
    } else {
        *Vroll = ((float(wire.data[2]) - 127) / 128 / 2); //回転速度の出力設定
    }
    
    //r
    //*r = (float)sqrt(std::pow(*x, 2.0)+std::pow(*y, 2.0));
    
    //シータ
    //*theta = std::atan2(*y,*x);
    
    // マシンの回転速度(Vroll) L2 & R2ver.
    // wire.data[4] → L2, wire.data[5] → R2
    /*
    if(wire.data[4]>127){
        Vroll = ((float(wire.data[4]) - 127)) / 128 / 2; //反時計回り
    }else if(wire.data[5]>127){
        Vroll = ((float(wire.data[5]) - 127)) / 128 *(-1) / 2; //時計回り
    }else{
        Vroll = 0; // ニュートラル
    }
    */
    
    //printf("x:%9.6f y:%9.6f Vroll:%9.6f ",x,y,Vroll); //各座標の値チェック用
    
    omni.input_cartesian(*x,*y,*Vroll); //入力関数(直交座標)に代入
    //omni.input_polar(*r,*theta,*Vroll);//入力関数(極座標)に代入
    omni.output(motor_out);          //出力関数に代入(配列の先頭アドレスを代入)
    
    //台形制御
    for(int i=0;i<4;i++){
        if(motor_out[i]>(float)0.0&&(motor_out[i]-cur_speed[i])>(float)0.0){
            cur_speed[i] += (float)dv;
        }
        if(motor_out[i]<(float)0.0&&(motor_out[i]-cur_speed[i])<(float)0.0){
            cur_speed[i] -= (float)dv;
        }
        if(cur_speed[i]>(float)0.0&&(cur_speed[i]-motor_out[i])>(float)0.0){
            cur_speed[i] -= (float)dv;
        }
        if(cur_speed[i]<(float)0.0&&(cur_speed[i]-motor_out[i])<(float)0.0){
            cur_speed[i] += (float)dv;
        }
        if(((0<=cur_speed[0])&&(cur_speed[0]<(float)dv))&&(motor_out[0]==(float)0.0)){
            cur_speed[0] = 0.0;
        }
        if(((0<=cur_speed[1])&&(cur_speed[1]<(float)dv))&&(motor_out[1]==(float)0.0)){
            cur_speed[1] = 0.0;
        }
        if(((0<=cur_speed[2])&&(cur_speed[2]<(float)dv))&&(motor_out[2]==(float)0.0)){
            cur_speed[2] = 0.0;
        }
        if(((0<=cur_speed[3])&&(cur_speed[3]<(float)dv))&&(motor_out[3]==(float)0.0)){
            cur_speed[3] = 0.0;
        }
    }
    
    /*
    for(int i=0;i<4;i++){
        motor_out[i] *= (-1); //必要に応じて出力を反転
    }
    */
    
    //信号線への出力
    if(cur_speed[0]>0){
        move_dig0 = 0;
        move_pwm0 = 0 + cur_speed[0]; //cur_speedの出力が正の場合
    }else{
        move_dig0 = 1;
        move_pwm0 = 1 + cur_speed[0]; //cur_speedの出力が負の場合( 1 - motor_out で正の値にしている)
    }
    
    if(cur_speed[1]>0){
        move_dig1 = 0;
        move_pwm1 = 0 + cur_speed[1]; 
    }else{
        move_dig1 = 1;
        move_pwm1 = 1 + cur_speed[1]; 
    }
    
    if(cur_speed[2]>0){
        move_dig2 = 0;
        move_pwm2 = 0 + cur_speed[2]; 
    }else{
        move_dig2 = 1;
        move_pwm2 = 1 + cur_speed[2]; 
    }
    
    if(cur_speed[3]>0){
        move_dig3 = 0;
        move_pwm3 = 0 + cur_speed[3];
    }else{
        move_dig3 = 1;
        move_pwm3 = 1 + cur_speed[3];
    }
    
    //モータ出力の確認
    /*
    int i = 0;
    while (i < 4){
        //printf("motor[%d]:%9.6f ", i, motor_out[i]);
        i++;
    }
    printf("\n");
    */
}
/*
void debug_out(LOAD_STATUS *load_status,LAUNCH_STATUS *launch_status,UPDOWN_STATUS *upd_status){
    pc.printf("ledR:%.3f G:%.3f B:%.3f ",(double)pwmR,(double)pwmG,(double)pwmB);
    pc.printf("buttons:0x%2X load_status:%2d launch_status:%2d upd_status:%2d ",wire.data[B1],(int)*load_status,(int)*launch_status,(int)*upd_status);
    pc.printf("suspension-d0:%d d1:%d d2:%d d3:%d updown-d8:%d load-d9:%d d10:%d ",(int)move_dig0,(int)move_dig1,(int)move_dig2,(int)move_dig3,(int)upd_dig8,(int)load_dig9,(int)load_dig10);
    pc.printf("suspension-p0:%.3f p1:%.3f p2:%.3f p3:%.3f updown-p8:%.3f load-p9:%.3f p10:%.3f \n\n\n",(double)move_pwm0,(double)move_pwm1,(double)move_pwm2,(double)move_pwm3,(double)upd_pwm8,(double)load_pwm9,(double)load_pwm10);
}
*/
//リセット・緊急停止用関数
void system(float *motor_out,char *can_msg,EMERGENCY_STATUS *stop_status){
    bool cz = 0;
    *stop_status = S1;
    can.write(msg);
    //printf("%d\n",can.write(msg));
    can_msg[1] = (char)0x00;
    if(select){
        NVIC_SystemReset();    
    }
    if(start){
        cz = 1;
    }
    if(cz && (m.read() > (double)0.3)){
        m.reset(); //(停止)インターバルをリセット
        m.start(); //(停止)インターバルを開始
        
        do{
            //全信号線への出力停止
            for(int i=0;i<3;i++){
                msg.data[i] = (char)0x00;
            }
            can.write(msg);
            move_dig0 = move_dig1 = move_dig2 = move_dig3 = 0;//launch_dig4 = launch_dig5 = launch_dig6 = launch_dig7 = 0;
            move_pwm0 = move_pwm1 = move_pwm2 = move_pwm3 = 0;//launch_pwm4 = launch_pwm5 = launch_pwm6 = launch_pwm7 = 0;
            upd_dig8 = load_dig9 = load_dig10 = 0;
            upd_pwm8 = load_pwm9 = load_pwm10 = 0;
            //tapeLED(pullup)
            pwmR = 0.0;
            pwmG = pwmB = 1.0;
            j.stop();
            j.reset();
            k.stop();
            k.reset();
            l.stop();
            l.reset();
            if(wire.get()){
                input();
                if(start && (m.read() > (double)0.3)){
                    cz = 0;
                }
            }
        }while(cz==1);
        NVIC_SystemReset();
    }
}

//物理緊急停止関数
void emergency(EMERGENCY_STATUS *stop_status,unsigned *c,unsigned int *tmp_e){
    switch(*stop_status){
        case S1:
            pwmR = 0.0;
            pwmG = pwmB = 1.0;
            *stop_status = E1;
            break;
        case E1:
            *tmp_e = *c%20;
            pwmR = (double)(*tmp_e/10.0);
            if(10<(*c%20)){
                *stop_status = E2;
            }
            break;
        case E2:
            *tmp_e = *c%20 - 10;
            pwmR = (double)(1.0-(*tmp_e/10.0));
            if(0==(*c%20)){
                *stop_status = E1;
            }
            break;
    }
    (*c)++;
}

//メイン関数
int main(){
    EMERGENCY_STATUS stop_status = S1;
    LOAD_STATUS load_status = LD_STANDBY;
    LAUNCH_STATUS launch_status = LH_STANDBY;
    RAINBOW_STATUS led_status = s1;
    UPDOWN_STATUS upd_status = UD_STANDBY;
    
    can.frequency(900000); // CANの周波数の設定(送信側と合わせる)
    char can_msg[3] = {0,0,0};//CAN通信用配列
    
    float motor_out[4];  //モータ出力用変数
    float cur_speed[4] = {0,0,0,0};  //台形駆動用変数
    double x, y; //各座標用の変数
    float Vroll;//, r, theta;   //各座標用の変数
    unsigned int b = 0;  //updown用の判定変数
    unsigned int a = 0;  //rainbow用カウンター
    unsigned int tmp = 0;//rainbow用変数(pwm)
    unsigned int c = 0;//物理緊急停止用カウンター
    unsigned int tmp_e = 0;//物理緊急停止用カウンター

    //シリアル通信の設定
    PS3.baud(38400);        //ボードレートは38400(FEP02を使う場合)
    stim.reset();           //タイマーリセット
    stim.start();           //タイマースタート
    wire.param(0xAF, 0xED); //先頭アドレスと終端アドレスの設定
    int fail = 0;           //通信チェック用カウンターの初期化

    //全方位移動ライブラリの初期設定
    omni.setup(4, -45); //車輪の数,車輪の角度(オムニ:45,メカナム:45)

    // PWM信号(モータ用)の周波数設定
    move_pwm0.period_us(83);
    move_pwm1.period_us(83);
    move_pwm2.period_us(83);
    move_pwm3.period_us(83);
    //launch_pwm4.period_us(83);
    //launch_pwm5.period_us(83);
    //launch_pwm6.period_us(83);
    //launch_pwm7.period_us(83);
    upd_pwm8.period_us(83);
    load_pwm9.period_us(83);
    load_pwm10.period_us(83);
    
    // PWM信号(テープLED用)の周波数設定
    pwmR.period_us(3000);
    pwmG.period_us(3000);
    pwmB.period_us(3000);
    j.stop();  //(射出)インターバルをストップ
    j.reset(); //(射出)インターバルをリセット
    j.start(); //(射出)インターバルを開始
    k.stop();  //(装填)インターバルをストップ
    k.reset(); //(装填)インターバルをリセット
    k.start(); //(装填)インターバルを開始
    l.stop();  //(昇降)インターバルをストップ
    l.reset(); //(昇降)インターバルをリセット
    l.start(); //(昇降)インターバルを開始
    m.stop();  //(停止)インターバルをストップ
    m.reset(); //(停止)インターバルをリセット
    m.start(); //(停止)インターバルを開始
    
    pwmR = 0.0;
    pwmG = 1.0;
    pwmB = 1.0;

    while (1){
        if(stop){
            rainbow(&led_status,&a,&tmp);
            if(wire.get()){ //通信成功の場合
                fail = 0; //通信失敗のカウントをリセット
                led1 = 1;  //通信成功で基板のLEDが点灯
                input();//記号ボタンを判定するため
                load(&load_status);//装填機構
                launcher(&launch_status,can_msg);//射出機構
                movement(motor_out,&x,&y,&Vroll,cur_speed);//,&r,&theta);//足回り機構
                updown(&upd_status,&b);//昇降機構
                //debug_out(&load_status,&launch_status,&upd_status);//デバック用
            }else{ //通信失敗の場合

                fail++; //通信失敗のカウントを1増やす
                pc.printf("error\n\n\n");

                if (fail > 10)
                { //通信失敗の判定は5回程度がベスト(場合によっては増やすことも)

                    led1 = 0;           //通信失敗で基板のled1消灯
                    pc.printf("false\n");//画面へのエラーの出力
                    a = 0;              //rainbow用カウンターを停止
                    tmp = 0;            //rainbow用変数(pwm)

                    //全信号線への出力停止
                    msg.data[0] = 0x00;
                    move_dig0 = move_dig1 = move_dig2 = move_dig3 = 0;//launch_dig4 = launch_dig5 = launch_dig6 = launch_dig7 = 0;
                    move_pwm0 = move_pwm1 = move_pwm2 = move_pwm3 = 0;//launch_pwm4 = launch_pwm5 = launch_pwm6 = launch_pwm7 = 0;
                    upd_dig8 = load_dig9 = load_dig10 = 0;
                    upd_pwm8 = load_pwm9 = load_pwm10 = 0;
                    pwmR = pwmG = pwmB = 0;
                    j.stop();
                    j.reset();
                    k.stop();
                    k.reset();
                    l.stop();
                    l.reset();
                }
            }
            system(motor_out,can_msg,&stop_status);
        }else{
            emergency(&stop_status,&c,&tmp_e);
        }
    }
}
