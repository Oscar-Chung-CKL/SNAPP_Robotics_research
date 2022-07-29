/* 
 *  The controller code from June 1, 2022
 *  Tim: idk how the previous code works
 *  so we make a new one
 * 
 */

//For the screen setting
#include <SPI.h>
#include <Wire.h>



// Timers
long int timer1 = 0;       //store the time when transmiting


// The byte array to store the message to be sent
byte msg[6];


//Variables for transmission
//signal1:R;     signal2:U;     signal3:T;      signal4:P
String sum_str;
char message[6];


//Button default values
int xMap, yMap;
int tMap = 5;         //default value of turnVal
int pMap = 0;         //default value of power

//declare Aunduino joystick
int sensorValueX = analogRead(A0);    
int sensorValueY = analogRead(A1);




void setup() {
   // Setup Serial
  Serial.begin(19200,SERIAL_8O1); // Hello, you might need to change it later
  //Serial.begin(19200);

  
//  Serial1.begin(38400,SERIAL_8E1);
//  Serial2.begin(38400,SERIAL_8E1);
//  Serial3.begin(38400,SERIAL_8E1);
//  only enable when mega is used
  
  
  

  //Setup Arduino pins
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
//  pinMode(A4, INPUT);
//  pinMode(A5, INPUT);
}

void loop() {
  
//  time_now = millis();

  //Update control variables from buttons
  //Reading Joystick Value
  sensorValueX = analogRead(A0);
  sensorValueY = analogRead(A1);
  xMap = map(sensorValueX, 50, 900 , 10, 90);
  //yMap = map(sensorValueY, 30, 1000, 1, 9);

  //yMap = map(sensorValueY, 0, 1024, 0, 9);
  yMap = map(sensorValueY, 50, 920, 10, 90);

  pMap =   map(analogRead(A4), 0, 1023 , 0, 9); 
  tMap =   map(analogRead(A3), 0, 900 , 1, 9);
  
    
    // Send signal after every 50 ms
   // send command
    if (millis() - timer1 > 50) {
      timer1 = millis(); //update timer1
  
      // Prepare string and send to receiver
  
      //speed, pitch, roll, yaw, in that order
      if (yMap < 10){
        yMap = 10;
      }
      if (xMap < 10){
        xMap = 10;
      }
      if (xMap > 90){
        xMap = 90;
      }
      if (yMap > 90){
        yMap = 90;
      }
      
      if (tMap <= 0){
        tMap = 1;
      }
      if (pMap <= 0){
        pMap = 0;
      }
      if (tMap > 9){
        tMap = 9;
      }
      if (pMap > 9){
        xMap = 9;
      }

      int newT = map(tMap, 1,9,9,1);
      sum_str = "c" + String(pMap) + String( (char)yMap) + String(newT) + String( (char) xMap) + "e"; //payload + checksum
      strcpy(message,sum_str.c_str());
      //Serial.println((String) xMap + " " +yMap);
      String(message).getBytes(msg,7);
      //Serial.println((String) yMap + " " + xMap);
      for (int i = 0; i < 6; i++){ 
        Serial.write(msg[i]);
        Serial.flush();
  
         //enable if using mega
        
//        Serial1.write(msg[i]);
//        Serial1.flush();
//  
//        Serial2.write(msg[i]);
//        Serial2.flush();
//        
//        Serial3.write(msg[i]);
//        Serial3.flush();
        
      }
      //Serial.println();
     }
  
  
    
    /*
    Serial1.print(message);
    Serial1.flush();
    
    Serial2.print(message);
    Serial2.flush();
    
    Serial3.print(message);
    Serial3.flush();
    */

   
   //This is the get sensor function, not tested yet
//    getSensor();
//    display.print("Water Level: ");
//    display.println(waterLevel);
//    display.print("Tempature ");
//    display.print(tempature);

    

}
