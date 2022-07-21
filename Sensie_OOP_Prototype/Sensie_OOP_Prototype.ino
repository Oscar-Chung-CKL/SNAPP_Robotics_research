//Libraries
#include "Fins.h"
#include "Sensie.h"
#include "Motor.h"

#define debug 0

Fins fins;
Motor motor;
Sensie sensie;


// Basic control values
int speedVal = 0;
int pitchVal = -1;
int rollVal = -1;
int yawVal = -1;

int inComingbyte[4] = {0};
float motor_Pwm = 0;        // Motor PWM

// Turn Differential
int turnVal = 5;            // Stores commmand value for control left and right. values 1-4 are left, 5 is straight, 6-9 are right


// Kill Switch
long int killTimer;       // timer for kill switch
// Possible scenario
// Fish runs for so long, that the timer resets to 0. Then the difference between the previous and the current will be >0, and kill the fish
// Might cause serious problems in the future when we run the fish across the harbor for instance


//For the yaw turning 
float q1 = 90; //0.25 of count
float q2 = 145; //0.4
float q3 = 270;//0.75
float q4 = 325; // 0.9

/////////////////////////////////////////// End of Yaw Turn variables //////////////////////////////////////////////////

void getData();


void setup() 
{
  //Initialize //Serial
  Serial.begin(115200,SERIAL_8O1);
  Serial2.begin(19200,SERIAL_8O1);

  #if debug
  Serial.println("Arming........");   // just some display message 
  #endif

  fins.initFin();
  motor.initMotor();

  //Initialize timer
  killTimer = millis();

  #if debug
  Serial.println("Setup is complete!");
  #endif
}


void loop() 
{
  getData();
  // CAUTION: Fish will turn on after kill switch activated once if signal is restored and checksum is passed
  killswitch();

  enc_pos = motor.encoderPosition();
  turnVal = yawVal;

  //Send PWM signal to motor
  motor_Pwm = (speedVal*255)/9;
  //Servo control expression. Linear combination of X and Y component of JoyStick // This part need to ask Tim
     
  s1 = (((43-rollVal)*maxAttacAngle/50 +(42-pitchVal)*maxAttacAngle/50) + 90.0);
  s2 = (((43-rollVal)*maxAttacAngle/50 -(42-pitchVal)*maxAttacAngle/50) + 90.0);

  //Serial.println((String) s1 + " " + s2 + " " + val1 + " " + val2);     
  //Servo motor angle is set

  servo1.write(s1);
  servo2.write(s2);

  fins.finControl(rollVal, pitchVal);

  int throttle = motor.yaw_turn( motor_Pwm, turnVal, speedVal);
  motor.driveMotor(throttle);
   
  Serial.flush();
  Serial2.flush();
}


void killswitch()
{
  if (millis() - killTimer > 3000) 
  {
    speedVal=0;
    motor_Pwm = 0;
    pusherESC.write(1500);
  }
}


// data update from controller
void getData()
{                  
// updates the speed, pitch, roll and yaw command 
// cmd signal would be : c speed(1) pitch(1) roll(1) yaw (1) e 
// the buffer would be of length 6.
  if(Serial2.available()>=6)
  {
    int x = Serial2.read();

    if (x == 'h')   // Home command
    {
      motor.resetMotor();   // Reset the encoder to 0
    }
    
    else if(x == 'c')    // Elif, just in case some wild signal pops in
    {
      for (int i = 0; i < 4; i++)
      {
        inComingbyte[i] = Serial2.read();
      }
        
      //Cycle through the entire buffer and rocesses the information  
      speedVal  = inComingbyte[0] -'0';
      pitchVal  = inComingbyte[1] ;
      yawVal    = inComingbyte[2] - '0';
      rollVal   = inComingbyte[3] ;
         
      killTimer = millis();  
    }
  }

  int temp = sensie.temperature();
  int waterLevel = sensie.waterLevel();
}
