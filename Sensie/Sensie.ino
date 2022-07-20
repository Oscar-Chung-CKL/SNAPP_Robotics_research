//Libraries
#include <Servo.h>
#include <Encoder.h>
#include <Sensie.h>

/*
 * Fixing up Saad's code. There are many things I would like to change, as I like my previous code more, but that will just be a lot of work. I need it to get it functional. 
 * will figure out how to make it tidy by using my own header files and function. 
 * 
 * Core things to have is a reliable communication system, will need to remove all the redundant signals. 
 * 
 * Just finished writing the encoder code that would allow us to measure position and speed. (relative to the starting position of course) 
 * We need to mechanically home it before starting. Add that at setup. 
 * 
 * Also change the comm signals by adding a 20 character requirement for the buffer before reading it. Removed the flush. 
 * Remember to add timer to the controller side. lets have it send signals every 50ms. 
 * 
 * 
 */

#define debug 0
//Servo
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

int speedVal = 0;
int pitchVal = -1;
int rollVal = -1;
int yawVal = -1;

//Communication

float s1, s2;
bool check;
int inComingbyte[6] = {0};
String w;
String char1, char2, char3, char4;
int val1 = 0, val2 = 0, val3 = 0, val4 = 0;
int power = 0;          //Stores power value from controller

/////////////////// sensor variable //////////////////
#define ledLeft 14
#define ledRight 15

int waterLevel = -1;
#define waterLevelPin  A5

int heatLevel = -1;
#define heatLevelPin 2 
/////////////////// sensor variable END //////////////////


//Motor

float motor_Pwm = 0;
bool red_Flag = LOW;
bool on = LOW;

//encoder
unsigned long int timer1 = 0;
unsigned long int timer2 = 0;
long int counter = 0;
long int runtime = 0;
long int timer = 0;
float timeStep = 0;
long int encoderCounter = 0;
long int encoderRawVal = 0; //Global Variable for encoder absolute value
double angularVelocity[3] = {0, 0, 0};
long int counter2 = 0;
volatile long unsigned int speed_Counter = 0;
bool flag_Motion = false; // this flag checks if the motor is in motion, returns false if it is stalled. 


float frequency = 0;
long int t_initial = 0;

//Turn Differential
int turnVal = 5;        //Stores commmand value for control left and right. values 1-4 are left, 5 is straight, 6-9 are right
int oldTurnVal = 5;
int encoderVal = 0;         //stores value of encoder at any given time. Updates from interrupt.
float turn_Pwm = 0;         // Pwm value with differential applied
int oldPower = 5;
long int tailDelay1 = 2000;
int encoderPin0   =  29;
double diff = 0.5;              //to store the differential value corresponding to the signal
double stepDiff = 0.1;          //sets the differential value
int highcutoff, lowcutoff, offset;

//Kill Switch
long int killTimer; //timer for kill switch

//Motor control through ESC Driver
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

#define ENCA 21 // YELLOW
#define ENCB 4 // WHITE
volatile long int pos_Main = 0;
const long int ticRatioMainMotor=6255; //Number of tic per revolution of the main motor. Implemnted to use the relative encoder as an absolute encoder temporarily. 
long int rotationCounter = 0;
long int prev_Ticks = 0;
long int delta_t = 0;

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
int userCommand = 0;

/////////////////////////////////////////// End of Yaw Turn variables //////////////////////////////////////////////////

float rel_angle = 0;

void getData(){                  //data update from controller
//updates the speed, pitch, roll and yaw command 
//cmd signal would be : c speed(1) pitch(1) roll(1) yaw (1) e 
// the buffer would be of length 6.
    //Serial.println("getting");
    if(Serial1.available()>=6)
    {
      int x = Serial1.read();
      if (x == 'h')
      {
        ///////////////////////////////////// home set here//////////////////////////////////////
        Serial.println("home got");
        // write 0
        fish_enc.write(0);
        ///////////////////////////////////////home set END//////////////////////////////////////
      }
      

      elif(x == 'c')
      {
        for (int i = 0; i < 6; i++)
        {
          inComingbyte[i] = Serial1.read();
        }
        speedVal  = inComingbyte[0] -'0';
        pitchVal    = inComingbyte[1] ;
        yawVal      = inComingbyte[2] - '0';
        rollVal     = inComingbyte[3] ;
        killTimer = millis();     
      }
    }
}

