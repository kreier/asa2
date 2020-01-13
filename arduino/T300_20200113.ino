// Setup with 10 keys as defined in the root folder and used with BitBlue on iOS
//
// Matrix for control - mode1 - modified for Sean and Yau
//
// key index text     M1[] M2[] SPD    maxtrix unusual becaus of color led direction:
//  F    0   forward  HIGH LOW  255    green  green
//  B    1   backward LOW  HIGH 255    orange orange
//  L    2   left     HIGH HIGH 255  
//  R    3   right    LOW  LOW  255  
//  T    4   triangle
//  C    5   circle
//  X    6   stop
//  Q    7   square
//  M    8   menu
//  S    9   start

#include <avr/wdt.h>  // for reset / reboot
#include <NewPing.h>
#include <Servo.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // include i/o class header

#define PIN_BUZZER     4
#define PIN_TRIGGER    7
#define PIN_ECHO       8
#define PIN_SERVO      9
#define PIN_E1        10 // enable - with PWM
#define PIN_M1        12
#define PIN_E2        11 // PWM
#define PIN_M2        13
#define MAX_DISTANCE  50 // cm for the ultrasonic sensor
#define RELAX_SONAR  200 // ms to check for obstacles
#define RELAX_DRIVE  412 // ms to drive until check direction
#define MIN1           5 // left and right
#define MAX1         175
#define MIN2          45 // up and down
#define MAX2         120
#define MIN3          65 // forward and back
#define MAX3         140
#define MIN4          90 // open and close
#define MAX4         125

const char keyindex[] = "ACDBEFGHMS_";
//const char keyindex[] = "FBLRTCXQMS_";
const int  motor1[] = {1, 0, 1, 0};
const int  motor2[] = {0, 1, 1, 0};
const String text[]  = {"forward","backward","left  ","right ","triange","circle","stop  ","square","select","start"};
const String text1[] = {"forward","backward","left  ","right ","Turbo ","Faster ","Stop  ","Slower ","Drive Mode","Start","invalid"};
const String text2[] = {"forward","backward","left  ","right ","up    ", "open  ","down  ","close ","Robot arm", "Start","invalid"};
const String text3[] = {"forward","backward","left  ","right ","triange","circle","stop  ","square","Ultrasonic","Start","invalid"};
const String text4[] = {"forward","backward","left  ","right ","triange","circle","stop  ","square","Autonomous","Start","invalid"};

char BTinput = '0';  // char input via bluetooth
int  BTkey   =  0;   // converted to numerical value
int  spd = 0;        // speed of the robot
int  mode = 1;
String message = "stop";
int verzug = 200;
int pos1 = 90;       // variable to store the servo position
int pos2 = MIN2;
int pos3 = MIN3;
int pos4 = MIN4;
int pos5 = 90;
int enable_ultrasonic = 0;

unsigned long timer_sonar;
unsigned long timer_drive;
int DistanceCm = 99;

hd44780_I2Cexp lcd;
Servo serv1;    // left and right    45 to 135 
Servo serv2;    // up and down       80 to 100
Servo serv3;    // forward and back  80 to 100
Servo serv4;    // open and close    80 to 100
Servo serv5;    // ultrasonic sensor 10 to 170
NewPing sonar(PIN_TRIGGER, PIN_ECHO, MAX_DISTANCE);

void distance() {
  DistanceCm = sonar.ping_cm(); // 10 pings per second
  if (DistanceCm == 0) DistanceCm = MAX_DISTANCE;
  lcd.setCursor(11,0);
  lcd.print(DistanceCm); 
  lcd.print(" cm  ");
  if(DistanceCm < 20) {
    if(enable_ultrasonic == 1) beep(1);
    if(mode == 3) beep(1);
    timer_sonar -= (RELAX_SONAR - DistanceCm * 19);
    if(DistanceCm < 10) { // please stop
      spd = 0;
      analogWrite(PIN_E1, spd);
      analogWrite(PIN_E2, spd);
    }
  }   
}

void beep(int beeps) {
  for(int i = 0; i < beeps; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    lcd.noBacklight();
    delay(20);
    digitalWrite(PIN_BUZZER, LOW);
    lcd.backlight();    
    delay(80);
  }
}

void reboot() {  // declare reboot function on pin 0 to make PWM available again
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while(1) {;}  
}

void disp(int lcd_x, int lcd_y, String text) {
  Serial1.print( text );        // return via bluetooth
  lcd.setCursor(lcd_x, lcd_y);
  lcd.print( text );
  lcd.print("    ");
}

