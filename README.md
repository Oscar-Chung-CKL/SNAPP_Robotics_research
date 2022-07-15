# SNAPP The Robotic Fish

## <br>Overview
---
This is a development code that drives SNAPP, our Robotic Fish!

## <br>Code Breakdown
---
As the title says, this section will break down every part of the code down the smallest detail.

### <br>**The Variables**
---

The variables span from (insert variables here, after cleanup)

### <br>**getData() Function**
---
Processes the data received from the RC transceiver. In the case of this fish, we use the Arduino's ***Serial 1*** TX/RX pair, and receive the data as a sequence of ***six characters***, stored in the variable called ***char[6]***. The ***first*** of these characters, ***char[0]*** is either an "h", which refers to "home", or in layman terms, ___reset or "home" the encoder to the zero position___, OR, ***char[0]*** can represent "c", which indicates a . The ***next four*** of these characters is then retrieved via a for loop, which is stored in the ***incomingByte[6]*** global variable. The first of these numbers represents the speed value, stored in ***incomingByte[0]***, stored as ***speedVal***, the second represents the pitch value, stored in ***incomingByte[1]***, and so on and so forth for the next yaw value and the roll value. In addition, a kill timer is also started to later detect if the signal is not being received for more than 3 seconds.


<br>**The code is thus implemented as follows:**

First, we check if we are even receiving a signal, and if we are, read the first character.

```
getData{
  if(Serial1.available()>=6){
    int x = Serial1.read();
        .
        .
        .
    // more code below
```


<br>Next, we check if it's a home command by interpreting the character we just read. If it's "h", then set the encoder position to zero, effectively resetting the system.

```
... // previous code

    if (x == 'h')   // Home command
    {
        fish_enc.write(0);   // Reset the encoder to 0
    }

...   // continued
```

<br>If it is not a home command however, we check if it's a "c" or "normal" command, whose following sequence consists of the speed, pitch, yaw and roll values as discussed previously. The code will also set off the kill timer, as shown.

```
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

<br> And that wraps up the getData() function!