//////////////////////////////////////////////////////////// Yaw turn functions ///////////////////////////////////////////////////

float encoder_position()
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


int pid_controller(float angle, float desired_angle, int k_p){
  //takes the angle and calculates the corrected signal. Returns the value in microseconds. 
  float error = desired_angle-angle;
  float x = (k_p*error)/3+1500;
  if (x > 1800) x = 1800;
  if (x < 1200) x = 1200;
}

long myAbs(long x){
  if (x > 0) return x;
  else if (x < 0) return -x;
  return 0;
}

int yaw_turn(int pwm, float diff, int turn){
  //turn is a value from 1-9, Convert turn from 0-9 to -4 to 4.
  turn = map(turn, 1, 9, -4 , 4);
  
  //Accepts a pwm signal and outputs a pwm signal from 0-255
  Serial.print(" This is the encoder position ");
  Serial.println(enc_pos);
  
  
  if( (enc_pos > q1) && (enc_pos < q2)){
    float x = (1 + turn*diff)*pwm;
    if (x > 255) x = 255;
    if (x < 0) x = 0;

    Serial.print(" First half PWM: ");
    Serial.println(x);
    return round(x);
  }
  if ( ( enc_pos > q3) && (enc_pos < q4)){
    float x = (1 - turn*diff)*pwm; 
    if (x > 255) x = 255;
    if (x < 0) x = 0;
    Serial.print("Second Half PWM: ");
    Serial.println(x);
    return round(x);
  }
   Serial.print("Normal PWM: ");
    Serial.println(pwm);
    Serial.flush();
    return (pwm);
}

//////////////////////////////////////////////////////////End of Yaw turn functions ///////////////////////////////////////////////


void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); // Off to indicate still in setup

  // When the fish is connected without shield, there is one more 5V pin require
  // The code below is to use digital pin to output %v as temp use
  // Plz remove this line if a shield is used

  digitalWrite(33, HIGH); // <- this is the line to be removed

  pinMode(ledLeft, OUTPUT);
  pinMode(ledRight, OUTPUT);
  digitalWrite(ledLeft, LOW);
  digitalWrite(ledRight, LOW);
    
//Interrupt pin pullup relative encoder
 // pinMode(ENCA,INPUT_PULLUP);
//  pinMode(ENCB,INPUT_PULLUP);
  
  //Initialize //Serial
  Serial.begin(115200,SERIAL_8O1);
  Serial1.begin(19200,SERIAL_8O1);
  while (!(Serial.available() || Serial1.available()));     //Ensure fish does not start until there is a signal from comp or controller
  


  //Initialize Servo
  pinMode(servoPin1, OUTPUT);
  pinMode(servoPin2, OUTPUT);
  servo1.attach(servoPin1); // Attach servo pins
  servo2.attach(servoPin2);
  servo1.write(initial1);
  servo2.write(initial2);
  

  
  //Initialize timer
  killTimer = millis();
  
  Serial.println("Encoder Pins Initialized!");
  Serial.println("Setup is complete! Click to begin the program....");
  


  Serial.println("Arming........");   // just some display message 
    pusherESC.attach(PIN_PUSHERESC);
    pusherESC.writeMicroseconds(THROTTLE_MIN);

    t_initial = 0; //initiallize speed encoder;

  Serial.println("Arming........After delay");  
  
  //Setup complete prompt
  digitalWrite(13, HIGH); //To Check if setup complete
  Serial.println("Setup is complete!");

  servo1.write(90); // Set Servo to defaults
  servo2.write(90);

  //////////////////// Yaw turn set up////////////////////////////////

  Serial.begin(115200);
  count_per_revolution = cpt*gearbox*encoder_resolution;

  delay(100);
  pusherESC.writeMicroseconds(1500);

  ///////////////////Yaw turn set up END /////////////////////////////

}

char cmd[12];    //to store the signal from transmitter

