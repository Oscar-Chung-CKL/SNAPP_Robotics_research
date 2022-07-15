# SNAPP The Robotic Fish

## Overview
---
This is a development code that drives SNAPP, our Robotic Fish!

## Code Breakdown
---


### **The Variables**

The variables span from (insert variables here, after cleanup)

### **getData() Function**

Processes the data received from the RC transceiver. In the case of this fish, we use the Arduino's ***Serial 1*** TX/RX pair, and receive the data as a sequence of ***six characters***, stored in the variable called ***char[6]***. The ***first*** of these characters, ***char[0]*** is either an "h", which refers to "home", or in layman terms, ___reset or "home" the encoder to the zero position___, OR, ***char[0]*** can represent "c", which indicates a . The ***next four*** of these characters is then retrieved via a for loop, which is stored in the ***incomingByte[6]*** global variable. The first of these numbers represents the speed value, stored in ***incomingByte[0]***, stored as ***speedVal***, the second represents the pitch value, stored in ***incomingByte[1]***, and so on and so forth for the next yaw value and the roll value. In addition, a kill timer is also started to later detect if the signal is not being received for more than 3 seconds.

The code is thus implemented as follows:
