//Libraries
#include <Servo.h>
#include <Encoder.h>

#define debug 0

// Servo
Servo servo1;
Servo servo2;
int servoPin1 = 9;
int servoPin2 = 11;
int initial1 = 90;
int initial2 = 90;
int min_angle = 0;
int max_angle = 180;
int mid_angle = 90;
int min_range = 0;
int max_range = 9;
int mid_range = 4;
float maxAttacAngle=40;

// Basic control values
int speedVal = 0;
int pitchVal = -1;
int rollVal = -1;
int yawVal = -1;


float s1, s2;               // s1: degree of servo 1      s2: degree of servo 2
int inComingbyte[4] = {0};
int power = 0;              // Stores power value from controller
float motor_Pwm = 0;        // Motor PWM


// Turn Differential
int turnVal = 5;            // Stores commmand value for control left and right. values 1-4 are left, 5 is straight, 6-9 are right


// Kill Switch
long int killTimer;       // timer for kill switch
// Possible scenario
// Fish runs for so long, that the timer resets to 0. Then the difference between the previous and the current will be >0, and kill the fish
// Might cause serious problems in the future when we run the fish across the harbor for instance



// Motor control through ESC Driver
#define PIN_PUSHERESC       5     // PIN to control ESC, normally the white wire from ESC 
Servo   pusherESC; 
/*///////////////////////////////////////////////////////////////////////////////////////////////////////
 * The following are the setting for the ESC used, for other ESC, just chnage setting according to
 * ESC Spec
 *///////////////////////////////////////////////////////////////////////////////////////////////////////
#define THROTTLE_MIN        1500
#define THROTTLE_MAX        2000
#define THROTTLE_BAKE       1000
// End of ESC setting ///////////////////////////////////////////////////////////////////////////////////


//Relative Encoder Main Motor

#define ENCA 21   // YELLOW
#define ENCB 4    // WHITE
volatile long int pos_Main = 0;
const long int ticRatioMainMotor=6255; //Number of tic per revolution of the main motor. 
                                       //Implemnted to use the relative encoder as an absolute encoder temporarily. 


//////////////////////////////////////////// Yaw turn variables /////////////////////////////////////////////////////////

const int encoder_resolution = 4;
const int gearbox = 12;
const int cpt = 500;

Encoder fish_enc(ENCA, ENCB);

long count_per_revolution; // this variable keeps tarck of how much is one revolution

//For the yaw turning 
float q1 = 90; //0.25 of count
float q2 = 145; //0.4
float q3 = 270;//0.75
float q4 = 325; // 0.9
float enc_pos = 0;

/////////////////////////////////////////// End of Yaw Turn variables //////////////////////////////////////////////////


// data update from controller
void getData()
{                  
// updates the speed, pitch, roll and yaw command 
// cmd signal would be : c speed(1) pitch(1) roll(1) yaw (1) e 
// the buffer would be of length 6.
  if(Serial1.available()>=6)
  {
    int x = Serial1.read();

    if (x == 'h')   // Home command
    {
      fish_enc.write(0);   // Reset the encoder to 0
    }
    
    else if(x == 'c')    // Elif, just in case some wild signal pops in
    {
      for (int i = 0; i < 4; i++)
      {
        inComingbyte[i] = Serial1.read();
      }
        
      //Cycle through the entire buffer and rocesses the information  
      speedVal  = inComingbyte[0] -'0';
      pitchVal  = inComingbyte[1] ;
      yawVal    = inComingbyte[2] - '0';
      rollVal   = inComingbyte[3] ;
         
      killTimer = millis();  
    }
  }
}

//////////////////////////////////////////////////////////// Yaw turn functions ///////////////////////////////////////////////////

float encoderPosition()
{
  // takes the current count and updates it to an angle from 0 degrees - 360 degress
  long new_fin = fish_enc.read();
  long x = abs(new_fin%count_per_revolution);
  
  if (x == 0)
  {
    return new_fin*1.0/count_per_revolution*360;
  }
  return x*1.0/count_per_revolution*360;

}

