#include <Encoder.h>
#include <Servo.h>

#define ENCA            21
#define ENCB            4
#define PIN_PUSHERESC   5

#define THROTTLE_MIN        1500
#define THROTTLE_MAX        2000
#define THROTTLE_BAKE       1000

Servo pusherESC;

const int encoder_resolution = 4;
const int gearbox = 12;
const int cpt = 500;

volatile long int pos_Main = 0;
const long int ticRatioMotor = 6255;

float enc_pos = 0;
float diff = 0.2;
long int count_per_revolution;

Encoder fish_enc(ENCA, ENCB);

class Motor
{
    public:

    float encoderPosition()
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

    int yaw_turn(int pwm, int turn, int speedVal)
    {
        //Map the turn value to +/- 4, which is easier to understand
        turn -= 5;
        //Accepts a pwm signal and outputs a pwm signal from 0-255

        if(turn == 0)
        {
            #if debug
            Serial.print("Normal PWM: ");
            Serial.println(pwm);
            Serial.flush();
            #endif

            return (pwm);
        }
  
        if( (enc_pos > 89.0) && (enc_pos < 271.0))
        {
            float x = (1 + turn*diff)*pwm;
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
            float x = (1 - turn*diff)*pwm; 
            if (x > 255) x = 255;
            if (x < 30 and speedVal != 0) x = 30;
            if (speedVal == 0) x = 0;
    
            #if debug
            Serial.print("Second Half PWM: ");
            Serial.println(x);
            #endif

            return round(x);
        }
    }

    void resetMotor()
    {
        fish_enc.write(0);
    }

    void initMotor()
    {
        pusherESC.attach(PIN_PUSHERESC);
        pusherESC.writeMicroseconds(THROTTLE_MIN);

        count_per_revolution = cpt*gearbox*encoder_resolution;
    }

    void driveMotor(int throttle)
    {
        throttle = map(throttle, 0, 255, THROTTLE_MIN, THROTTLE_MAX);
        pusherESC.writeMicroseconds(throttle);
    }
};