void loop() {

  getData();
  enc_pos = encoder_position();
  //Serial.println(enc_pos);

//left / right LED lights when turning, both lights when going straight line
  if (turnVal >= 1 && turnVal <= 4){
    digitalWrite(ledLeft, HIGH);
    digitalWrite(ledRight, LOW);
  }
  else if(turnVal >= 6 && turnVal <= 9){
    digitalWrite(ledLeft, LOW);
    digitalWrite(ledRight, HIGH);    
  }
  else{
    digitalWrite(ledLeft, HIGH);
    digitalWrite(ledRight, HIGH);
  }
  
  turnVal = yawVal;

      //Send PWM signal to motor
      motor_Pwm = (speedVal*255)/9;
     
      //Servo motor angle is set
      servo1.write(s1);
      servo2.write(s2);
      
     //Servo control expression. Linear combination of X and Y component of JoyStick // This part need to ask Tim
     
       s1= (((43-rollVal)*maxAttacAngle/50 +(42-pitchVal)*maxAttacAngle/50) + 90.0)  ;
       s2=  ( ((43-rollVal)*maxAttacAngle/50 -(42-pitchVal)*maxAttacAngle/50) + 90.0) ;

      //Serial.println((String) s1 + " " + s2 + " " + val1 + " " + val2);
      
    // CAUTION: Fish will turn on after kill switch activated once if signal is restored and checksum is passed
    killswitch();
    
    //rel_Encoder();
    

  //////////////////////////////////////// Yaw turn action//////////////////////////////////
  float  turn_differential = 0.2;
  int throttle = yaw_turn( motor_Pwm, turn_differential, turnVal);
  throttle = map(throttle, 0, 255, THROTTLE_MIN, THROTTLE_MAX);
  pusherESC.writeMicroseconds(throttle);
  ////////////////////////////////////////  Yaw turn action END///////////////////////////////////

      if (power==0){
        digitalWrite(13, LOW);
      }
      else{
        digitalWrite(13, HIGH);
      }


  #if debug
  //For debugging purposes, we can print everything on the Serial to test, just change debug to 1. 

    Serial.print("Encoder Items: ");
    Serial.print("\t");
    Serial.print(rotationCounter);
    Serial.print("\t");
    Serial.print(frequency); 
    Serial.print("\t");
    Serial.print(rel_angle);
    Serial.println();

    // Communication Items

    // Actuation Signals 
  #endif

  //Serial.println((String)"power: " + power + "turning: " + turnVal);
  
  Serial.flush();
  Serial1.flush();
}

void rel_Encoder(){

  long int x = pos_Main; //variable for current ticks
  if(prev_Ticks <= x );
  
  else{
    
    if(x < ticRatioMainMotor){
      rel_angle = (x/(ticRatioMainMotor*1.0))*360; //this is angle in degrees
    }
    else if ( x >= ticRatioMainMotor){
      // this command happense whenever we pass a turn
      
      pos_Main = x - ticRatioMainMotor; //reset the number of ticks
      prev_Ticks = x - ticRatioMainMotor; // the comparison for motion depends on the tick counter, which we reset after a full loop. Otherwise we can have tick count as two parts, like hour and minutes thing. 
      rotationCounter++; //keep track of the number of rotation
    }
    
    // Speed determination 
    if(speed_Counter > 10){
      int temp = millis(); //placeholder for the current time, so that all calculations reference this time
      delta_t = temp - t_initial;
    if (delta_t > 0) frequency = speed_Counter/(delta_t);
    t_initial = temp;
    prev_Ticks = x;
    speed_Counter = 0; 
    }   
  }
}


void killswitch(){
   if (millis() - killTimer > 3000) {
    power=0;
    motor_Pwm = 0;
    pusherESC.write(1500);
    digitalWrite(ledLeft, LOW);
    digitalWrite(ledRight, LOW);
  }
}

//function for check sum
bool checkSum(char incomingByte, int siglen, char cmd[]) {
  check = false;
  int cSum=0;
  for (int c=0; c<8; c++){
    cSum+=cmd[c];
  }
  String numcheck = String(cmd[8]) + String(cmd[9]) + String(cmd[10]);
  int number = numcheck.toInt();
  if (cSum == number) {
    check = true;
  }
  return check;
}

/////////////////////////////////////////////// send back data functions //////////////////////////////////////////////////////
int waterAmount(){
  waterLevel = analogRead(waterLevelPin);
  waterLevel = map(waterLevel, 0, 1024, 0, 255);
  return waterLevel;
}

int tempature(){
//  heatLevel = reading;
//  return heatLevel;
}

void response(int water, int heat){
  Serial.write((byte)water);
  Serial.write((byte)heat);
  Serial.flush();
}
////////////////////////////////////////////////// end of function //////////////////////////////////////////////////////
