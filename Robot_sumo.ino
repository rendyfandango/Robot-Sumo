#include <Ps3Controller.h>

#define PWM_A 23      
#define EN_A 22
#define PWM_B 21      
#define EN_B 19

#define PWM_a 18      
#define EN_a 5
#define PWM_b 17      
#define EN_b 16

float LF_GAIN = 1.0;   
float RF_GAIN = 1.0;   

float LB_GAIN = 1.0;
float RB_GAIN = 1.0;

const int frequency = 10000;
const int resolution = 8;

const int chRightForward  = 0;
const int chRightBackward = 1;
const int chLeftForward   = 2;
const int chLeftBackward  = 3;

int joyX;
int joyY;

int speed_mode = 200; 
int turn_mode  = 120; 

int targetLeft = 0;
int targetRight = 0;

int currentLeft = 0;
int currentRight = 0;

const int deadZone = 15;

const int accel = 2;   
const int minPWM = 20; 

void rampMotor()
{
    if(currentLeft < targetLeft)
    {
        currentLeft += accel;
        if(currentLeft > targetLeft)
            currentLeft = targetLeft;
    }
    else if(currentLeft > targetLeft)
    {
        currentLeft -= accel;
        if(currentLeft < targetLeft)
            currentLeft = targetLeft;
    }

    if(currentRight < targetRight)
    {
        currentRight += accel;
        if(currentRight > targetRight)
            currentRight = targetRight;
    }
    else if(currentRight > targetRight)
    {
        currentRight -= accel;
        if(currentRight < targetRight)
            currentRight = targetRight;
    }
}
void driverON()
{
    digitalWrite(EN_A, HIGH);
    digitalWrite(EN_B, HIGH);
    digitalWrite(EN_a, HIGH);
    digitalWrite(EN_b, HIGH);
}

void driverOFF()
{
    ledcWrite(chRightForward,0);
    ledcWrite(chRightBackward,0);
    ledcWrite(chLeftForward,0);
    ledcWrite(chLeftBackward,0);

    digitalWrite(EN_A, LOW);
    digitalWrite(EN_B, LOW);
    digitalWrite(EN_a, LOW);
    digitalWrite(EN_b, LOW);
}
void motor(int kiri, int kanan)
{
    kiri  = constrain(kiri,-255,255);
    kanan = constrain(kanan,-255,255);


    if(kiri > 0)     
    {
        kiri = kiri * LF_GAIN;

        if(kiri > 255) kiri = 255;
        if(kiri < minPWM) kiri = minPWM;

        ledcWrite(chLeftForward,kiri);
        ledcWrite(chLeftBackward,0);
    }
    else if(kiri < 0)   
    {
        kiri = -kiri;

        kiri = kiri * LB_GAIN;

        if(kiri > 255) kiri = 255;
        if(kiri < minPWM) kiri = minPWM;

        ledcWrite(chLeftForward,0);
        ledcWrite(chLeftBackward,kiri);
    }
    else
    {
        ledcWrite(chLeftForward,0);
        ledcWrite(chLeftBackward,0);
    }


    if(kanan > 0)    
    {
        kanan = kanan * RF_GAIN;

        if(kanan > 255) kanan = 255;
        if(kanan < minPWM) kanan = minPWM;

        ledcWrite(chRightForward,kanan);
        ledcWrite(chRightBackward,0);
    }
    else if(kanan < 0)   
    {
        kanan = -kanan;

        kanan = kanan * RB_GAIN;

        if(kanan > 255) kanan = 255;
        if(kanan < minPWM) kanan = minPWM;

        ledcWrite(chRightForward,0);
        ledcWrite(chRightBackward,kanan);
    }
    else
    {
        ledcWrite(chRightForward,0);
        ledcWrite(chRightBackward,0);
    }
}


void setup()
{
    Serial.begin(115200);
    Ps3.begin("a0:5a:5c:bc:21:4c");
    pinMode(PWM_A,OUTPUT);
    pinMode(PWM_B,OUTPUT);
    pinMode(PWM_a,OUTPUT);
    pinMode(PWM_b,OUTPUT);

    pinMode(EN_A,OUTPUT);
    pinMode(EN_B,OUTPUT);
    pinMode(EN_a,OUTPUT);
    pinMode(EN_b,OUTPUT);

    ledcSetup(chRightForward ,frequency,resolution);
    ledcSetup(chRightBackward,frequency,resolution);
    ledcSetup(chLeftForward  ,frequency,resolution);
    ledcSetup(chLeftBackward ,frequency,resolution);

    ledcAttachPin(PWM_A,chRightForward);
    ledcAttachPin(PWM_B,chRightBackward);
    ledcAttachPin(PWM_a,chLeftForward);
    ledcAttachPin(PWM_b,chLeftBackward);
    driverOFF();
    Serial.println("================================");
    Serial.println(" PS3 Robot Ready");
    Serial.println(" Differential Drive");
    Serial.println("================================");
}

void loop()
{
    if(!Ps3.isConnected())
    {
        driverOFF();
        return;
    }

    if(Ps3.data.button.triangle){
        speed_mode = 255;
        turn_mode  = 120;
    }
    else if(Ps3.data.button.circle){
        speed_mode = 200;
        turn_mode  = 100;
    }
    else if(Ps3.data.button.cross){
        speed_mode = 145;
        turn_mode  = 85;
    }
    else if(Ps3.data.button.square){
        speed_mode = 105;
        turn_mode  = 70;
    }

    joyY = Ps3.data.analog.stick.ly;      
    joyX = Ps3.data.analog.stick.rx;      

    if(abs(joyY) < deadZone) joyY = 0;
    if(abs(joyX) < deadZone) joyX = 0;
    if(joyX == 0 && joyY == 0)
    {
        targetLeft = 0;
        targetRight = 0;

        currentLeft = 0;
        currentRight = 0;

        driverOFF();
        return;
    }
    else
    {
        driverON();
    }

    int maju = map(joyY,
                   -128,127,
                   -speed_mode,
                   speed_mode);

    int belok = map(joyX,
                    -128,127,
                    turn_mode,
                    -turn_mode);

    
    targetLeft  = belok + maju;
    targetRight = belok - maju;
    targetLeft  = constrain(targetLeft,
                            -255,
                             255);

    targetRight = constrain(targetRight,
                            -255,
                             255);

    
    rampMotor();

    motor(currentLeft,currentRight);


    Serial.print("Y:");
    Serial.print(joyY);

    Serial.print(" X:");
    Serial.print(joyX);

    Serial.print(" L:");
    Serial.print(currentLeft);

    Serial.print(" R:");
    Serial.print(currentRight);

    Serial.print(" Mode:");
    Serial.println(speed_mode);
}