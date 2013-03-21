arrowprint
==========

Node.js &amp; Arduino DUE powered 3D printer. 

![Arrow Print image](/arrowprint.jpg "Arrow print image")

Step 1 Arduino side of things
=============================

Use arduino toolkit version 1.5.2 or later for DUE
Use arduino toolkit version 1.0.3 or later for Leonardo (not recommended as it is slow, but works)
 
Install aJson library from https://github.com/interactive-matter/aJson

Upload firmware.

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
