//reference required libraries
#include <DigitalToggle.h> 
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//global variables
int stepCountX = 0; //number of moves along X axis
int stepCountY = 0; //number of moves along Y axis
int distanceX = 1000; //how far to move along X axis
int distanceY = 2000; //how far to move along Y axis
int numberOfImagesX = 5; // how many images to take per row
int numberOfImagesY = 3; // how many rows of images to take
int stepSpeed = 8000; //delay in microseconds between motor steps
int startPosX = 20000; //distance to move away from limitswitches on X axis, dependant on film format
int startPosY = 10000; //distance to move away from limitswitches on X axis, dependant on film format
int manualSpeedX = 0; //delay between steps in manual mode on X axis, governing motor speed
int manualSpeedY = 0; //delay between steps in manual mode on Y axis, governing motor speed
int joyStickreadingX = 0; //current analogue reading of X axis on joystick
int joyStickreadingY = 0; //current analogue reading of Y axis on joystick
int rotaryCounter = 1; //which menu item to display when turning rotary encoder
int lcdloopCounter = 0; //count number of loops to periodically update lcd

//rotary encoder pins
#define ENC_A A0
#define ENC_B A1
#define ENC_PORT PINC

//assign other analogue pins
int joyStickX = A2; //analogue joystick for manual positioning
int joyStickY = A3; //analogue joystick for manual positioning

//assign digital pins
int pushButton = 2;  // Pin 2 = Start/ Stop button
int rotarypushButton = 3; //select/ unselect menu item button
int motorXdir = 4; //X stepper motor direction
int motorXstep = 5; //send step signal to X stepper motor
int motorYdir = 6; //Y stepper motor direction
int motorYstep = 7; //send step signal to Y stepper motor
int limitSwitchesX = 8; //limit switches to stop stepping if end of travel on rail is reached at either end
int limitSwitchesY = 9; //limit switches to stop stepping if end of travel on rail is reached at either end
int focus = 10; //send an autofocus signal to the camera
int shutter = 11; //send a shutter signal to the camera

//pushButton toggle
volatile int buttonState = HIGH; // the current state of the output pin
volatile int reading; // the current reading from the input pin
volatile int previous = LOW; // the previous reading from the input pin
volatile long time = 0; // the last time the output pin was toggled
volatile long debounce = 400; // the debounce time, increase if the output flickers

//rotary pushButton toggle
volatile int rbbuttonState = HIGH; //the current state of the output pin
volatile int rbreading; //the current reading from the input pin
volatile int rbprevious = LOW; //the previous reading from the input pin
volatile long rbtime = 0; //the last time the output pin was toggled
volatile long rbdebounce = 400; //the debounce time, increase if the output flickers

void setup()
{

  Serial.begin(9600); 
  lcd.begin(16, 2);

  pinMode(joyStickX, INPUT); //define pin as an input
  pinMode(joyStickY, INPUT); //define pin as an input
  pinMode(pushButton, INPUT); //define pin as an input
  pinMode(rotarypushButton, INPUT); //define pin as an input
  pinMode(ENC_A, INPUT); //define pin as an input
  pinMode(ENC_B, INPUT); //define pin as an input
  pinMode(limitSwitchesX, INPUT); //define pin as an input
  pinMode(limitSwitchesY, INPUT); //define pin as an input
  pinMode(motorXdir, OUTPUT); //define pin as an output
  pinMode(motorXstep, OUTPUT); //define pin as an output 
  pinMode(motorYdir, OUTPUT); //define pin as an output 
  pinMode(motorYstep, OUTPUT); //define pin as an output
  pinMode(focus, OUTPUT); //define pin as an output 
  pinMode(shutter, OUTPUT); //define pin as an output 

  digitalWrite(motorXstep, LOW); //start with motor step pin low
  digitalWrite(motorYstep, LOW); //start with motor step pin low
  digitalWrite(motorXdir, LOW); //start with motor in default fwd direction
  digitalWrite(motorYdir, LOW); //start with motor in default fwd direction
  digitalWrite(focus, LOW); //start with focus pin low
  digitalWrite(shutter, LOW); //start with shutter pin low
  digitalWrite(ENC_A, HIGH); //start with encoder pin high
  digitalWrite(ENC_B, HIGH); //start with encoder pin high
  digitalWrite(pushButton, HIGH); //start with pushButton pin high
  digitalWrite(rotarypushButton, HIGH); //start with rotarypushButton pin high
  digitalWrite(limitSwitchesX, HIGH); //start with limitSwitchesX pin high
  digitalWrite(limitSwitchesY, HIGH);//start with limitSwitchesY pin high

  lcd.setCursor(0, 0);
  lcd.print("Scanduino");
  delay(2000);
  //findStart();//move to default start point
}

void loop(){

  //X axis manual joystick control{

  joyStickreadingX = analogRead(joyStickX); //get current position of joystick

  int j = joyStickreadingX - 516;
  j = abs(j);
  manualSpeedX = 70000/j;  


  if (joyStickreadingX >= 550){
    digitalWrite(motorXdir, LOW);  //go forwards
    digitalWrite(motorXstep,HIGH);
    delayMicroseconds(manualSpeedX);

  }
  if (joyStickreadingX <= 480){
    digitalWrite(motorXdir, HIGH);  //go backwards
    digitalWrite(motorXstep,HIGH);
    delayMicroseconds(manualSpeedX);
  }

  lcdloopCounter = lcdloopCounter + 1;

  if (lcdloopCounter >= 40){

  lcd.setCursor(0, 0);
  lcd.print("X value: ");
  if (joyStickreadingX < 10){
    lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
  }
  if (joyStickreadingX < 100){
    lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
  }
  if (joyStickreadingX < 1000){
    lcd.print (0, DEC); //adds one leading zero to triple digit Step size numbers on the display
  }
  lcd.print (joyStickreadingX  , DEC);
  
  lcd.setCursor(0, 1);
  lcd.print("Speed value: ");
  if (manualSpeedX < 10){
    lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
  }
  if (manualSpeedX < 100){
    lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
  }
  if (manualSpeedX < 1000){
    lcd.print (0, DEC); //adds one leading zero to triple digit Step size numbers on the display
  }
  lcd.print (manualSpeedX  , DEC);
  
  lcdloopCounter == 0;
  
  }

}














