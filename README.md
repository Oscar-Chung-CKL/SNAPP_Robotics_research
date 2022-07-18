# SNAPP The Robotic Fish

## Overview
This is a development code that drives SNAPP, our Robotic Fish!

## <br>Code Breakdown
As the title says, this section will break down every part of the code down the smallest detail.

### <br>**The Variables**
---

The variables are as follows:

- The debug variable is a flag that allows for verbose logging of the stuff going on in the fish for easier debugging through serial outputs.

```Arduino
#define debug 0
```

- ```Servo```s ```servo1``` and ```servo2``` create the structs as required by the ```<Servo.h>``` library, which represent the servos controlling the fins. These are mapped to pins 9 and 11, stored in the ```servoPin1``` and ```servoPin2``` variable. Subsequently, the ```initial, min_angle, max_angle, mid_angle, maxAttacAngle``` for both the fins are there to help with common angles the servo might be in.
<br> 
```c
Servo servo1;
Servo servo2;
int servoPin1 = 9;
int servoPin2 = 11;
int min_angle = 0;
int max_angle = 180;
int mid_angle = 90;
int min_range = 0;
int max_range = 9;
int mid_range = 4;
float maxAttacAngle=40;
```
- The ```speedVal, pitchVal, rollVal and yawVal``` correspond to the speed, pitch, roll and yaw values respectively.
<br> 
```c
int speedVal = 0;
int pitchVal = -1;
int rollVal = -1;
int yawVal = -1;
```

- The ```float```s ```s1``` and ```s2``` store the angle of the servos 1 and 2 as discussed previously, and the inComingbyte variable stores the ```speed, pitch, yaw``` and ```roll``` values in that order into an array, while the ```power``` and ```motor_Pwm``` set the power and motor's PWM form the controller.

```c
float s1, s2; // s1: degree of servo 1      s2: degree of servo 2
int inComingbyte[4] = {0};
int power = 0;          //Stores power value from controller
//Motor PWM
float motor_Pwm = 0;
```

- The turnVal and killTimer stores the turn value for the fish and the last time the fish received a signal, to determine if the fish has been swimming for more than 3 seconds without a signal.

```c
int turnVal = 5;
long int killTimer;
```
- Then we declare some basic variable, like ```PIN_PUSHERESC, THROTTLE_MIN, THROTTLE_MAX, THROTTLE_BAKE, ENCA, ENCB, pos_Main, ticRatioMainMotor, encoder_resolution, gearbox``` and ```cpt```, which represent the pin the main motor signal pin is connected to, the minimum throttle, maximum throttle, brake throttle, encoder A pin, encoder B pin, main position for the encoder, the tick ratio of the motor (explained later in the ```encoderPosition()``` function), the encoder resolution, ~~witch hunt Tim and do it again~~.

```c
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
const long int ticRatioMainMotor=6255; //Number of tic per revolution of the main motor. 
                                       //Implemnted to use the relative encoder as an absolute encoder temporarily. 


//////////////////////////////////////////// Yaw turn variables /////////////////////////////////////////////////////////

const int encoder_resolution = 4;
const int gearbox = 12;
const int cpt = 500;
```

Next, we create the structs for the Encoders A and B.

```c
Encoder fish_enc(ENCA, ENCB);
```

And finally, ```counts_per_revolution, q1 ... q4``` and ```enc_pos``` determine the counts per revolution of the encoder, common angles for the rotations and for storing the encoder position.

```c
long count_per_revolution; // this variable keeps tarck of how much is one revolution

//For the yaw turning 
float q1 = 90; //0.25 of count
float q2 = 145; //0.4
float q3 = 270;//0.75
float q4 = 325; // 0.9
float enc_pos = 0;
```
And that wraps up the variables we need!

### <br> **The getData() Function**
---
Processes the data received from the RC transceiver. In the case of this fish, we use the Arduino's ***Serial 1*** TX/RX pair, and receive the data as a sequence of ***six characters***, stored in the variable called ***char[6]***. The ***first*** of these characters, ***char[0]*** is either an "h", which refers to "home", or in layman terms, ___reset or "home" the encoder to the zero position___, OR, ***char[0]*** can represent "c", which indicates a normal transmision. The ***next four*** of these characters is then retrieved via a for loop, which is stored in the ***incomingByte[6]*** global variable. The first of these numbers represents the speed value, stored in ***incomingByte[0]***, which is then reassigned to ***speedVal***, the second represents the pitch value, reassiged to **pitchVal**, stored in ***incomingByte[1]***, and so on and so forth for the next yaw value and the roll value for convenience. In addition, a kill timer is also started to later detect if the signal is not being received for more than 3 seconds.


