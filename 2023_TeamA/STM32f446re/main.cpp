//-----宣言開始-----// ピンの宣言時にu+200b ゼロ幅スペースに注意
#include "mbed.h"
#include "SerialCtrl.h"   //シリアル通信ライブラリ
#include "math.h"
#include <cstdint>

/*****UART通信*****/
Serial pc(USBTX, USBRX, 115200); //PCモニタ用
Serial PS3(PB_6, PB_7);          //PS3のシリアル(tx:送信 TIM4_CH1,rx:受信 TIM4_CH2)
Timer stim;                      //シリアルのタイマー設定
SerialCtrl wire(&PS3, &stim);    //(読みたいコントローラーのアドレス,タイマーのアドレス)

/*****CAN通信*****/
CAN can(PA_11,PA_12); // CANの宣言(RD, TD)
CANMessage msg;       // CAN送信用のバッファの宣言(初期値 8byte : char型の要素数8個の配列)

/*****基盤LED*****/
DigitalOut bord_leds [] = {
    DigitalOut(PC_0),
    DigitalOut(PA_15),
    DigitalOut(PC_3)
};

/*****テープLED*****/
PwmOut rgb_leds [] = {
    PwmOut(PA_0),  //TIM2_CH1 R
    PwmOut(PA_1),  //TIM2_CH2 B
    PwmOut(PA_10)  //TIM1_CH3 G
};
typedef enum{ //FSM
    EMSstg1 = 0U,
    EMSstg2
} TAPELED_EMSver_STATUS;
uint8_t start_case_new = 0U;
uint8_t start_case_old = 0U;
typedef enum{ //FSM
    Team_RED = 0U,
    Team_BLUE,
} TEAM_Color_STATUS;
DigitalIn team_color(PC_1);

/*****足回り*****/
Timer m_tim; //move用 インターバル用タイマー
#define dv 0.05 //台形制御のゲイン
DigitalOut move_digs[] = {
    DigitalOut(PB_8),
    DigitalOut(PB_15),
    DigitalOut(PA_6),
    DigitalOut(PC_5)
};
PwmOut move_pwms[] = {
    PwmOut(PC_9_ALT0),      //TIM8_CH4  LFront
    PwmOut(PC_8),           //TIM3_CH3  RF
    PwmOut(PB_9_ALT1),      //TIM11_CH1 LBack
    PwmOut(PC_6_ALT0)       //TIM8_CH1  RB
};

#define MEC_L1 1.0
#define MEC_L2 1.0
#define MEC_R 1.0

float motor_dir[4] = {
    1.0,
    1.0,
    1.0,
    1.0
};
uint8_t ps_case_new = 0U;
uint8_t ps_case_old = 0U;
uint8_t xy_chg_n = 0;//y座標変更用
Timer xy_chg_tim;

/*****昇降機構*****/
Timer elv_tim; //lift用 インターバル用タイマー
DigitalOut lift_dig(PA_7);
PwmOut lift_pwm(PC_7_ALT0); //TIM8_CH2
typedef enum{ //FSM
    ELV_STANDBY = 0U, //初期設定
    ELV_ACTIVE //昇降
} ELV_STATUS;

/*****角材越え*****/
Timer rope_tim;
DigitalOut rope_dig(PA_9);
PwmOut rope_pwm(PB_4); //TIM3_CH1
typedef enum{ //FSM
    ROPE_STANDBY = 0U, //初期設定
    ROPE_ACTIVE //ロープ昇降
}ROPE_STATUS;

/*****緊急停止ボタン*****/
Timer ems_tim; //ems用　インターバル用タイマー
uint8_t ems_xn = 0;
uint8_t ems_xp = 0;
DigitalIn ems(PB_12);
typedef enum{ //FSM
    EMS_STANDBY = 0U,//初期設定
    EMS_ACTIVE, //ランプ発光
} EMERGENCY_STATUS;

/*****button*****/
int  right = 0, up = 0, down = 0, triangle = 0, circle = 0, cross = 0, square = 0; 
int  l1 = 0, r1 = 0, start = 0, select = 0, ps = 0;

//-----宣言終了-----//

