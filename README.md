Scanduino-by-ReallySmall
========================

Scanduino -  an Arduino sketch to control a DSLR based film scanner

Flickr member Really Small has created a program called Stackduino, see: 

http://www.flickr.com/photos/reallysmall/8270022807/in/photostream/ 

Stackduino's relevance to the plans of Scanduino was immediately recognized. 
ReallySmall is very generously offering his expert attention to adapting 
Stackduino to Scanduino : ) From all of us, THANK YOU! 

The ultimate goal of Scanduino is to automate the digitizing of a sheet of film
by capturing an array of images taken by a DSLR for later stitching 
via panoramic stitching program.

Scanduino development discussion at Flickr:
http://www.flickr.com/photos/35334802@N04/8472236472/in/photostream/
--------------------------------------------

Overview of the various sketches at this point:

The Joystick sketch is meant to allow calibrating different joysticks. It will output the resting X and Y values,
with which one can edit the working sketch. This helps in two ways; first, if your motors begin to move upon powering 
up and before the actual program runs this means your values must be changed before proceeding to reflect your joystick's
actual resting state. Second, setting the upper and lower limits at approx. 20 points on either side of the resting value
will make the joystick behave better.

The Variale sketch is meant to be used to establish settings to use in the main sketch. You can use it to find the best
values for each film size you plan to 'scan' and input those figures into the Main sketch, whereupon you will find those
settings simplified in the form of selectable presets.

The Main sketch, as noted, requires all your values to be established before proceeding.

Please note that ALL SKETCHES!!! require limit switches be in place to prevent mishaps.
