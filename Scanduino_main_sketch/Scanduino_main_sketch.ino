//SCANDUINO MAIN SKETCH

//the ultimate goal of Scanduino is to automate the digitizing of a sheet of film by capturing 
//an array of images taken by a DSLR for later stitching via panoramic stitching program

//bodged together by Richard Iles - ReallySmall on Flickr http://www.flickr.com/photos/reallysmall/
//analogue joystick code from http://www.instructables.com/id/CRANE-GAME/step2/Control-a-stepper-with-joy-stick/

//reference required libraries
#include <DigitalToggle.h>
#include <LiquidCrystal.h> 
#include <Wire.h>
//#include <Adafruit_MCP23017.h> commented out pending replacement with LiquidTWI2
//#include <Adafruit_RGBLCDShield.h>  commented out pending replacement with LiquidTWI2
#include <LiquidTWI2.h>

LiquidTWI2 lcd(0);


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
int menuFormatItem = 1; //default film format to be scanned
int settlingDelay = 1000; //time to pause in millis to allow vibrations to cease before taking photo
int cameraDelay = 2000; //time to pause in millis to allow camera to take photo

//assign analogue pins
int joyStickX = A2; //analogue joystick for manual positioning
int joyStickY = A3; //analogue joystick for manual positioning
// The lcd shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5
//Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

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

int lastPress;

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

//menuSectionState pushButton toggle - button used is on the lcd shield which does its won debouncing
volatile int menuSectionState = HIGH; //the current state

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

      manualControl(); //function translates analogue joystick feedback into motor movement

    } //end of manual control

      else{ //enable lcd menu

      lcd.setCursor(0, 0);
      lcd.print("FILM FORMAT < > ");

      uint8_t buttons = lcd.readButtons();

      if (buttons & BUTTON_RIGHT) { //if right button pushed go forward one menu item
      if(millis() - lastPress > 800) {
        menuFormatItem++;
        lastPress = millis();
      }
      }  

      if (buttons & BUTTON_LEFT) { //if left button pushed go back one menu item
        if(millis() - lastPress > 800) {
        menuFormatItem--;
        lastPress = millis();
      }
      }   

      menuFormatItem = constrain(menuFormatItem, 0, 6); //limits choice to specified range

      if (menuFormatItem == 6){ //when counter value exceeds number of menu items
        menuFormatItem = 1; //reset it to 1 again to create a looping navigation
      }

      if (menuFormatItem == 0){ //when counter value goes below minimum number of menu items
        menuFormatItem = 5; //reset it to 5 again to create a looping navigation
      }  

      switch (menuFormatItem) { //selects which film format to scan

      case 1: //4 x 5

        lcd.setCursor(0, 1);
        lcd.print("4 x 5           ");

        distanceX = 20;
        distanceY = 20; 
        numberOfImagesX = 7; 
        numberOfImagesY = 5;
        startPosX = 100;
        startPosY = 100;

        break;

      case 2: //6 x 9

        lcd.setCursor(0, 1);
        lcd.print("6 x 9           ");

        distanceX = 20;
        distanceY = 20; 
        numberOfImagesX = 7; 
        numberOfImagesY = 5;
        startPosX = 100;
        startPosY = 100;

        break;

      case 3: //6 x 7

        lcd.setCursor(0, 1);
        lcd.print("6 x 7           ");

        distanceX = 20;
        distanceY = 20; 
        numberOfImagesX = 7; 
        numberOfImagesY = 5;
        startPosX = 100;
        startPosY = 100;

        break;

      case 4: //6 x 6

        lcd.setCursor(0, 1);
        lcd.print("6 x 6           ");

        distanceX = 20;
        distanceY = 20; 
        numberOfImagesX = 7; 
        numberOfImagesY = 5;
        startPosX = 100;
        startPosY = 100;

        break;

      case 5: //35mm

        lcd.setCursor(0, 1);
        lcd.print("35mm            ");

        distanceX = 20;
        distanceY = 20; 
        numberOfImagesX = 7; 
        numberOfImagesY = 5;
        startPosX = 100;
        startPosY = 100;

        break;

      }

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
