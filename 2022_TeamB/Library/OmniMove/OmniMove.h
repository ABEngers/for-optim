#ifndef OmniMove_H
#define OmniMove_H

#include "mbed.h"

/*
OmniMove.h
オムニ・メカナム等の全方位移動ベクトル演算ライブラリ
Designer: Watanabe Yuuto

このライブラリは極座標(or直交座標)で表したマシンの走行速度、進行方向から
各車輪の回転速度を演算するライブラリです。
3~8輪までの車輪数や車輪の位置などを指定できます。

・ホイールの位置関係(4つの場合)
  注.4つの場合は0,1,2,3とナンバリングする
       front
  ┌───────────────┐
  │     w1←       │
 l│       y       │r
 e│       ↑     w0│i
 f│↓      . →x   ↑│g
 t│w2             │h
  │               │t
  │       →w3     │
  └───────────────┘
         back       
この場合の配置では0番目の車輪軸の角度fstWheelAngは0度となる

・マシンの進行方向thetaについて
例)theta=0の場合→right方向に進む
   theta=90の場合→front方向に進む
   theta=-90の場合→back方向に進む
   theta=180(or theta=-180)の場合→left方向に進む
   
・マシンの回転速度Vrollについて
　値が正(>0)であれば反時計回り、値が負(<0)であれば時計回りにマシンが回転する

・使用例(4輪の場合)
#include "mbed.h"
#include "OmniMove/OmniMove.h" //インクルァぁドしてネ
 
OmniMove mekanamu; //TickerやTimerのように宣言
 
int main(){
    float motor_output[4];//それぞれのモータ出力変数(配列)を用意
    
    mekanamu.setup(4,45); //車輪の個数、位置を設定(1度宣言すればいい)
    
    while(1) {
        
        mekanamu.input_polar(0.5,90,0); //マシンの走行速度、進行方向、回転速度の入力
        
        //配列の先頭アドレスを渡すことでそれぞれのモータ出力変数に値を入力
        mekanamu.output(motor_output);
        
    }
}
*/

class OmniMove
{
public:
    /*
    セットアップ関数
    nWheel:車輪の個数(3~8),fstWheelAng:0番目のx軸と車輪軸のなす角度(deg) */
    void setup(int nWheel,float fstWheelAng);
    
    /*
    全方位移動入力関数(極座標)
    r:マシンの速度(0~1),theta:マシンの進行方向(角度deg ex.90,-45)
    Vroll:マシンの回転速度(-1~1)     */
    void input_polar(float r,float theta,float Vroll);
    
    /*
    マシンの傾き角度を考慮した全方位移動入力関数(極座標)
    MachineAng:マシンの現在角度(角度deg ex.90,-45)  */
    void input_polar(float r,float theta,float Vroll,float MachineAng);
    
    /*
    全方位移動入力関数(直交座標)
    x:マシンのx方向速度(-1~1),y:マシンのy方向速度(-1~1)
    Vroll:マシンの回転速度(-1~1)    */
    void input_cartesian(float x,float y,float Vroll);
    
    /*
    マシンの傾き角度を考慮した全方位移動入力関数(直交座標)
    MachineAng:マシンの現在角度(角度deg ex.90,-45)  */
    void input_cartesian(float x,float y,float Vroll,float MachineAng);
    
    /*
    全方位移動出力関数(アドレスよくわからん人用)
    引数:n番目の車輪の回転速度  */
    float output_(int n);
    
    /*
    全方位移動出力関数
    *v:車輪回転速度の配列の先頭アドレス  */
    void output(float *V);

private:
    int i,nWheel;
    float Vx_wheel[8],Vy_wheel[8],Vx,Vy,Vroll,r,theta;
    
    float limit(float min,float max,float _value);
    float conv_deg(float _rad);
    float conv_rad(float _deg);
};

#endif