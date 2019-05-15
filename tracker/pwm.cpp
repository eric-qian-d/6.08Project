#include <Arduino.h>

class PWM_608
{
  public:
    int pin; //digital pin class controls
    int period; //period of PWM signal (in milliseconds)
    int on_amt; //duration of "on" state of PWM (in milliseconds)
    PWM_608(int op, float frequency); //constructor op = output pin, frequency=freq of pwm signal in Hz
    void set_duty_cycle(float duty_cycle); //sets duty cycle to be value between 0 and 1.0
    void update(); //updates state of system based on millis() and duty cycle
};