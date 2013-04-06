//SCANDUINO VARIABLE CALIBRATION SKETCH

//used to quickly experiment with variable values to calibrate to individual hardware
//use values obtained to populate film format presets menu in Scanduino_main_sketch.ino

//analogue joystick code from http://www.instructables.com/id/CRANE-GAME/step2/Control-a-stepper-with-joy-stick/

//reference required libraries
#include <DigitalToggle.h>
#include <LiquidCrystal.h> 
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

//global variables
int stepCountX = 0; //number of moves along X axis
int stepCountY = 0; //number of moves along Y axis
int distanceX = 40; //how far to move along X axis
int distanceY = 40; //how far to move along Y axis
int numberOfImagesX = 5; //how many images to take per row
int numberOfImagesY = 3; //how many rows of images to take
int stepSpeedX = 8000; //delay in microseconds between motor steps for X axis
int stepSpeedY = 8000; //delay in microseconds between motor steps for Y axis
int startPosX = 200; //distance to move away from limitswitches on X axis, dependant on film format
int startPosY = 200; //distance to move away from limitswitches on X axis, dependant on film format
int manualSpeedX = 0; //delay between steps in manual mode on X axis, governing motor speed
int manualSpeedY = 0; //delay between steps in manual mode on Y axis, governing motor speed
int joyStickreadingX = 0; //current analogue reading of X axis on joystick
int joyStickreadingY = 0; //current analogue reading of Y axis on joystick
int menuItem = 1; //which menu item to display when turning rotary encoder
int settlingDelay = 1000; //time to pause in millis to allow vibrations to cease before taking photo
int cameraDelay = 2000; //time to pause in millis to allow camera to take photo

//assign analogue pins
int joyStickX = A2; //analogue joystick for manual positioning
int joyStickY = A3; //analogue joystick for manual positioning
// The lcd shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//assign digital pins
int pushButton = 2;  // Pin 2 = Start/ Stop button
int manualControlButton = 3; //select/ unselect menu item button
int motorXdir = 4; //X stepper motor direction
int motorXstep = 5; //send step signal to X stepper motor
int motorYdir = 6; //Y stepper motor direction
int motorYstep = 7; //send step signal to Y stepper motor
int limitSwitchesX = 8; //limit switches to stop stepping if end of travel on rail is reached at either end
int limitSwitchesY = 9; //limit switches to stop stepping if end of travel on rail is reached at either end
int focus = 10; //send an autofocus signal to the camera
int shutter = 11; //send a shutter signal to the camera
int disableMotorX = 12; //send a signal to Easydriver to disable motor
int disableMotorY = 13; //send a signal to Easydriver to disable motor

//pushButton toggle
volatile int buttonState = HIGH; // the current state of the output pin
volatile int reading; // the current reading from the input pin
volatile int previous = LOW; // the previous reading from the input pin
volatile long time = 0; // the last time the output pin was toggled
volatile long debounce = 400; // the debounce time, increase if the output flickers

//manualControl pushButton toggle
volatile int mcbuttonState = HIGH; //the current state of the output pin
volatile int mcreading; //the current reading from the input pin
volatile int mcprevious = LOW; //the previous reading from the input pin
volatile long mctime = 0; //the last time the output pin was toggled
volatile long mcdebounce = 400; //the debounce time, increase if the output flickers

void setup(){

  Serial.begin(9600); 
  lcd.begin(16, 2);

  attachInterrupt(0, buttonChange, CHANGE); // Button on interrupt 0 - pin 2
  attachInterrupt(1, manualControlButtonChange, CHANGE); // Button on interrupt 1 - pin 3

  pinMode(joyStickX, INPUT); //define pin as an input
  pinMode(joyStickY, INPUT); //define pin as an input
  pinMode(pushButton, INPUT); //define pin as an input
  pinMode(manualControlButton, INPUT); //define pin as an input
  pinMode(limitSwitchesX, INPUT); //define pin as an input
  pinMode(limitSwitchesY, INPUT); //define pin as an input
  pinMode(motorXdir, OUTPUT); //define pin as an output
  pinMode(motorXstep, OUTPUT); //define pin as an output 
  pinMode(motorYdir, OUTPUT); //define pin as an output 
  pinMode(motorYstep, OUTPUT); //define pin as an output
  pinMode(focus, OUTPUT); //define pin as an output 
  pinMode(shutter, OUTPUT); //define pin as an output
  pinMode(disableMotorX, OUTPUT); //define pin as an output 
  pinMode(disableMotorY, OUTPUT); //define pin as an output 

  digitalWrite(motorXstep, LOW); //start with motor step pin low
  digitalWrite(motorYstep, LOW); //start with motor step pin low
  digitalWrite(motorXdir, LOW); //start with motor in default fwd direction
  digitalWrite(motorYdir, LOW); //start with motor in default fwd direction
  digitalWrite(focus, LOW); //start with focus pin low
  digitalWrite(shutter, LOW); //start with shutter pin low
  digitalWrite(pushButton, HIGH); //start with pushButton pin high
  digitalWrite(manualControlButton, HIGH); //start with manualControlButton pin high
  digitalWrite(limitSwitchesX, HIGH); //start with limitSwitchesX pin high
  digitalWrite(limitSwitchesY, HIGH);//start with limitSwitchesY pin high
  digitalWrite(disableMotorX, LOW);//start with disableMotorX pin low
  digitalWrite(disableMotorY, LOW);//start with disableMotorY pin low

  lcd.setCursor(0, 0);
  lcd.print("Scanduino");

}

