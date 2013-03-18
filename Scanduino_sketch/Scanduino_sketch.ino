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
int menuItem = 1; //which menu item to display when turning rotary encoder

//assign analogue pins
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
int manualControlButton = 12; //toggle between manual control and setup menu

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

//manual pushControlButton toggle
volatile int mcbuttonState = HIGH; //the current state of the output pin
volatile int mcreading; //the current reading from the input pin
volatile int mcprevious = LOW; //the previous reading from the input pin
volatile long mctime = 0; //the last time the output pin was toggled
volatile long mcdebounce = 400; //the debounce time, increase if the output flickers

void setup()
{

  Serial.begin(9600); 
  lcd.begin(16, 2);

  attachInterrupt(0, buttonChange, CHANGE);  // Button on interrupt 0 - pin 2
  attachInterrupt(1, rotarybuttonChange, CHANGE);  // Button on interrupt 1 - pin 3

  pinMode(joyStickX, INPUT); //define pin as an input
  pinMode(joyStickY, INPUT); //define pin as an input
  pinMode(pushButton, INPUT); //define pin as an input
  pinMode(rotarypushButton, INPUT); //define pin as an input
  pinMode(manualControlButton, INPUT); //define pin as an input
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
  digitalWrite(pushButton, HIGH); //start with pushButton pin high
  digitalWrite(rotarypushButton, HIGH); //start with rotarypushButton pin high
  digitalWrite(manualControlButton, HIGH); //start with manualControlButton pin high
  digitalWrite(limitSwitchesX, HIGH); //start with limitSwitchesX pin high
  digitalWrite(limitSwitchesY, HIGH);//start with limitSwitchesY pin high

  lcd.setCursor(0, 0);
  lcd.print("Scanduino");
  delay(2000);
  //findStart();//move to default start point
}

