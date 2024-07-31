#include "mbed.h"
#include "math.h"
#include "OmniMove.h"

void OmniMove::setup(int nWheel,float fstWheelAng){
    this->nWheel = limit(3,8,nWheel);
    
    for(i = 0;i < nWheel; i++){
        Vx_wheel[i] = -1 * sin( conv_rad( fstWheelAng + (360 / nWheel) * i ) );
        Vy_wheel[i] = cos( conv_rad( fstWheelAng + (360 / nWheel) * i ) );
    }
}

void OmniMove::input_polar(float r,float theta,float Vroll){
    r = limit(0,1,r);
    this->Vroll = limit(-1,1,Vroll);
    Vx = r * cos( conv_rad( theta ) ) * ( 1 - fabs(this->Vroll) );
    Vy = r * sin( conv_rad( theta ) ) * ( 1 - fabs(this->Vroll) );
}

void OmniMove::input_polar(float r,float theta,float Vroll,float MachineAng){
    r = limit(0,1,r);
    this->Vroll = limit(-1,1,Vroll);
    Vx = r * cos( conv_rad( theta - MachineAng ) ) * ( 1 - fabs(this->Vroll) );
    Vy = r * sin( conv_rad( theta - MachineAng ) ) * ( 1 - fabs(this->Vroll) );
}

void OmniMove::input_cartesian(float x,float y,float Vroll){
    x = limit(-1,1,x);
    y = limit(-1,1,y);
    this->Vroll = limit(-1,1,Vroll);
    this->r = limit(0,1,hypotf(x,y));
    this->theta = conv_deg(atan2(y,x));
    this->Vx = r * cos( conv_rad( this->theta ) ) * ( 1 - fabs(this->Vroll) );
    this->Vy = r * sin( conv_rad( this->theta ) ) * ( 1 - fabs(this->Vroll) );
}

void OmniMove::input_cartesian(float x,float y,float Vroll,float MachineAng){
    x = limit(-1,1,x);
    y = limit(-1,1,y);
    this->Vroll = limit(-1,1,Vroll);
    r = limit(0,1,hypotf(x,y));
    theta = conv_deg(atan2(y,x));
    Vx = r * cos( conv_rad( theta - MachineAng ) ) * ( 1 - fabs(this->Vroll) );
    Vy = r * sin( conv_rad( theta - MachineAng ) ) * ( 1 - fabs(this->Vroll) );
}

float OmniMove::output_(int n){
    if( this->nWheel < 8 ){
        return (float)(this->Vx*this->Vx_wheel[n] + this->Vy*this->Vy_wheel[n] + this->Vroll);
    }else{
        return 0;
    }
}

void OmniMove::output(float *V){
    for(i = 0;i < this->nWheel;i++){
        V[i] = (float)(Vx*Vx_wheel[i] + Vy*Vy_wheel[i] + Vroll);
    }
}

float OmniMove::limit(float min,float max,float _value){
    if(_value > max){
        return max;
    }else if(_value < min){
        return min;
    }else{
        return _value;
    }
}

float OmniMove::conv_deg(float _rad){
    return _rad * 180 / 3.14159265;
}

float OmniMove::conv_rad(float _deg){
    return 3.14159265 * _deg / 180;
}