void loop(){

  if (buttonState == HIGH){ //configuration section provides manual control and configures settings using a simple lcd menu system

    manualControlButtonChange(); //check current toggle status of the manual control button

    if (mcbuttonState == HIGH){ //enable manual control using joystick

      manualControl(); //function translates analogue joystick feedback into motot movement

    } //end of manual control

      else{ //enable lcd menu

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

      switch (menuItem) { //displays menu item defined by current value of menuItem

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

      } //end of menu items switch case statement

    } //end of lcd menu

  } //end of configuration section

  else{ //this section runs the scan using the settings chosen in the previous section

    lcd.clear();
    lcd.print("Finding start");

    findStart(); //move to the home position

    lcd.clear();
    lcd.print("Scan started");

    for (int i = 0; i < (numberOfImagesY - 1); i++){ //Repeat until count equals numberOfImagesY - 1

      for (int i = 0; i < (numberOfImagesX - 1); i++){ //Repeat the function until count equals numberOfImagesX - 1

        lcd.clear();
        lcd.print("Taking picture");
        takeImage(); //take picture
        lcd.clear();
        lcd.print("Moving");

        int i = 0; //used to count steps made
        while (i < distanceX && digitalRead(limitSwitchesX) == HIGH){ // Move as far as distanceX
          doXStep();
          i++;
          if (digitalRead(limitSwitchesX) == LOW){ //stop motor and reverse if limit switch hit
            retreatX();
            break;
          }
        }
        i = 0; //reset counter
      } //end of looping row function

      //Last image of row
      lcd.clear();
      lcd.print("Final pic of row");
      takeImage(); //take picture
      digitalToggle(motorXdir); //reverse x axis motor direction for next row

      //Move down to next row
      lcd.clear();
      lcd.print("Next row");
      delay(500);
      int j = 0; //used to count steps made
      while (j < distanceY && digitalRead(limitSwitchesY) == HIGH){ // Move as far as distanceY - moves down to the next row
        doYStep();
        j++;
        if (digitalRead(limitSwitchesY) == LOW){ //stop motor and reverse if limit switch hit
          retreatY();
          break;
        } //end of conditional limit switch statement
      } //end of motor movement down to next row
      j = 0; //reset counter
    } //end of main looping function

    //Final row runs once outside of main looping function
    lcd.clear();
    lcd.print("Final row");

    for (int i = 0; i < (numberOfImagesX - 1); i++){ //Repeat the function until count equals numberOfImagesX - 1

      lcd.clear();
      lcd.print("Taking picture");
      takeImage(); //take picture
      int i = 0; //used to count steps made
      while (i < distanceX && digitalRead(limitSwitchesX) == HIGH){ // Move as far as distanceX
        doXStep();
        i++;
        if (digitalRead(limitSwitchesX) == LOW){ //stop motor and reverse if limit switch hit
          retreatX();
          break;
        }
      }
      i = 0; //reset counter
    }

    lcd.clear();
    lcd.print("Final picture");
    takeImage(); //take picture

      //Return to start position
    lcd.clear();
    lcd.print("Return to start");
    findStart();//move to default start point 
    lcd.clear();
    lcd.print("finished");
    delay(2000);
    lcd.clear();
    buttonState = HIGH; // Return to menu options

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

void findStart(){ //uses limitswitch feedback to reset carriage to a preset start position

  //first reset the x axis

  while (digitalRead(limitSwitchesX) == HIGH){ //keep reversing until the limitswitch is pressed
    digitalWrite(motorXdir, HIGH); //reverse stepper direction
    doXStep();
  }

  int i=0; //used to count steps made to move to startPosX

  while (digitalRead(limitSwitchesX) == LOW || i < startPosX){ //iterate doStep signal until start position reached
    digitalWrite(motorXdir, LOW); //reset stepper direction
    doXStep();
    i++;
  }

  i=0; //reset counter

  //then reset the y axis

  while (digitalRead(limitSwitchesY) == HIGH){ //keep reversing until the limitswitch is pressed
    digitalWrite(motorYdir, HIGH); //reverse stepper direction
    doYStep();
  }

  int j=0; //used to count steps made to move to startPosY

  while (digitalRead(limitSwitchesY) == LOW || j < startPosY){ //iterate doStep signal until start position reached
    digitalWrite(motorYdir, LOW); //reset stepper direction
    doYStep();
    j++;
  }

  j=0; //reset counter

}

void retreatX(){ //moves platform back into safe zone if a limit switch on the X axis is tripped during operation

  digitalToggle(motorXdir); //reverse motor direction to move away from limitswitch

  lcd.setCursor(0, 0);
  lcd.print("End of travel!  ");
  lcd.setCursor(0, 1);
  lcd.print("Reversing...    ");

  while (digitalRead(limitSwitchesX) == LOW){ //iterate doXStep signal for as long as either X axis limit switch remains hit   
    doXStep();
  }

  digitalToggle(motorXdir); //reset motor back to original direction once limit switch is no longer pressed
  lcd.clear();
}

void retreatY(){ //moves platform back into safe zone if a limit switch on the Y axis is hit during operation

  digitalToggle(motorYdir); //reverse motor direction to move away from limit switch

  lcd.setCursor(0, 0);
  lcd.print("End of travel!  ");
  lcd.setCursor(0, 1);
  lcd.print("Reversing...    ");

  while (digitalRead(limitSwitchesY) == LOW){ //iterate doYStep signal for as long as either Y axis limit switch remains hit  
    doYStep();
  }

  digitalToggle(motorYdir); //reset motor back to original direction once limit switch is no longer pressed
  lcd.clear();
}

void manualControl(){

  //X axis manual joystick control
  if(digitalRead(limitSwitchesX) == HIGH){ //if limit switch not hit

    joyStickreadingX = analogRead(joyStickX); //get current position of joystick

    int j = joyStickreadingX - 516;
    j = abs(j);
    manualSpeedX = 700000/j; //adjust this value to change base speed of manual movement on X axis   


    if (joyStickreadingX >= 625){ //go forwards
      digitalWrite(motorXdir, LOW);  
      digitalWrite(motorXstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorXstep,LOW);
      delayMicroseconds(manualSpeedX);

    }
    if (joyStickreadingX <= 585){ //go backwards
      digitalWrite(motorXdir, HIGH);  
      digitalWrite(motorXstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorXstep,LOW);
      delayMicroseconds(manualSpeedX);
    }

  }

  else {
    retreatX(); //move back from limit switch if hit
  }

  //Y axis manual joystick control
  if(digitalRead(limitSwitchesY) == HIGH){ //if limit switch not hit
    
    joyStickreadingY = analogRead(joyStickY); //get current position of joystick

    int k = joyStickreadingY - 516;
    k = abs(k);
    manualSpeedY = 700000/k; //adjust this value to change base speed of manual movement on Y axis  


    if (joyStickreadingY >= 625){ //go forwards
      digitalWrite(motorYdir, LOW);  
      digitalWrite(motorYstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorYstep,LOW);
      delayMicroseconds(manualSpeedY);
    }

    if (joyStickreadingY <= 585){ //go backwards
      digitalWrite(motorYdir, HIGH);  
      digitalWrite(motorYstep,HIGH);
      delayMicroseconds(2);
      digitalWrite(motorYstep,LOW);
      delayMicroseconds(manualSpeedY);
    }
  }

  else{
    retreatY(); //move back from limit switch if hit
  }
}

void takeImage(){ //disable motors, pause, then take a picture
  digitalWrite(disableMotorX, HIGH); //disable motor to avoid vibration
  digitalWrite(disableMotorY, HIGH); //disable motor to avoid vibration
  delay(settlingDelay); //pause to allow any vibrations to fade out before taking picture
  digitalWrite(focus, HIGH); // Trigger camera autofocus - camera may not take picture in some modes if this is not triggered first
  digitalWrite(shutter, HIGH); // Trigger camera shutter
  delay(200); // Small delay needed for camera to process above signals
  digitalWrite(shutter, LOW); // Switch off camera trigger signal
  digitalWrite(focus, LOW); // Switch off camera focus signal
  delay(cameraDelay); //Pause to allow for camera to take picture
  digitalWrite(disableMotorX, LOW); //re-enable motor
  digitalWrite(disableMotorY, LOW); //re-enable motor
}

void doXStep(){ //send a step signal to the X axis motor
  digitalWrite(motorXstep, LOW); //this LOW to HIGH change is what creates the
  digitalWrite(motorXstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
  delayMicroseconds(stepSpeedX); //delay time between steps, too fast and motor stalls 
}

void doYStep(){ //send a step signal to the Y axis motor
  digitalWrite(motorYstep, LOW); //this LOW to HIGH change is what creates the
  digitalWrite(motorYstep, HIGH); //"Rising Edge" so the easydriver knows to when to step
  delayMicroseconds(stepSpeedY); //delay time between steps, too fast and motor stalls 
}

