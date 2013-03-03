#include <DigitalToggle.h> 

//global variables
int stepCountX = 0; //number of moves along X axis
int stepCountY = 0; //number of moves along Y axis
int distanceX = 1000; //how far to move along X axis
int distanceY = 2000; //how far to move along Y axis
int numberOfImagesX = 5; // how many images to take per row
int numberOfImagesY = 3; // how many rows of images to take

//assign pins
int pushButton = 2;  // Pin 2 = Start/ Stop button
int motorXdir = 4;
int motorXstep = 5;
int motorYdir = 6;
int motorYstep = 7;
int focus = 8;
int shutter = 9;

//pushButton toggle
volatile int buttonState = HIGH;      // the current state of the output pin
volatile int reading;           // the current reading from the input pin
volatile int previous = LOW;    // the previous reading from the input pin
volatile long time = 0;         // the last time the output pin was toggled
volatile long debounce = 400;   // the debounce time, increase if the output flickers

void setup()
{

Serial.begin(9600); 

attachInterrupt(0, buttonChange, CHANGE);  // Button on interrupt 0 - pin 2

pinMode(pushButton, INPUT);  //define pin as an input
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
}

void loop(){
  
if (buttonState == HIGH){ //this section configures settings using a simple lcd menu system
//menu section goes here
}  

else{
for (int i = 0; i < numberOfImagesY - 1; i++){ //Repeat until count equals numberOfImagesY - 1
for (int i = 0; i < numberOfImagesX - 1; i++){ //Repeat the function until count equals numberOfImagesX - 1
digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
digitalWrite(shutter, HIGH); // Trigger camera shutter
delay(200); // Small delay needed for camera to process above signals
digitalWrite(shutter, LOW); // Switch off camera trigger signal
digitalWrite(focus, LOW); // Switch off camera focus signal
delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
for (int i = 0; i < distanceX; i++){ // Move as far as distanceX
digitalWrite(motorXstep, LOW); // This LOW to HIGH change is what creates the
digitalWrite(motorXstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
delayMicroseconds(8000); // Delay time between steps, too fast and motor stalls
}
}
digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
digitalWrite(shutter, HIGH); // Trigger camera shutter
delay(200); // Small delay needed for camera to process above signals
digitalWrite(shutter, LOW); // Switch off camera trigger signal
digitalWrite(focus, LOW); // Switch off camera focus signal
delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
digitalToggle(motorXdir);
for (int i = 0; i < distanceY; i++){ // Move as far as distanceY - moves down to the next row
digitalWrite(motorYstep, LOW); // This LOW to HIGH change is what creates the
digitalWrite(motorYstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
delayMicroseconds(8000); // Delay time between steps, too fast and motor stalls
}
}
//Final row
for (int i = 0; i < numberOfImagesX - 1; i++){ //Repeat the function until count equals numberOfImagesX - 1
digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
digitalWrite(shutter, HIGH); // Trigger camera shutter
delay(200); // Small delay needed for camera to process above signals
digitalWrite(shutter, LOW); // Switch off camera trigger signal
digitalWrite(focus, LOW); // Switch off camera focus signal
delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
for (int i = 0; i < distanceX; i++){ // Move as far as distanceX
digitalWrite(motorXstep, LOW); // This LOW to HIGH change is what creates the
digitalWrite(motorXstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
delayMicroseconds(8000); // Delay time between steps, too fast and motor stalls
}
}
digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
digitalWrite(shutter, HIGH); // Trigger camera shutter
delay(200); // Small delay needed for camera to process above signals
digitalWrite(shutter, LOW); // Switch off camera trigger signal
digitalWrite(focus, LOW); // Switch off camera focus signal
delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
digitalToggle(motorXdir);

//Return to start position
if ((numberOfImagesY % 2) != 0) { // If number of rows is odd move camera back to left
digitalWrite(motorXdir, HIGH); //Reverse X axis motor
for (int i = 0; i < distanceX * (numberOfImagesX - 1); i++){
digitalWrite(motorXstep, LOW); // This LOW to HIGH change is what creates the
digitalWrite(motorXstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
delayMicroseconds(8000); // Delay time between steps, too fast and motor stalls
}
digitalWrite(motorXdir, LOW); //Set motor back to default direction
}

digitalWrite(motorYdir, HIGH); //Reverse Y axis motor
for (int i = 0; i < (numberOfImagesY - 1) * distanceY; i++){
digitalWrite(motorYstep, LOW); // This LOW to HIGH change is what creates the
digitalWrite(motorYstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
delayMicroseconds(8000); // Delay time between steps, too fast and motor stalls
}
digitalWrite(motorYdir, LOW); //Set motor back to default direction


delay(8000);
Serial.print("finished");
buttonState = HIGH; // Return to menu options
}
}

void buttonChange(){ //function to read the current state of the push button

  reading = digitalRead(pushButton);

  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
    if (buttonState == HIGH)
      buttonState = LOW;
    else
      buttonState = HIGH;

    time = millis();    
  }

  previous = reading;
} 
