#include <Wire.h>
#include <NewPing.h>

#define  PIN_BUZZER     4
#define  PIN_TRIGGER    7
#define  PIN_ECHO       8
#define  MAX_DISTANCE 350 // Maximum distance we want to ping for (in centimeters).
                          // Maximum sensor distance is rated at 400-500cm.
int DistanceCm;

NewPing sonar(PIN_TRIGGER, PIN_ECHO, MAX_DISTANCE);

void setup()
{
  pinMode(PIN_BUZZER, OUTPUT);
}

void distance() {
   DistanceCm = sonar.ping_cm(); // 10 pings per second
   
}

void loop()
{
  distance();
  digitalWrite(PIN_BUZZER, HIGH);
  delay(20);
  digitalWrite(PIN_BUZZER, LOW);
  delay(DistanceCm * 10);
  // delay(100);   // waits 100 milliseconds between pings. 29 milliseconds is the shortest delay between 2 pings
}
