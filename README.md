# SNAPP The Robotic Fish

## Overview
---
This is a development code that drives SNAPP, our Robotic Fish!

## Code Breakdown
---


### **The Variables**

The variables span from (insert variables here, after cleanup)

### **getData() Function**

Processes the data received from the RC transceiver. In the case of this fish, we use the Arduino's ***Serial 1*** TX/RX pair, and receive the data as a sequence of ***six characters***, stored in the variable called ***char[6]***. The ***first*** of these characters, ***char[0]*** is either an "h", which refers to "home", or in layman terms, ___reset or "home" the encoder to the zero position___, OR, ***char[0]*** can represent "c", which indicates a . The ***next four*** of these characters is a sequence of numbers transmitted through the same char, where the ***char[1]*** represents the speed value, stored as ***speedVal***, ***char[2]*** represents