void loop(){

  if (buttonState == HIGH){ //this section provides manual control and configures settings using a simple lcd menu system

    manualControlButtonChange();//check current toggle status of the manual control button

    if (mcbuttonState == HIGH){

      manualControl(); //manual control function using joystick

    }

    else{
      
        uint8_t buttons = lcd.readButtons();
        
        if (buttons & BUTTON_RIGHT) { //if right button pushed go forward one menu item
         menuItem++;
        }  
        
        if (buttons & BUTTON_LEFT) { //if left button pushed go back one menu item
         menuItem--;
        }   

        menuItem = constrain(menuItem, 0, 7); //limits choice to specified range

        if (menuItem == 7){ //when counter value exceeds number of menu items
          menuItem = 1; //reset it to 1 again to create a looping navigation
        }

        if (menuItem == 0){ //when counter value goes below minimum number of menu items
          menuItem = 6; //reset it to 6 again to create a looping navigation
        }  

      switch (menuItem) {

      case 1: //this menu screen changes the number of rows to scan

        if (buttons & BUTTON_UP) { //if up button pushed increment menu item value
         numberOfImagesY++;
        }   
        
        if (buttons & BUTTON_DOWN) { //if down button pushed decrement menu item value
         numberOfImagesY--;
        } 
          
        numberOfImagesY = constrain(numberOfImagesY, 1, 25); //limits choice of input step size to specified range

        lcd.setCursor(0, 0);
        lcd.print("Number of rows: ");
        lcd.setCursor(0, 1);

        if (numberOfImagesY < 10){
          lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
        }
        if (numberOfImagesY < 100){
          lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }

        lcd.print (numberOfImagesY, DEC);

        lcd.print("           "); //fill rest of display line with empty chars to overwrite conetnt of previous screen  
        break;



      case 2: //this menu screen changes the height of rows  

        if (buttons & BUTTON_UP) { //if up button pushed increment menu item value
         distanceY = (distanceY + 10);
        }   
        
        if (buttons & BUTTON_DOWN) { //if down button pushed decrement menu item value
         distanceY = (distanceY - 10);
        } 
          
        distanceY = constrain(distanceY, 10, 9990); //limits choice of input step size to specified range

        lcd.setCursor(0, 0);
        lcd.print("Height of rows: ");
        lcd.setCursor(0, 1);

        if (distanceY < 10){
          lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
        }
        if (distanceY < 100){
          lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }
        if (distanceY < 1000){
          lcd.print (0, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }

        lcd.print (distanceY, DEC);

        lcd.print("           "); //fill rest of display line with empty chars to overwrite conetnt of previous screen   

        break;

      case 3: //this menu screen changes the number of columns to scan per row

        if (buttons & BUTTON_UP) { //if up button pushed increment menu item value
         (numberOfImagesX++);
        }   
        
        if (buttons & BUTTON_DOWN) { //if down button pushed decrement menu item value
         (numberOfImagesX--);
        } 

        numberOfImagesX = constrain(numberOfImagesX, 1, 25); //limits choice of input step size to specified range

        lcd.setCursor(0, 0);
        lcd.print("Number of cols: ");
        lcd.setCursor(0, 1);

        if (numberOfImagesX < 10){
          lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
        }
        if (numberOfImagesX < 100){
          lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }

        lcd.print (numberOfImagesX, DEC);

        lcd.print("           "); //fill rest of display line with empty chars to overwrite conetnt of previous screen  

        break;

      case 4: //this menu screen changes the width of columns

        if (buttons & BUTTON_UP) { //if up button pushed increment menu item value
         distanceX = (distanceX + 10);
        }   
        
        if (buttons & BUTTON_DOWN) { //if down button pushed decrement menu item value
         distanceX = (distanceX - 10);
        } 

        distanceX = constrain(distanceX, 10, 9990); //limits choice of input step size to specified range

        lcd.setCursor(0, 0);
        lcd.print("Width of cols:  ");
        lcd.setCursor(0, 1);

        if (distanceX < 10){
          lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
        }
        if (distanceX < 100){
          lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }
        if (distanceX < 1000){
          lcd.print (0, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }

        lcd.print (distanceX, DEC);

        lcd.print("           "); //fill rest of display line with empty chars to overwrite conetnt of previous screen  

        break;

      case 5: //this menu screen changes the start position on the X axis

        if (buttons & BUTTON_UP) { //if up button pushed increment menu item value
         startPosX = (startPosX + 10);
        }   
        
        if (buttons & BUTTON_DOWN) { //if down button pushed decrement menu item value
         startPosX = (startPosX - 10);
        }

        startPosX = constrain(startPosX, 10, 9990); //limits choice of input step size to specified range

        lcd.setCursor(0, 0);
        lcd.print("Xaxis start pos:");
        lcd.setCursor(0, 1);

        if (startPosX < 10){
          lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
        }
        if (startPosX < 100){
          lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }
        if (startPosX < 1000){
          lcd.print (0, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }

        lcd.print (startPosX, DEC);

        lcd.print("            "); //fill rest of display line with empty chars to overwrite conetnt of previous screen  

        break;

      case 6: //this menu screen changes the start position on the Y axis 

        if (buttons & BUTTON_UP) { //if up button pushed increment menu item value
         startPosY = (startPosY + 10);
        }   
        
        if (buttons & BUTTON_DOWN) { //if down button pushed decrement menu item value
         startPosY = (startPosY - 10);
        }

        startPosY = constrain(startPosY, 10, 9990); //limits choice of input step size to specified range


        lcd.setCursor(0, 0);
        lcd.print("Yaxis start pos:");
        lcd.setCursor(0, 1);

        if (startPosY < 10){
          lcd.print (000, DEC); //adds three leading zeros to single digit Step size numbers on the display
        }
        if (startPosY < 100){
          lcd.print (00, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }
        if (startPosY < 1000){
          lcd.print (0, DEC); //adds two leading zeros to double digit Step size numbers on the display
        }

        lcd.print (startPosY, DEC);

        lcd.print("            "); //fill rest of display line with empty chars to overwrite conetnt of previous screen    

        break;

      }
    }

  }  

  else{ //this section runs the actual scan using the settings chosen in the previous section
    for (int i = 0; i < numberOfImagesY - 1; i++){ //Repeat until count equals numberOfImagesY - 1
      for (int i = 0; i < numberOfImagesX - 1; i++){ //Repeat the function until count equals numberOfImagesX - 1
        digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
        digitalWrite(shutter, HIGH); // Trigger camera shutter
        delay(200); // Small delay needed for camera to process above signals
        digitalWrite(shutter, LOW); // Switch off camera trigger signal
        digitalWrite(focus, LOW); // Switch off camera focus signal
        delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
        int i = 0;
        while (i < distanceX && digitalRead(limitSwitchesX) == HIGH){ // Move as far as distanceX
          digitalWrite(motorXstep, LOW); // This LOW to HIGH change is what creates the
          digitalWrite(motorXstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
          delayMicroseconds(stepSpeed); // Delay time between steps, too fast and motor stalls
          i = i + 1;
          if (digitalRead(limitSwitchesX) == LOW){ //stop motor and reverse if limit switch hit
            retreatX();
            break;
          }
        }
        digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
        digitalWrite(shutter, HIGH); // Trigger camera shutter
        delay(200); // Small delay needed for camera to process above signals
        digitalWrite(shutter, LOW); // Switch off camera trigger signal
        digitalWrite(focus, LOW); // Switch off camera focus signal
        delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
        digitalToggle(motorXdir);
        int j = 0;
        while (j < distanceY && digitalRead(limitSwitchesY) == HIGH){ // Move as far as distanceY - moves down to the next row
          digitalWrite(motorYstep, LOW); // This LOW to HIGH change is what creates the
          digitalWrite(motorYstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
          delayMicroseconds(stepSpeed); // Delay time between steps, too fast and motor stalls
          j = j + 1;
          if (digitalRead(limitSwitchesY) == LOW){ //stop motor and reverse if limit switch hit
            retreatY();
            break;
          }
        }
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
      int i = 0;
      while (i < distanceX && digitalRead(limitSwitchesX) == HIGH){ // Move as far as distanceX
        digitalWrite(motorXstep, LOW); // This LOW to HIGH change is what creates the
        digitalWrite(motorXstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
        delayMicroseconds(stepSpeed); // Delay time between steps, too fast and motor stalls
        i = i + 1;
        if (digitalRead(limitSwitchesX) == LOW){ //stop motor and reverse if limit switch hit
          retreatX();
          break;
        }
      }
      digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
      digitalWrite(shutter, HIGH); // Trigger camera shutter
      delay(200); // Small delay needed for camera to process above signals
      digitalWrite(shutter, LOW); // Switch off camera trigger signal
      digitalWrite(focus, LOW); // Switch off camera focus signal
      delay(2000); //Pause to allow for camera to take picture with 2 sec mirror lockup
      digitalToggle(motorXdir);
    }

    //Return to start position
    if ((numberOfImagesY % 2) != 0) { // If number of rows is odd move camera back to left
      digitalWrite(motorXdir, HIGH); //Reverse X axis motor
      int i = 0;
      while (i < distanceX * (numberOfImagesX - 1) && digitalRead(limitSwitchesX) == HIGH){
        digitalWrite(motorXstep, LOW); // This LOW to HIGH change is what creates the
        digitalWrite(motorXstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
        delayMicroseconds(stepSpeed); // Delay time between steps, too fast and motor stalls
        i = i + 1;
        if (digitalRead(limitSwitchesX) == LOW){ //stop motor and reverse if limit switch hit
          retreatX();
          break;
        }
        digitalWrite(motorXdir, LOW); //Set motor back to default direction
      }

      digitalWrite(motorYdir, HIGH); //Reverse Y axis motor
      int j = 0;
      while (j < (numberOfImagesY - 1) * distanceY && digitalRead(limitSwitchesY) == HIGH){
        digitalWrite(motorYstep, LOW); // This LOW to HIGH change is what creates the
        digitalWrite(motorYstep, HIGH); // "Rising Edge" so the easydriver knows to when to step
        delayMicroseconds(stepSpeed); // Delay time between steps, too fast and motor stalls
        j = j + 1;
        if (digitalRead(limitSwitchesY) == LOW){ //stop motor and reverse if limit switch hit
          retreatY();
          break;
        }
        digitalWrite(motorYdir, LOW); //Set motor back to default direction


        delay(8000);
        Serial.print("finished");
        findStart();//move to default start point
        buttonState = HIGH; // Return to menu options
      }

    }
  }
}

void buttonChange(){ //function to read the current state of the push button

  reading = digitalRead(pushButton);

  if (reading == LOW && previous == HIGH && millis() - time > debounce) {
    if (buttonState == HIGH)
      buttonState = LOW;
    else
      buttonState = HIGH;

    time = millis();    
  }

  previous = reading;
} 

void rotarybuttonChange(){ //function to read the current state of the push button

  rbreading = digitalRead(rotarypushButton);

  if (rbreading == LOW && rbprevious == HIGH && millis() - rbtime > rbdebounce) {
    if (rbbuttonState == HIGH)
      rbbuttonState = LOW;
    else
      rbbuttonState = HIGH;

    rbtime = millis();    
  }

  rbprevious = rbreading;
} 

void manualControlButtonChange(){ //function to read the current state of the push button

  mcreading = digitalRead(manualControlButton);

  if (mcreading == LOW && mcprevious == HIGH && millis() - mctime > mcdebounce) {
    if (mcbuttonState == HIGH)
      mcbuttonState = LOW;
    else
      mcbuttonState = HIGH;

    mctime = millis();    
  }

  mcprevious = mcreading;
} 

void findStart (){ //uses limitswitch feedback to reset carriage to a preset start position

  //first reset the x axis
  digitalWrite(motorXdir, HIGH); //reverse stepper direction

  while (digitalRead(limitSwitchesX) == HIGH){ //keep reversing until the limitswitch is pressed

    digitalWrite(motorXstep, LOW);  //this LOW to HIGH change is what creates the
    digitalWrite(motorXstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
    delayMicroseconds(stepSpeed); //delay time between steps, too fast and motor stalls
  }

  digitalWrite(motorXdir, LOW); //reset stepper direction

  int i=0;

  while (digitalRead(limitSwitchesX) == LOW || i < startPosX) //iterate doStep signal for as long as the limit switch remains pressed
    //and until preset distance away from switch is reached 

  {  
    digitalWrite(motorXstep, LOW); //this LOW to HIGH change is what creates the
    digitalWrite(motorXstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
    delayMicroseconds(stepSpeed); //delay time between steps, too fast and motor stalls
    i = i + 1;
  }


  //then reset the y axis
  digitalWrite(motorYdir, LOW); //reverse stepper direction

  while (digitalRead(limitSwitchesY) == HIGH){ //keep reversing until the limitswitch is pressed

    digitalWrite(motorYstep, LOW);  //this LOW to HIGH change is what creates the
    digitalWrite(motorYstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
    delayMicroseconds(stepSpeed); //delay time between steps, too fast and motor stalls
  }

  digitalWrite(motorYdir, LOW); //reset stepper direction

  int j=0;

  while (digitalRead(limitSwitchesY) == LOW || j < startPosY) //iterate doStep signal for as long as the limit switch remains pressed
    //and until preset distance away from switch is reached 

  {  
    digitalWrite(motorYstep, LOW); //this LOW to HIGH change is what creates the
    digitalWrite(motorYstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
    delayMicroseconds(stepSpeed); //delay time between steps, too fast and motor stalls
    j = j + 1;
  }


}


void retreatX(){ //moves platform back into safe zone if a limit switch on the X axis is tripped during operation

  digitalToggle(motorXdir); //reverse motor direction to move away from limitswitch

  lcd.setCursor(0, 0);
  lcd.print("End of travel!  ");
  lcd.setCursor(0, 1);
  lcd.print("Reversing...    ");

  while (digitalRead(limitSwitchesX) == LOW) //iterate doStep signal for as long as eitherlimit switch remains pressed 

  {  
    digitalWrite(motorXstep, LOW); //this LOW to HIGH change is what creates the
    digitalWrite(motorXstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
    delayMicroseconds(stepSpeed); //delay time between steps, too fast and motor stalls
  }

  digitalToggle(motorXdir); //reset motor back to original direction once limit switch is no longer pressed
  lcd.clear();
}


void retreatY(){ //moves platform back into safe zone if a limit switch on the Y axis is tripped during operation

  digitalToggle(motorYdir); //reverse motor direction to move away from limitswitch

  lcd.setCursor(0, 0);
  lcd.print("End of travel!  ");
  lcd.setCursor(0, 1);
  lcd.print("Reversing...    ");

  while (digitalRead(limitSwitchesY) == LOW) //iterate doStep signal for as long as eitherlimit switch remains pressed 

  {  
    digitalWrite(motorYstep, LOW); //this LOW to HIGH change is what creates the
    digitalWrite(motorYstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
    delayMicroseconds(stepSpeed); //delay time between steps, too fast and motor stalls
  }

  digitalToggle(motorYdir); //reset motor back to original direction once limit switch is no longer pressed
  lcd.clear();
}

void manualControl(){

  //X axis manual joystick control
  if(digitalRead(limitSwitchesX) == HIGH){

    joyStickreadingX = analogRead(joyStickX); //get current position of joystick

    int j = joyStickreadingX - 516;
    j = abs(j);
    manualSpeedX = 70000/j;  


    if (joyStickreadingX >= 625){
      digitalWrite(motorXdir, LOW);  //go forwards
      digitalWrite(motorXstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorXstep,LOW);
      delayMicroseconds(manualSpeedX);

    }
    if (joyStickreadingX <= 585){
      digitalWrite(motorXdir, HIGH);  //go backwards
      digitalWrite(motorXstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorXstep,LOW);
      delayMicroseconds(manualSpeedX);
    }

  }

  else {
    retreatX();
  }

  //Y axis manual joystick control
  if(digitalRead(limitSwitchesY) == HIGH){
    joyStickreadingY = analogRead(joyStickY); //get current position of joystick

    int k = joyStickreadingY - 516;
    k = abs(k);
    manualSpeedY = 70000/k;  


    if (joyStickreadingY >= 625){
      digitalWrite(motorYdir, LOW);  //go forwards
      digitalWrite(motorYstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorYstep,LOW);
      delayMicroseconds(manualSpeedY);
    }

    if (joyStickreadingY <= 585){
      digitalWrite(motorYdir, HIGH);  //go backwards
      digitalWrite(motorYstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorYstep,LOW);
      delayMicroseconds(manualSpeedY);
    }
  }

  else{
    retreatY();
  }
}















