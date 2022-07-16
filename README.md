# SNAPP The Robotic Fish

## <br>Overview
This is a development code that drives SNAPP, our Robotic Fish!

## <br>Code Breakdown
As the title says, this section will break down every part of the code down the smallest detail.

### <br>**The Variables**
---

The variables span from (insert variables here, after cleanup)

### <br>**The getData() Function**
---
Processes the data received from the RC transceiver. In the case of this fish, we use the Arduino's ***Serial 1*** TX/RX pair, and receive the data as a sequence of ***six characters***, stored in the variable called ***char[6]***. The ***first*** of these characters, ***char[0]*** is either an "h", which refers to "home", or in layman terms, ___reset or "home" the encoder to the zero position___, OR, ***char[0]*** can represent "c", which indicates a normal transmision. The ***next four*** of these characters is then retrieved via a for loop, which is stored in the ***incomingByte[6]*** global variable. The first of these numbers represents the speed value, stored in ***incomingByte[0]***, which is then reassigned to ***speedVal***, the second represents the pitch value, reassiged to **pitchVal**, stored in ***incomingByte[1]***, and so on and so forth for the next yaw value and the roll value for convenience. In addition, a kill timer is also started to later detect if the signal is not being received for more than 3 seconds.


<br>**The code is thus implemented as follows:**

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