/*****map関数*****/
double clamp(double v, double low, double high){
    return min(max(v, low), high);
}

/*****ボタン判定簡略用関数*****/
void input(){
    if(wire.get()){
        triangle = (wire.data[B1] & 0x01);
        circle = (wire.data[B1] & 0x02)>>1;
        cross = (wire.data[B1] & 0x04)>>2;
        square = (wire.data[B1] & 0x08)>>3;
        up = (wire.data[B1] & 0x10)>>4;
        right = (wire.data[B1] & 0x20)>>5;
        down = (wire.data[B1] & 0x40)>>6;
        l1 = (wire.data[B2] & 0x01);
        r1 = (wire.data[B2] & 0x04)>>2;
        select = (wire.data[B2] & 0x10)>>4;
        start = (wire.data[B2] & 0x20)>>5;
        ps = (wire.data[B2] & 0x40)>>6;
    }
}

//テープLED(pullup)用関数
void tapeled(EMERGENCY_STATUS *_EMS_STATUS, TAPELED_EMSver_STATUS *_TAPELED_EMSver_STATUS, TEAM_Color_STATUS *_TEAM_Color_STATUS, unsigned int *t_ctr2, unsigned int *tmp){
    uint16_t i;
    switch(*_EMS_STATUS){
        case EMS_STANDBY:
            if(team_color){
                *_TEAM_Color_STATUS = Team_RED;
            }else{
                *_TEAM_Color_STATUS = Team_BLUE;
            }
            switch(*_TEAM_Color_STATUS){
                case Team_RED:
                    rgb_leds[0] = 0.0;
                    rgb_leds[1] = 1.0;
                    rgb_leds[2] = (xy_chg_n)?0.5:1.0;
                    break;
                case Team_BLUE:
                    rgb_leds[0] = 1.0;
                    rgb_leds[1] = 0.3;
                    rgb_leds[2] = (xy_chg_n)?0.0:1.0;
                    break;
            }
            break;
        case EMS_ACTIVE:
            i = *t_ctr2 % 100;
            rgb_leds[1] = rgb_leds[2] = 1.0;
            switch(*_TAPELED_EMSver_STATUS){
                case EMSstg1:
                    *tmp = i;
                    rgb_leds[0] = (double)(1.0 - (*tmp / 50.0));
                    if(50 < i){
                        *_TAPELED_EMSver_STATUS = EMSstg2;
                    }
                    break;
                case EMSstg2:
                    *tmp = i - 50;
                    rgb_leds[0] = (double)(*tmp / 50.0);
                    if(0 == i){
                        *_TAPELED_EMSver_STATUS = EMSstg1;
                    }
                    break;
            }
            (*t_ctr2)++;
            break;
    }
}

void mecanum_move(float *motor_out, float x, float y, float Vroll){

    x /= 128.0;
    y /= 128.0;
    float tmp = y;

    Vroll /= 128.0;

    float input_arr[3] = {x, y, Vroll};
    float calc_arr[4][3] = {
        {1.0, 1.0, -1.0*(MEC_L1+MEC_L2)},
        {1.0, -1.0, -1.0*(MEC_L1+MEC_L2)},
        {1.0, 1.0, 1.0*(MEC_L1+MEC_L2)},
        {1.0, -1.0, 1.0*(MEC_L1+MEC_L2)}
    };
    
    for(int i=0;i<4;++i)
        motor_out[i] = 0.0;

    for(int i=0;i<4;++i){
        for(int j=0;j<3;++j){
            motor_out[i] += calc_arr[i][j] * input_arr[j];
        }
    }
    for(int i=0;i<4;++i){
        motor_out[i] *= MEC_R;
    }
}

