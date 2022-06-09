void setup() {
  Serial.begin(19200);
  Serial1.begin(19200);
  Serial2.begin(19200);
  Serial3.begin(19200);
}

long int counter;

void loop() {

  //in case you need a timer:
  counter = millis();
  if (counter % 30 == 0){
    Serial.println("c1234e");
    Serial1.println("c1234e");
    Serial2.println("c1234e");
    Serial3.println("c1234e");

    Serial.flush();
    Serial1.flush();
    Serial2.flush();
    Serial3.flush();
    //Serial.println();
  }

  
  // speed, pitch, roll, yaw, in that order
  //Serial.print("c9999e");
}
