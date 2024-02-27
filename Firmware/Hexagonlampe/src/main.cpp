#include <Arduino.h>

void setup()
{
    pinMode(2, OUTPUT);
}

void loop()
{
    digitalWrite(2, HIGH); // turn the LED on
    delay(500);            // wait for 500 milliseconds
    digitalWrite(2, LOW);  // turn the LED off
    delay(500);
}