<br>The code is implemented as follows:

First, we check if we are even receiving a signal, and if we are, read the first character.

```c
getData{
  if(Serial1.available()>=6){
    int x = Serial1.read();
        .
        .
        .
    // more code below
```


<br>Next, we check if it's a home command by interpreting the character we just read. If it's "h", then set the encoder position to zero, effectively resetting the system.

```c
... // previous code

    if (x == 'h')   // Home command
    {
        fish_enc.write(0);   // Reset the encoder to 0
    }

... // continued
```

<br>If it is not a home command however, we check if it's a "c" or "normal" command, whose following sequence consists of the speed, pitch, yaw and roll values as discussed previously. The code will also set off the kill timer, as shown.

```c
    ... // previous code

    elif(x == 'c')    // Elif, just in case some wild signal pops in
    {
      for (int i = 0; i < 6; i++)
      {
        inComingbyte[i] = Serial1.read();
      }
        
      //Cycle through the entire buffer and rocesses the information  
      speedVal  = inComingbyte[0] -'0';
      pitchVal    = inComingbyte[1] ;
      yawVal      = inComingbyte[2] - '0';
      rollVal     = inComingbyte[3] ;
         
      killTimer = millis();  
    }
  }
}  // end of getData()
```

### <br><br> **The encoder_position() Function**
---
As the title suggests, this function returns a floating point value corresponding to the encoder position from 0-360 degrees, **very precisely**.

Before reading this section, please note that the encoder values we read here are measured in "ticks", where a "tick" is the single smallest rotation that we can detect. For example, a clock has a total of 60 seconds, or "ticks", where a second represents a single "tick". In the case of the encoder, the number of ticks is determined by the manufacturer, but usually, it's in the order of 6000+ ticks! (if that number doesn't impress you, try doing some math and figure out how many degrees represents a single tick on these encoders vs a clock hand)

First, we read the encoder value into a long variable **new_fin**, and then gets the remainder of the division of **new_fin** by the **count_per_revolution** global variable.

```c
  long new_fin = fish_enc.read();
  long x = abs(new_fin%count_per_revolution);
```

Hold up, why are we using a long variable instead of an int?
<br><br> **Answer:** The encoder we are using has 500 counts per turn, with 4 readings per count, and even the smallest gearbox we use has a ratio of 15:1. This means we can have at least 500 x 4 x 15 = 30,000, which is approaching very close to the limits of an int variable, which can only store up to 32,768. So in order to use higher gear ratio gearboxes, we definitely need a larger variable to store this data.

