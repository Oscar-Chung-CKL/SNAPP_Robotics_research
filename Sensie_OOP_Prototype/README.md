# SNAPP The Robotic Fish - OOP Prototype

## Overview
This is the newer, more organized version of the original code, sectioned off into header files. It does the same thing (probably with the same if not faster runtime), with the primary aim of making the code a LOT more digestable and easy to read, and in turn, easier to maintain.

The code is divided into five parts:
- The main .c file which serves as the driver code
- The _**Motor.h**_ which contains the code and math to drive the motor.
- The ___Fins.h___, which contains the code and math to drive the servos within the fins.
- The ___Power.h___, which contains the code to handle the power equipment.
- The ___Sensie.h___, which contains code to handle the smart sensing equipment.

First, we shall discuss the header files and their uses.

## **Motor.h**
This file handles the motor controls.

### <br> **The Variables:**
___
We declare some basic variables, as follows, to define the pins for the different components and for flow control in each of the functions, as well as the necessary libraries.

```c
#include <Encoder.h>
#include <Servo.h>

#define ENCA            21
#define ENCB            4
#define PIN_PUSHERESC   5

#define THROTTLE_MIN        1500
#define THROTTLE_MAX        2000

Servo pusherESC;

const int encoder_resolution = 4;
const int gearbox = 12;
const int cpt = 500;

volatile long int pos_Main = 0;
const long int ticRatioMotor = 6255;

float enc_pos = 0;
float diff = 0.2;
long int count_per_revolution;

const double pi = 3.14159265359; // it is pi... of course
```

Next, we create the structs for the Encoders A and B.

```c
Encoder fish_enc(ENCA, ENCB);
```
### <br>**The encoder_position() Function**
---
As the title suggests, this function returns a floating point value corresponding to the encoder position from 0-360 degrees, **very precisely**.

Before reading this section, please note that the encoder values we read here are measured in "ticks", where a "tick" is the single smallest rotation that we can detect. For example, a clock has a total of 60 seconds, or "ticks", where a second represents a single "tick". In the case of the encoder, the number of ticks is determined by the manufacturer, but usually, it's in the order of 6000+ ticks! (if that number doesn't impress you, try doing some math and figure out how many degrees represents a single tick on these encoders vs a clock hand)

First, we read the encoder value into a long variable **new_fin**, and then gets the remainder of the division of **new_fin** by the **count_per_revolution** global variable.

```c
  long new_fin = fish_enc.read();
  long x = new_fin - floor(new_fin / count_per_revolution) * count_per_revolution;
```

Hold up, why are we using a long variable instead of an int?
<br><br> **Answer:** The encoder we are using has 500 counts per turn, with 4 readings per count, and even the smallest gearbox we use has a ratio of 15:1. This means we can have at least 500 x 4 x 15 = 30,000, which is approaching very close to the limits of an int variable, which can only store up to 32,768. So in order to use higher gear ratio gearboxes, we definitely need a larger variable to store this data.

Hol' up (2), why not use the modulo operator instead of all that math overhead?
<br><br> **Answer:** It's very inefficient, to the point where we suspect it actually misses the timings.

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

### <br> **The toRadian() function**:
---
As the name suggests, this function converts an angle from degrees to radians. The value of pi is taken from the ```const double pi``` variable declared beforehand.
```C
float toRadian(float degree)
{
  return (degree / 360 * 2 * pi);
}
```

### <br> **The yaw_turn() function**:
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
    float x = ( 1 + (diff*turn*sin( toRadian(enc_pos) )) * pwm);
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
    float x = ( 1 + (diff*turn*sin( toRadian(enc_pos) )) * pwm);
    if (x > 255) x = 255;
    if (x < 30 and speedVal != 0) x = 30;
    if (speedVal == 0) x = 0;

    #if debug
    Serial.print("Second Half PWM: ");
    Serial.println(x);
    #endif

    return round(x);
}
```

### <br> **The resetMotor() function**
---
As the name suggests, it resets the motor. The end.
```C
void resetMotor()
{
    fish_enc.write(0);
}
```

### <br> **The initMotor() function:**
---
Contains the code necessary to initialise the motor, called once during the setup() stage.

```C
void initMotor()
{
    pusherESC.attach(PIN_PUSHERESC);
    pusherESC.writeMicroseconds(THROTTLE_MIN);

    count_per_revolution = cpt*gearbox*encoder_resolution;
}
```

### <br> **The driveMotor() function:**
---
This takes in a *throttle* variable and maps it to 1500 and 2000, before feeding it into the motor's handler.
```C
void driveMotor(int throttle)
{
    throttle = map(throttle, 0, 255, THROTTLE_MIN, THROTTLE_MAX);
    pusherESC.writeMicroseconds(throttle);
}
```