//足回り機構用関数
void movement(float *motor_out,double *x,double *y,float *Vroll,float *cur_speed){//,double *hl_speed,unsigned int *hl){//,float *r,float *theta){
    int8_t i, j;

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
    if(wire.data[0] >= 97 && wire.data[0] <= 157) { //ニュートラルの設定
        *x = 0;
    } else {
        *x = (-1)*((float)(wire.data[0]) - 127);// * (*hl_speed) ; //x軸のモータ出力設定
        *x *= (xy_chg_n)?-1:1;
    }

    //マシンの動き(LY)
    // wire.data[1] → LY
    if(wire.data[1] >= 97 && wire.data[1] <= 157) { //ニュートラルの設定
        *y = 0;
    } else {
        *y = ((float)(wire.data[1]) - 128);// * (*hl_speed); //y軸のモータ出力設定
        *y *= (xy_chg_n)?-1:1;    
    }

    // マシンの回転速度(Vroll) アナログスティック(Rスティック)ver.
    // wire.data[2] → RX
    if(wire.data[2] >= 97 && wire.data[2] <= 157) { //ニュートラルの設定
        *Vroll = 0;
    } else {
        *Vroll = ((float)(wire.data[2]) - 127) / 2;// * (*hl_speed); //回転速度の出力設定
    }
    mecanum_move(motor_out, *x, *y, *Vroll);
    for(int i = 0; i < 4; ++i){
        motor_out[i] *= motor_dir[i];
    }
    
    /*for(int i = 0; i < 4; i++){
        pc.printf("%f ", motor_out[i]);
    }
    pc.putc('\n');
    */
    /*
    for(int i=0;i<4;i+=2){
        motor_out[i] *= (-1); //必要に応じて出力を反転
    }*/

    //台形制御
    for(i=0;i<4;++i){
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
        for(j=0;j<4;++j){
            if(((0<=cur_speed[j])&&(cur_speed[j]<(float)dv))&&(motor_out[j]==(float)0.0)){
                cur_speed[j] = 0.0;
            }
        }
    }
    //信号線への出力
    for(i=0;i<4;++i){
        if(cur_speed[i]>0){
            move_digs[i] = 0;
            move_pwms[i] = 0 + cur_speed[i];  //cur_speedの出力が正の場合
        }else{
            move_digs[i] = 1;
            move_pwms[i] = 1 + cur_speed[i];  //cur_speedの出力が負の場合( 1 - motor_out で正の値にしている)
        }
    }
    

}

//昇降機構用関数
void lift(ELV_STATUS *_ELV_STATUS, unsigned int *l_ctr1){
    switch(*_ELV_STATUS){
        case ELV_STANDBY:
            lift_dig = 0;
            lift_pwm = 0;
            if((triangle || cross) && ((double)elv_tim.read() > 0.3)){
            //if(((triangle && (double)(elv_tim.read())) > 0.3)||((cross && (double)(elv_tim.read())) > 0.3)){
                *_ELV_STATUS = ELV_ACTIVE;
            }
            break;
        case ELV_ACTIVE:
            *l_ctr1 = 3;
            if(square){
                *l_ctr1 = 0;
                elv_tim.stop();  //インターバルをストップ
                elv_tim.reset(); //インターバルをリセット
                elv_tim.start(); //インターバルをスタート
            }
            if(triangle) *l_ctr1 = 1;
            if(cross)    *l_ctr1 = 2;
            switch(*l_ctr1){
                case 0:
                    *_ELV_STATUS = ELV_STANDBY;
                    break;
                case 1:
                    lift_dig = 0;
                    lift_pwm = 0.75;
                    break;
                case 2:
                    lift_dig = 1;
                    lift_pwm = 0.7;
                    break;
                case 3:
                    lift_dig = 0;
                    lift_pwm = 0;
                    break;
            }
            break;
    }
}

//角材越え機構用関数
void rope(ROPE_STATUS *_ROPE_STATUS, unsigned int *r_ctr1){
    switch(*_ROPE_STATUS){
        case ROPE_STANDBY:
            rope_dig = 0;
            rope_pwm = 0;
            if((up || down) && ((double)rope_tim.read() > 0.3)){
            //if(((triangle && (double)(elv_tim.read())) > 0.3)||((cross && (double)(elv_tim.read())) > 0.3)){
                *_ROPE_STATUS = ROPE_ACTIVE;
            }
            break;
        case ROPE_ACTIVE:
            *r_ctr1 = 3;
            if(right){
                *r_ctr1 = 0;
                rope_tim.stop();  //インターバルをストップ
                rope_tim.reset(); //インターバルをリセット
                rope_tim.start(); //インターバルをスタート
            }
            if(up) *r_ctr1 = 1;
            if(down)    *r_ctr1 = 2;
            switch(*r_ctr1){
                case 0:
                    *_ROPE_STATUS = ROPE_STANDBY;
                    break;
                case 1:
                    rope_dig = 0;
                    rope_pwm = 1.0;
                    break;
                case 2:
                    rope_dig = 1;
                    rope_pwm = 0.0;
                    break;
                case 3:
                    rope_dig = 0;
                    rope_pwm = 0;
                    break;
            }
            break;
    }
}