void statuslcd() {
  lcd.setCursor(5, 0);
  lcd.print(mode);                  // Mode 1-4
  lcd.setCursor(7, 0);
  lcd.print(DistanceCm);            // Distance ultrasonic
  lcd.print(" ");
  lcd.setCursor(10, 0);
  if(spd < 100) lcd.print(" ");     // Speed 10-255
  lcd.print(spd);
  lcd.print(" ");
  lcd.setCursor(14, 0);
  lcd.print(digitalRead(PIN_M1));   // Motor 1 and 2
  lcd.print(digitalRead(PIN_M2));   // forward (1) or backward (0)
  lcd.setCursor(12, 1);
  lcd.print(millis() / 1000);       // seconds the system is running
}

void setup() {
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO,    INPUT);
  pinMode(PIN_SERVO,   OUTPUT);
  pinMode(PIN_BUZZER,  OUTPUT);
  pinMode(PIN_M1, OUTPUT);
  pinMode(PIN_M2, OUTPUT);
  pinMode(PIN_E1, OUTPUT);
  pinMode(PIN_E2, OUTPUT);
  digitalWrite(PIN_M1, HIGH);        // forward (green)
  digitalWrite(PIN_M2, LOW);         // forward (green)
  analogWrite(PIN_E1, spd);
  analogWrite(PIN_E2, spd);
  Serial1.begin(9600);               // HC-10 BLE on pin 0 and 1  
  timer_sonar = millis();
  timer_drive = millis();
  //  serv1.attach(A0);   no servo attached here - PWM on pin 10 immediately lost!
  //  serv1.write(pos1);  let's write this later
  //  serv1.detach();     does not work, PWM is lost (issue since 2016) - reset in mode 3
  beep(2);              // https://github.com/arduino-libraries/Servo/issues/1
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("T300");
  disp(0, 1, "Drive mode");
}


void loop() {
  if (Serial1.available()) 
  {
    BTinput = Serial1.read();
    for(BTkey = 0; BTkey < 10; BTkey += 1) {
      if(BTinput == keyindex[BTkey]) {
        break;
      }
    }
    if(BTkey == 8) {
      mode++;
      if(mode > 4) mode = 1;
      beep(mode);
    }     
    if(mode == 1) { // Mode 1: Drive
      message = text1[BTkey];
      if(BTkey > -1 && BTkey < 4) {
        digitalWrite(PIN_M1, motor1[BTkey]);
        digitalWrite(PIN_M2, motor2[BTkey]);   
      }
      if(BTkey == 4) {
        spd = 255;
      }
      if(BTkey == 5) {
        spd += 15;
        message += spd;
      }
      if(BTkey == 7) {
        spd -= 15;
        message += spd;
      }
      if(spd < 0) spd = 0;
      if(spd > 255) spd = 255;
      if(BTkey == 6) spd = 0;
      if(BTkey == 9) {           // start ultrasonic poweroff
        if(enable_ultrasonic == 0) {
          enable_ultrasonic = 1;
          message = "Sonar on ";
        } else {
          enable_ultrasonic = 0;
          message = "Sonar OFF ";
        }
      }
    }
   
    if(mode == 2) { // Mode 2: Robot arm control
      message = text2[BTkey];
      if(BTkey == 0) pos3 += 5;
      if(BTkey == 1) pos3 -= 5;
      if(BTkey == 2) pos1 += 5;
      if(BTkey == 3) pos1 -= 5;
      if(BTkey == 4) pos2 += 5;
      if(BTkey == 5) pos4 += 5;
      if(BTkey == 6) pos2 -= 5;
      if(BTkey == 7) pos4 -= 5;
      if(BTkey == 9) { // start and attach the servos!
        pos1 = 90;
        pos2 = MIN2;
        pos3 = MIN3;
        pos4 = MIN4;
        serv1.attach(A0);    // attach the servos, PWM on pin 10 is lost
        serv2.attach(A1);
        serv3.attach(A2);
        serv4.attach(A3);
        serv5.attach(9);           
      }
      if(pos1 < MIN1) pos1 = MIN1;
      if(pos1 > MAX1) pos1 = MAX1;
      if(pos2 < MIN2) pos2 = MIN2;
      if(pos2 > MAX2) pos2 = MAX2;
      if(pos3 < MIN3) pos3 = MIN3;
      if(pos3 > MAX3) pos3 = MAX3;
      if(pos4 < MIN4) pos4 = MIN4;
      if(pos4 > MAX4) pos4 = MAX4;
      serv1.write(pos1);
      serv2.write(pos2);
      serv3.write(pos3);
      serv4.write(pos4);
      serv5.write(pos5);
    }
    if(mode == 3) { // Mode 3: Drive with ultrasonic distance - no servo
      message = text3[BTkey];
      if(BTkey == 9){
        reboot();         //    resets the Arduino - PWM is available again!
      }
    }
    if(mode == 4) { // Mode 4: Autonomous Drive - no servo
      message = text4[BTkey];
    }
    disp(0, 1, message);
    analogWrite(PIN_E1, spd);
    analogWrite(PIN_E2, spd);
  }
  if(millis() > timer_sonar + RELAX_SONAR) {
    timer_sonar = millis();
    distance();
    statuslcd();
  }
}