<br>Next, we check the value of x. If it is zero, that means it is at the start of the clock cycle (i.e., at the 12 o'clock position). The value is thus returned appropriately, by simple ratio.

```c
  if (x == 0)
  {
    return new_fin*1.0/count_per_revolution*360;
  }
```
<br> Otherwise, if that condition is not met, we can just return the default angle by short circuit evaluation.

```c
return x*1.0/count_per_revolution*360;
```

### <br> **The yaw_turn() Function**
---
This function drives the "yaw" of the fish by taking into account the encoder position and setting the appropriate PWM signal on the motor. To visualize this, to make the fish "turn", let's say to the left, we need to push the tail to the left very quickly, then slowly turn to the right, and repeat this many times until the desired turn is achieved.

The first thing we do is to convert the ```turn``` value, which has the values from 1 to 9 to -4 to 4. This is to visualize the turn values easier, as we can think of negative values being to the left, and positive ones to the right.

```c
turn -= 5;
```
Next, we check three condtions for the turn value:

- If the turn is 0:
<br> Just return the pre-existing pwm, as it is at the center of the cycle, and no changes to the pwm are necessary.

```c
if(turn == 0)
{
  return(pwm);
}
```
- If the encoder position is betwen 89 and 271:
<br> Timothy onii chan pls explain kudasai~ But basically it determines which part of the cycle the motor is in based on the encoder position, then it does some math magic and sets the appropriate pwm. Go witch hunting Timothy for details. tyvm
```c
  if( (enc_pos > 89.0) && (enc_pos < 271.0))
  {
    float x = (1 + turn*diff)*pwm;
    if (x > 255) x = 255;
    if (x < 30 and speedVal != 0) x = 30;
    if (speedVal == 0) x = 0;

    Serial.print(" First half PWM: ");
    Serial.println(x);
    return round(x);
  }
  if ( ( enc_pos > 270.0 && enc_pos < 361.0) || (enc_pos >= 0.0 && enc_pos < 90.0))
  {
    float x = (1 - turn*diff)*pwm; 
    if (x > 255) x = 255;
    if (x < 30 and speedVal != 0) x = 30;
    if (speedVal == 0) x = 0;
    Serial.print("Second Half PWM: ");
    Serial.println(x);
    return round(x);
  }
```

### **The setup() Function**
---
You have conquered the mathematical bullshittery going on here. Impressive! Let's move on to the real deal and set this shit up.

Firstly, we set the serial channel 0 and serial channel 1 to 115200 and 19200 baud rate respectively, or in layman terms, set the RX0/TX0 pair on the Arduino spit out and receive data at 115200 bits per second and the RX1/TX1 to 19200 bits per second.

```c
Serial.begin(115200,SERIAL_8O1);
  Serial1.begin(19200,SERIAL_8O1);
```

Then we set the servo pins as output, and attach them as required by the servo library. These will ultimately control the fins on the side of the fish.

```c
 pinMode(servoPin1, OUTPUT);
  pinMode(servoPin2, OUTPUT);
  servo1.attach(servoPin1); // Attach servo pins
  servo2.attach(servoPin2);
  servo1.write(initial1);
  servo2.write(initial2);
```

Next, we start a kill timer, to determine how long the fish is moving without the signal. This is important so the fish doesn't become sentient and keeps moving when it losees the signal.

```c
killTimer = millis();
```

Next, we "attach" the main tail section, which is a geared, brushed DC motor to the system through an ESC. While the hardware is different, it handles information just like the servos, and so we can just use the servo library to drive the tail.

```c
pusherESC.attach(PIN_PUSHERESC);
pusherESC.writeMicroseconds(THROTTLE_MIN);
```

Next, we set the servos to the horizontal position relative to the water, and set the tail ESC to a value of 1500, aka not moving at all.

```c
servo1.write(90); // Set Servo to defaults
servo2.write(90);
```

### **The loop() function**
---
The easiest yet the most important part of the whole system. Can't drive no fish without loopin'.

First, we get the data from the controller, and set the encoder position. We also start off the killswitch() function to detect if the fish has been swimming more than 3 seconds without receiving a signal (and cut off the signal if that is the case).
```c
getData();
killswitch()
enc_pos = encoderPosition();
```
First, we ~~go witch hunting Tim again for details~~ do some math to determine the angle the fins need to be in into variables s1 and s2, which reprsent the angle of s1 and s2 respectively.
```c
 s1= (((43-rollVal)*maxAttacAngle/50 +(42-pitchVal)*maxAttacAngle/50) + 90.0)  ;
s2=  (((43-rollVal)*maxAttacAngle/50 -(42-pitchVal)*maxAttacAngle/50) + 90.0) ;

```

Next, we write the servo 1 and servo 2 angles based on the data received from the controller (roll, turn, pitch, etc.). In layman terms, it changes the angle of the fins.
```c
servo1.write(s1);
servo2.write(s2);
```

Next, we get the throttle speed by receiving the data from the ```yaw_turn()``` function, map it to the MIN and MAX throttle values, and then pass the information to the ESC.
```c
float  turn_differential = 0.2;
int throttle = yaw_turn( motor_Pwm, turn_differential, turnVal);
throttle = map(throttle, 0, 255, THROTTLE_MIN, THROTTLE_MAX);
pusherESC.writeMicroseconds(throttle);
```
Finally, we ```flush()``` all the serial pins to avoid any sort of delay.

### <br> **The killswitch() Function:**
---
Basically what the title says, it acts as a kill switch that will "kill" the fish's movement if it loses the signal. It checks if the difference between the current time and the previous kill timer is greater than 3000ms, and sets the power, motor_pwm, servo positions and ESC to their stopped position. In layman terms, this resets all the fish's components to the normal position and stops its motion, if it does not receive a signal from the controller for more than 3 seconds.
```c
void killswitch()
{
  if (millis() - killTimer > 3000) 
  {
    power=0;
    motor_Pwm = 0;
    pusherESC.write(1500);
    servo1.write(90);
    servo2.write(90);
  }
}
```

And that wraps up the entirety of SNAPP's guts!