int main(){
    EMERGENCY_STATUS _EMS_STATUS = EMS_STANDBY;
    ELV_STATUS _ELV_STATUS = ELV_STANDBY;
    ROPE_STATUS _ROPE_STATUS = ROPE_STANDBY;
    TEAM_Color_STATUS _TEAM_Color_STATUS;
    TAPELED_EMSver_STATUS _TAPELED_EMSver_STATUS = EMSstg1;
    uint8_t i;

    /*****CAN通信*****/
    can.frequency(900000);     // CANの周波数の設定(送信側と合わせる)
    char can_msg[3] = {0,0,0}; //CAN通信用配列

    /*****シリアル通信*****/
    /*****Controller*****/
    PS3.baud(38400);        //ボードレートは38400(FEP02を使う場合)
    stim.reset();           //タイマーリセット
    stim.start();           //タイマースタート
    wire.param(0xAF, 0xED); //先頭アドレスと終端アドレスの設定
    int fail = 0;           //通信チェック用カウンターの初期化

    /*****PWM信号(足回り用)の周波数設定*****/
    for(i=0;i<4;++i){
        move_pwms[i].period_us(83);
    }
    /*****足回り用各種変数*****/
    float motor_out[4];              //モータ出力用変数
    float cur_speed[4] = {0,0,0,0};  //台形駆動用変数
    double x, y;                     //各座標用の変数
    float Vroll;                     //, r, theta;   //各座標用の変数
    bool r1_past = 0, r1_new;

    //昇降機構
    /*****PWM信号(昇降用)の初期設定*****/
    lift_pwm.period_us(83);
    /*****昇降機構用変数*****/
    unsigned int l_ctr1 = 0;         //昇降機構用カウンター変数

    //ロープ機構
    /*****PWM信号(ロープ用)の初期設定*****/
    rope_pwm.period_us(83);
    /*****ロープ機構用変数*****/
    unsigned int r_ctr1 = 0;         //ロープ機構用カウンター変数

    //テープLED
    /*****PWM信号(テープLED用)の初期設定*****/
    for(i=0;i<3;++i)
        rgb_leds[i].period_us(3000);
    /*****テープLED用各種変数*****/

    unsigned int tmp = 0;            //tapeled用ダミー変数
    unsigned int t_ctr1 = 0;         //tapeled用STDカウンター変数
    unsigned int t_ctr2 = 0;         //tapeled用EMSカウンター変数

    //緊急停止
    unsigned int c = 0;              //物理緊急停止用カウンター
    unsigned int tmp_e = 0;          //物理緊急停止用カウンター

    m_tim.stop();
    m_tim.reset();
    m_tim.start();
    elv_tim.stop();
    elv_tim.reset();
    elv_tim.start();
    rope_tim.stop();
    rope_tim.reset();
    rope_tim.start();
    ems_tim.stop();
    ems_tim.reset();
    ems_tim.start();
    xy_chg_tim.stop();
    xy_chg_tim.reset(); 
    xy_chg_tim.start();

    while (1){
        if(0 == ems){                  //物理緊急停止
            _EMS_STATUS = EMS_ACTIVE;
            ems_xn |= 0x01;
        }else{
            if(ems_xn & 0x01){
                ems_xn &= ~0x01;
            }
        }
        if(wire.get()){
            if(ems_xn & 0x04){
                ems_xn &= ~0x04;
            }
            fail = 0;          //通信失敗のカウントをリセット
            bord_leds[0] = 1;  //通信成功で基板のLEDが点灯
            input();           //記号ボタンを判定するため
            r1_new = (r1)?1:0;
            if(select){
                NVIC_SystemReset();    
            }
            if((ps) && (xy_chg_tim.read() > (double)0.3)){
                ps_case_new ^= 1;
                xy_chg_tim.stop();
                xy_chg_tim.reset();
                xy_chg_tim.start();
                switch(ps_case_new - ps_case_old){
                    case -1:
                        if(xy_chg_n & 0x01){
                            xy_chg_n &= ~0x01;
                        }
                        break;
                    case 1:
                        xy_chg_n |= 0x01;
                        break;
                }
                ps_case_old = ps_case_new;
            }
            if((start) && (ems_tim.read() > (double)0.3)){
                start_case_new ^= 1;
                ems_tim.stop();  //(停止)インターバルをストップ
                ems_tim.reset(); //(停止)インターバルをリセット
                ems_tim.start(); //(停止)インターバルを開始
                switch(start_case_new - start_case_old){
                    case -1:
                        if(ems_xn & 0x02){
                            ems_xn &= ~0x02; 
                        }
                        break;
                    case 1:
                        _EMS_STATUS = EMS_ACTIVE;
                        ems_xn |= 0x02;
                        break;
                }
                start_case_old = start_case_new;
            }
        }else{
            bord_leds[0] = 0;           //通信失敗で基板のled1消灯
            fail++;                     //通信失敗のカウントを1増やす
            if (fail > 10)
            { //通信失敗の判定は5回程度がベスト(場合によっては増やすことも)
                //printf("false\n");//画面へのエラーの出力
                _EMS_STATUS = EMS_ACTIVE;
                ems_xn |= 0x04;
            }
        }
        switch(_EMS_STATUS){
            case EMS_STANDBY://遠隔or物理緊急停止されていないとき
                bord_leds[2] = 0;
                bord_leds[1] = 1;
                //足回り機構
                if((r1_new ^ r1_past)||r1){
                    if(!r1_new){
                        for(int i=0;i<2;i++){
                            motor_out[i] = 0;
                            cur_speed[i] = 0;
                        }
                        motor_out[2] = motor_out[3] = 0;
                        cur_speed[2] = 1.0;
                        cur_speed[3] = -1.0;
                        //pc.printf("u_tan\n");
                    }
                    move_digs[0] = move_digs[1] = 1;
                    move_pwms[0] = move_pwms[1] = 1.0;
                    move_digs[2] = 0;
                    move_digs[3] = 1;
                    move_pwms[2] = 1.0;
                    move_pwms[3] = 0.0;
                }else{
                    movement(motor_out,&x,&y,&Vroll,cur_speed);//,&hl_speed,&hl);
                }
                //昇降機構
                lift(&_ELV_STATUS, &l_ctr1);            
                //ロープ機構
                rope(&_ROPE_STATUS, &r_ctr1);
                break;
            case EMS_ACTIVE://通信エラーor遠隔・物理緊急停止されているとき
                bord_leds[2] = 1;
                bord_leds[1] = 0;
                //全信号線への出力停止
                for(i=0;i<3;++i){
                    msg.data[i] = (char)0x00;
                }
                can.write(msg);
                for(i=0;i<4;++i){
                    move_digs[i] = 1.0;
                    move_pwms[i] = 1.0;
                }
                lift_dig = lift_pwm = 0.0;
                rope_dig = rope_pwm = 1.0;

                //タイマーリセット
                m_tim.stop();
                m_tim.reset();
                elv_tim.stop();
                elv_tim.reset();
                rope_tim.stop();
                rope_tim.reset();

                break;
        }
        if((ems_xn ^ ems_xp)&&(ems_xn == 0)){
            _EMS_STATUS = EMS_STANDBY;
            NVIC_SystemReset();
        }
        r1_past = r1_new;
        ems_xp = ems_xn;
        tapeled(&_EMS_STATUS, &_TAPELED_EMSver_STATUS, &_TEAM_Color_STATUS, &t_ctr2, &tmp);
    }
}
