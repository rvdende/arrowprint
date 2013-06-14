![Imgur](http://i.imgur.com/rQf4Q2T.jpg)
![Imgur](http://i.imgur.com/30PxcT5.jpg)
![Imgur](http://i.imgur.com/aii8c4w.jpg)
![Imgur](http://i.imgur.com/O18fwEC.jpg)

VIDEOS: [video1](http://vimeo.com/60076876) [video2](http://vimeo.com/61339150)

arrowprint
==========

Node.js &amp; Arduino DUE powered 3D printer. 

The following is for those of you who would enjoy the process of building your own 3D printer.

It really depends on what tools and skills you have at your disposal. This might include a spare computer, some old electronic bits you can salvage. You might have a fat wallet and willing to fork out for some really good parts, or you'd like to build a tiny desktop machine from old floppy drives. I'm going to try to cover the commonalities, but first you need to understand what we're trying to do here.

We need to build a machine that can move, and move how we want it to move. So the basis is something like a cnc, plotter/printer robot. So you'll learn how to not only build a 3D printer, but just about any kind of robot. Which is no doubt a useful thing to know.

In the case of a 3D printer. It needs to squeeze a strand of plastic down into a hotend or nozzle. It is a little melting pot with a 0.35 to 0.5mm diameter hole at the bottom. It puts the molten plastic where we want it to go. 

![Imgur](http://i.imgur.com/aii8c4w.jpg)![Imgur](http://i.imgur.com/O18fwEC.jpg)

So first things first. We need movement.
========================================

You'll need:

* arduino (any will do for now, the [UNO](http://robotics.org.za/index.php?route=product/product&path=47&product_id=68) is nice to begin with since it is robust, but later on you might want the [DUE](http://robotics.org.za/index.php?route=product/product&path=47&product_id=604)
*	stepper motor(s) - NEMA17 or niice [NEMA23](http://robotics.org.za/index.php?route=product/product&path=241&product_id=238)
*	power source for motor(s), in our case we use a [48V 10A powersupply](http://robotics.org.za/index.php?route=product/product&path=55_205&product_id=766). You can also use a PSU (pc power supply with 12V)
*	stepper controller which takes in signals from the arduino and pushes power to your motor. You could go small [1Amp](http://robotics.org.za/index.php?route=product/product&keyword=controller&category_id=0&product_id=712) or medium [2Amp](http://robotics.org.za/index.php?route=product/product&keyword=controller&category_id=0&product_id=641) or best [industrial](http://robotics.org.za/index.php?route=product/product&keyword=controller&product_id=432) 

So in general you have two signals you need to understand to move your motor accurately

*	Direction. If you want to move in one direction you keep this pin LOW (gnd/0 Volt) and the opposite direction you set it HIGH (5V or 3.3V)
*	Step. This is pulsed. You'll see your stepper will have a set number of steps per rotation, most are 200 or 400 steps but yours may vary. So to do a full rotation you would have to toggle between HIGH and LOW 400 times.

Give it a try, experiment with different delays between STEP pulses to control the speed of your motor. If you try too fast it will just humm or buzz and wont be able to turn over. Too slow and your machine will take ages to do anything. Eventually you'll want to be able to accelerate/decelerate to optimize both speed and power.

Once you've got the hang of this you'll need more motors and some kind of frame for them to be useful. You can ask a friend with a 3D printer or CNC to make parts for your machine. Common parts include threaded rod with nuts and washers. Silversteel (smooth rod) and linear bearings or brass bushings. Timing belts and pulleys. High wattage resistors for heating. Variable resistence temperature sensors. Mosfets, transistors and relays for switching things on and off. 

Okay, so by now you should have your motors working nicely, but you might sit with the problem of keeping them busy with usefull tasks. For this you need to be able to send commands to your robot from a file, interface or computer. You could use a memory card reader and read line by line. You could attach a display and some control buttons and rotary encoders/pots so you can use the robot manually in real time. In my case I decided to control the robot over usb/serial connection. 

With the help of JSON we can combine many commands at the same time, send it over the text based serial connection and split them out again at the other end. So far the best baud rate I found is 115200bps. For this I used nodejs, but I guess you could also use processing, C or anything that supports serial communication. It looks something like this:

{cmd: "G1", x: 12.345, y: -45.678, z: 1.246}

{cmd: "G1", x: 14.650, y: -41.123, z: 2.000}

{cmd: "G..... }

The cmd G1 thing is just something of a relic from g-code. A standard people use with CNC machines. They have different codes for different functionality, where G1 just means move here in a straight line. So in this example it will tell the machine to move to the first set of coordinates, and once it is done it will execute the second line, which is also a straight line to the second set of coordinates.

Drawing a line
==============

So.. you might run into a little trickyness with moving in a straight line using 2 or more motors/axis. What you'll most likely find is that your motors run at full speed, and one of them reaches the distance on its axis before the others reach theirs, so you need to make them reach the destination at the same time, or in other words, the motor that needs to move a shorter distance needs to move slow on purpose to allow the other motor to keep up.

With the help of pythagoras we calculate little movements instead of one big movement. See the line function in the arduino firmware source.

Features
========

Working! See video: https://vimeo.com/61339150
Feed data from nodejs over serial connection to the arduino. Realtime control.
Simple code base.

Todo
====

Web GUI/Remote printing.
Temperature control for safety and easier use.
Document physical build. Below is overview of how to build this printer.

Build
=====

Electronics:

1X Arduino DUE https://www.sparkfun.com/products/11589
You can also use an UNO, but you should run with less steps otherwise it will get slow.

4X Big Easy Driver https://www.sparkfun.com/products/10735
We run both Z (vertical) motors from a single stepper controller driver.

5X Stepper Motor with Cable https://www.sparkfun.com/products/9238

Frame:

Documentation https://github.com/josefprusa/PrusaMendel/blob/master/docs/documentation.pdf

Frame Printable parts STL: https://github.com/josefprusa/PrusaMendel/tree/master/metric-prusa-lm8uu

Nozzle: http://www.reprap.org/wiki/LulzBot/Budaschnozzle

We can also custom machine parts for your build. Just ask.

Step 1 Arduino side of things
=============================

Arduino Toolkit Download: http://arduino.cc/en/Main/Software
Use arduino toolkit version 1.5.2 or later for DUE. 
Use arduino toolkit version 1.0.3 or later for Leonardo (not recommended as it is slow, but works). 
 
Extract aJson library from https://github.com/interactive-matter/aJson to your ~/sketchbook/libraries

Edit arrowprint_firmware/arrowprint_firmware.ino and set up all your pin outs to your motors, zero switches and physical limitations in distances/steps.

Upload firmware.

Step 2 Nodejs gcode sender
==========================

Make sure you have node serialport working. 

```
npm install serialport
```

Edit arrowprint.js and change file to reference your gcode you want to print. To generate this gcode use Slic3r http://slic3r.org/ to convert .STL to .gcode
Use Blender to model/import/view .STL files http://www.blender.org/

```
var file = "wade_big.gcode";
```

edit nodejs program to read correct gcode file. (best to use Slic3r in my experience to generate this from STL/OBJ).




```
node arrowprint.js
```

![Arrow Print image](/arrowprint.jpg "Arrow print image")
