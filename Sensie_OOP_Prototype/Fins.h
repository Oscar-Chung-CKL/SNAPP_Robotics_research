#include <Servo.h>

Servo servo1;
Servo servo2;

int servoPin1 = 9;
int servoPin2 = 11;

float s1, s2;
float maxAttacAngle = 40;

class Fins
{
    public:

    void initFin()
    {
        pinMode(servoPin1, OUTPUT);
        pinMode(servoPin2, OUTPUT);
        servo1.attach(servoPin1);
        servo2.attach(servoPin2);
        servo1.write(90);
        servo2.write(90);
    }

    void finControl(int rollVal, int pitchVal)
    {
        s1 = (((43-rollVal)*maxAttacAngle/50 +(42-pitchVal)*maxAttacAngle/50) + 90.0);
        s2 = (((43-rollVal)*maxAttacAngle/50 -(42-pitchVal)*maxAttacAngle/50) + 90.0);
        servo1.write(s1);
        servo2.write(s2);
    }
};