arrowprint
==========

Node.js &amp; Arduino DUE powered 3D printer. 

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

5X Big Easy Driver https://www.sparkfun.com/products/10735

5X Stepper Motor with Cable https://www.sparkfun.com/products/9238

Frame:

Documentation https://github.com/josefprusa/PrusaMendel/blob/master/docs/documentation.pdf

Frame Printable parts STL: https://github.com/josefprusa/PrusaMendel/tree/master/metric-prusa-lm8uu

Nozzle: http://www.reprap.org/wiki/LulzBot/Budaschnozzle

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
var file = "printer_bracket2.gcode";
```

edit nodejs program to read correct gcode file. (best to use Slic3r in my experience to generate this from STL/OBJ).




```
node arrowprint.js
```

![Arrow Print image](/arrowprint.jpg "Arrow print image")