int yaw_turn(int pwm, float diff, int turn)
{
  //Map the turn value to +/- 4, which is easier to understand
  turn -= 5;
  //Accepts a pwm signal and outputs a pwm signal from 0-255
  Serial.print(" This is the encoder position ");
  Serial.println(enc_pos);

  if(turn == 0)
  {
    #if debug
    Serial.print("Normal PWM: ");
    Serial.println(pwm);
    Serial.flush();
    #endif

    return (pwm);
  }
  
  if( (enc_pos > 89.0) && (enc_pos < 271.0))
  {
    float x = (1 + turn*diff)*pwm;
    if (x > 255) x = 255;
    if (x < 30 and speedVal != 0) x = 30;
    if (speedVal == 0) x = 0;
    
    #if debug
    Serial.print(" First half PWM: ");
    Serial.println(x);
    #endif

    return round(x);
  }
  if ( ( enc_pos > 270.0 && enc_pos < 361.0) || (enc_pos >= 0.0 && enc_pos < 90.0))
  {
    float x = (1 - turn*diff)*pwm; 
    if (x > 255) x = 255;
    if (x < 30 and speedVal != 0) x = 30;
    if (speedVal == 0) x = 0;
    
    #if debug
    Serial.print("Second Half PWM: ");
    Serial.println(x);
    #endif

    return round(x);
  }
  
}

//////////////////////////////////////////////////////////End of Yaw turn functions ///////////////////////////////////////////////


void setup() 
{
  //Initialize //Serial
  Serial.begin(115200,SERIAL_8O1);
  Serial1.begin(19200,SERIAL_8O1);

  #if debug
  Serial.println("Arming........");   // just some display message 
  #endif
  //while (!(Serial.available() || Serial1.available()));     
  //Ensure fish does not start until there is a signal from comp or controller
  
  //Initialize Servo
  pinMode(servoPin1, OUTPUT);
  pinMode(servoPin2, OUTPUT);
  servo1.attach(servoPin1); // Attach servo pins
  servo2.attach(servoPin2);
  servo1.write(initial1);
  servo2.write(initial2);
  
  //Initialize timer
  killTimer = millis();

  // Attach ESC
  pusherESC.attach(PIN_PUSHERESC);
  pusherESC.writeMicroseconds(THROTTLE_MIN); 

  servo1.write(90); // Set Servo to defaults
  servo2.write(90);

  //////////////////// Yaw turn set up////////////////////////////////

  count_per_revolution = cpt*gearbox*encoder_resolution;

  // delay(100);  // set this on in case the arduino fks up
  pusherESC.writeMicroseconds(1500);

  ///////////////////Yaw turn set up END /////////////////////////////
  #if debug
  Serial.println("Setup is complete!");
  #endif
}


void loop() 
{
  getData();
  // CAUTION: Fish will turn on after kill switch activated once if signal is restored and checksum is passed
  killswitch();
  //rel_Encoder();
  enc_pos = encoderPosition();

  //Serial.println(enc_pos);
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

  //////////////////////////////////////// Yaw turn action//////////////////////////////////
  float  turn_differential = 0.2;
  int throttle = yaw_turn( motor_Pwm, turn_differential, turnVal);
  throttle = map(throttle, 0, 255, THROTTLE_MIN, THROTTLE_MAX);
  pusherESC.writeMicroseconds(throttle);
  ////////////////////////////////////////  Yaw turn action END///////////////////////////////////
  //Serial.println((String)"power: " + power + "turning: " + turnVal);
  
  Serial.flush();
  Serial1.flush();
}


void killswitch()
{
  if (millis() - killTimer > 3000) 
  {
    power=0;
    speedVal=0;
    motor_Pwm = 0;
    pusherESC.write(1500);
  }
}