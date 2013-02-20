arrowprint
==========

Simple, elegant and as clean as possible Node.js &amp; Arduino powered 3D printer. 

![Arrow Print image](/arrowprint.jpg "Arrow print image")

Step 1 Arduino side of things
=============================

Use arduino toolkit version 1.5.2 or later for DUE
Use arduino toolkit version 1.0.3 or later for Leonardo (not recommended as it is slow, but works)
 
Install aJSON library from http://www.open.com.au/mikem/arduino/AccelStepper/

See http://bildr.org/2012/11/big-easy-driver-arduino/ for stepper controllers with this library.
 
Install aJson library from https://github.com/interactive-matter/aJson

Edit the firmware and make sure your pins are set correctly
Upload arduino firmware and zero axis, printer now waits for gcode over serial.

Step 2 Nodejs gcode sender
==========================

Make sure you have node serialport working. 

```
npm install serialport
```

edit nodejs program to read correct gcode file. (best to use Slic3r in my experience to generate this from STL/OBJ).

```
node arrowprint.js
```

Boom. It should now be sending and printing.
