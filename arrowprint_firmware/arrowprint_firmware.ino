/*
 * Arrow 3D Printer Firmware
 * by Rouan van der Ende - fluentart.com - code meets art
 * rouan@8bo.org - 8bitoctopus - creative studio
 * 
 * Use arduino toolkit version 1.5.2 or later
 * Make sure you use an Arduino DUE (Or Leonardo)
 * Install aJSON library
 * Install AccelStepper library
 */
#include <aJSON.h>

#include <AccelStepper.h>

String version = "2.4";



/* 
 *  =======================================================================================
 *    CONFIGURATION
 *  =======================================================================================
 */

//physical movement scale (bigger == more);
double tablexsteps    = 13000;     
double tableysteps    = 13000;
double tablezsteps    = 210000;    //257142 (100mm) from top (STEPS FROM TABLE SURFACE TO TOP) 0,0,0 is FAR LEFT BOTTOM
double tableesteps  = -3600;

//measured (1000 = 1mm)
double tablexdistance = 250000;    //in microns (158mm )
double tableydistance = 250000;    //in microns (200mm)
double tablezdistance = 96000;    //in microns (96mm)
double tableedistance = 1000;    //in microns (96mm)

double maxmotorSpeed = 12600;
double motorSpeed = 9600;                                           //default 9600
double motorAccel = 80000; //steps/second/second to accelerate      //default 80000

// Motor pins
int motorpinXstep = 7;
int motorpinXdir  = 6;
int motorpinYstep = 9;
int motorpinYdir  = 8;
int motorpinZstep = 13;
int motorpinZdir  = 12;
int motorpinEstep = 11;
int motorpinEdir  = 10;

// Motor Zero switch pins
int xZeropin = 5;        //INPUT pin
int yZeropin = 4;        //INPUT pin
int zZeropin = 3;        //INPUT pin

int xZeroNC  =  1;        //Normally closed/open
int yZeroNC  =  1;          //Normally closed/open
int zZeroNC  =  0;          //Normally closed/open

// =========================================================================================

//latest position in microns (1000 = 1mm)
double lastx = tablexdistance/2;
double lasty = tableydistance/2;
double lastz = 0;
double laste = 0;




// =========================================================================================

AccelStepper stepper5(1, motorpinXstep, motorpinXdir);          //CROSSBEAM      X AXIS
AccelStepper stepper4(1, motorpinYstep, motorpinYdir);          //TABLE IN/OUT   Y AXIS
AccelStepper stepper1(1, motorpinZstep, motorpinZdir);          //UP DOWN        Z AXIS
AccelStepper stepper2(1, motorpinEstep, motorpinEdir);          //EXSTRUDER FEED E AXIS

void setup() {
  Serial.begin(115200);
  Serial.print("{ \"log\" : \"Connecting...\"}");
  //ZERO SWITCHES
  pinMode(xZeropin, INPUT);                  //X SWITCH
  pinMode(yZeropin, INPUT);                  //Y SWITCH
  pinMode(zZeropin, INPUT);                  //Z SWITCH


  stepper1.setMaxSpeed(maxmotorSpeed);
  stepper2.setMaxSpeed(maxmotorSpeed);                         
  stepper4.setMaxSpeed(maxmotorSpeed);                         
  stepper5.setMaxSpeed(maxmotorSpeed);      

  zeroaxis();                                //ZERO AXIS
  
  stepper1.setSpeed(motorSpeed);
  stepper2.setSpeed(motorSpeed);
  stepper4.setSpeed(motorSpeed);
  stepper5.setSpeed(motorSpeed);
  stepper1.setAcceleration(motorAccel);  
  stepper2.setAcceleration(motorAccel);  
  stepper4.setAcceleration(motorAccel);  
  stepper5.setAcceleration(motorAccel);    
  
  //testaxis();                                //(optional) RUNS TO EXTREMES AND STOPS AT (X0,Y0,Z0,E0)
  Serial.print("{ \"log\" : \"Success\"}");
  Serial.print("{ \"version\" : \""+version+"\"}");  
  //rapid(75000,100000, 10000, 0);

  line(tablexdistance/2, tableydistance/2, 0, 0);  //CENTER

}

aJsonStream serial_stream(&Serial);

void loop() {
  //line(10000,10000,-20000,0);
  //line(150000,127052,-30000,20000);
  //gotoxyze(0,0,0,0);
  //testsurface();  //(optional) this runs around on the table so you can test height consistancy
  
  //gotoxyze(150000,100000,0,0);  
  //circle(50000,50000,0,0,20000);
  //
  //
      if (serial_stream.available()) {
        /* First, skip any accidental whitespace like newlines. */
        serial_stream.skip();
      }
    
      if (serial_stream.available()) {
        /* Something real on input, let's take a look. */
        aJsonObject *msg = aJson.parse(&serial_stream);
        processMessage(msg);
        aJson.deleteItem(msg);        
      }   
  Serial.print("{ \"status\" : \"waiting\"}");  
}

//=======================================================================================
// START ZEROAXIS()
void zeroaxis() {
  /*
      Runs each motor towards the zero switch and resets its position. Must be run at start.
  */

///////////////////////////////////////////////////////ZERO X OVERHANG SIDE TO SIDE     
    
    stepper5.setSpeed(6400);
    stepper5.setAcceleration(999999);          
    
    while (digitalRead(xZeropin) != xZeroNC) {   
      stepper5.moveTo(stepper5.currentPosition()-10);      //change to +4 for zero on opposite side      
      stepper5.run();      
    }
    
    stepper5.stop();
    stepper5.setCurrentPosition(-1000);
    stepper5.setSpeed(motorSpeed);   
    stepper5.setAcceleration(motorAccel); 
    stepper5.moveTo(7000);
    while (stepper5.distanceToGo() != 0) {
      stepper5.run(); 
    }    

  
    ///////////////////////////////////////////////////////ZERO Y TABLE
    stepper4.setSpeed(6400);
    stepper4.setAcceleration(999999);       
    while (digitalRead(yZeropin) != yZeroNC) {   
      stepper4.moveTo(stepper4.currentPosition()+10);            
      stepper4.run();      
    }
    delay(100);
    stepper4.stop();
    stepper4.setCurrentPosition(tableysteps+1000);
    stepper4.moveTo(0);
    while (stepper4.distanceToGo() != 0) {
      stepper4.run(); 
    }
    ////////////////////////////////////////////////////////ZERO Z - UPDOWN
    stepper1.setSpeed(8000);
    stepper1.setAcceleration(999999);          

    while (digitalRead(zZeropin) != zZeroNC) {   
      stepper1.moveTo(stepper1.currentPosition()+320);          
      stepper1.runToPosition();
    }
    stepper1.stop();
    stepper1.setCurrentPosition(tablezsteps);
    stepper1.setSpeed(motorSpeed);   
    stepper1.setAcceleration(motorAccel);     
    stepper1.moveTo(0);
    while (stepper1.distanceToGo() != 0) {
      stepper1.run(); 
    }    

    //////////////////////////////////////////////////////////E zero - feeder    
    stepper2.setSpeed(6400);
    stepper2.setAcceleration(999999);
    stepper2.moveTo(0);
    while (stepper2.distanceToGo() != 0) {
      stepper2.runSpeedToPosition(); 
    }
  
}
// END ZEROAXIS()

// ==============================================================================================================================================================================
// START TESTAXIS()
void testaxis() {
  /* 
    Runs all axis to extremes and also gives extruder a test spin 
  */
    //HIGH
    stepper5.moveTo(tablexsteps);
    stepper4.moveTo(tableysteps);    
    stepper1.moveTo(tablezsteps);
    stepper2.moveTo(tableesteps); 
    while ((stepper5.distanceToGo() != 0) || (stepper4.distanceToGo() != 0) || (stepper1.distanceToGo() != 0) || (stepper2.distanceToGo() != 0)) {
      stepper5.run();
      stepper4.run();
      stepper1.run();
      stepper2.run();
    }  
    //ZERO
    stepper5.moveTo(0);
    stepper4.moveTo(0);    
    stepper1.moveTo(0);
    stepper2.moveTo(0); 
    while ((stepper5.distanceToGo() != 0) || (stepper4.distanceToGo() != 0) || (stepper1.distanceToGo() != 0) || (stepper2.distanceToGo() != 0)) {
      stepper5.run();
      stepper4.run();
      stepper1.run();
      stepper2.run();
    }      
}
// END TESTAXIS();

// ==============================================================================================================================================================================

// START RAPID()
void rapid(int x, int y, int z, int e) {
  /* 
    Allows you to position the printer and control the feeder
    dimensions are in microns. 1000 = 1mm
    Moves proportionally so all motors arrive at destination at the same time.
  */
    double nx = tablexsteps*x/tablexdistance;
    double ny = tableysteps*y/tableydistance;    
    
    stepper5.moveTo(nx);
    stepper4.moveTo(ny);    
    stepper1.moveTo(z);
    stepper2.moveTo(e); 

    while ((stepper5.distanceToGo() != 0) || (stepper4.distanceToGo() != 0) || (stepper1.distanceToGo() != 0) || (stepper2.distanceToGo() != 0)) {
      stepper5.runSpeedToPosition();
      stepper4.runSpeedToPosition();
      stepper1.runSpeedToPosition();
      stepper2.runSpeedToPosition();
    }        
}
// END RAPID();
// ==============================================================================================================================================================================
// START CIRCLE()
void circle(int x, int y,int z,int e, int rad) {
  double nx = tablexsteps*x/tablexdistance;
  double ny = tableysteps*y/tableydistance;  
  
  for (int i = 0; i < 3600; i++) 
  { 
    float angle = i*2*3.14/1000; 
    double cx = nx + (cos(angle) * (tablexsteps*rad/tablexdistance)); 
    double cy = ny + (sin(angle) * (tableysteps*rad/tableydistance)); 
    stepper5.moveTo(cx);
    stepper4.moveTo(cy);
    stepper1.moveTo(z);
    stepper2.moveTo(e); 
    while ((stepper5.distanceToGo() != 0) || (stepper4.distanceToGo() != 0) || (stepper1.distanceToGo() != 0) || (stepper2.distanceToGo() != 0)) {
          stepper5.runSpeedToPosition();
          stepper4.runSpeedToPosition();
          stepper1.runSpeedToPosition();
          stepper2.runSpeedToPosition();
        }      
  } 
}
// END CIRCLE()
// ==============================================================================================================================================================================
// START LINE()
void line(double x, double y, double z, double e) {
  /* 
    Allows you to position the printer and control the feeder
    dimensions are in microns. 1000 = 1mm
    Moves proportionally so all motors arrive at destination at the same time.
  */
/*
      Serial.print("RAWLINE:X");      Serial.print(x);
      Serial.print(" Y");             Serial.print(y);      
      Serial.print(" Z");             Serial.print(z);            
      Serial.print(" E");             Serial.print(e);  
      */

   double steps = 100;
   
        //calculate new step positions from micron position
   double nx = tablexsteps*(x/tablexdistance);
   double ny = tableysteps*(y/tableydistance);  
   double nz = tablezsteps*(z/tablezdistance);      
   double ne = tableesteps*(e/tableedistance);         

   double cx = float(stepper5.currentPosition());
   double cy = float(stepper4.currentPosition());   
   double cz = float(stepper1.currentPosition());      
   double ce = float(stepper2.currentPosition());         
   
   double dx = ((nx-cx)/steps);
   double dy = ((ny-cy)/steps);   
   double dz = ((nz-cz)/steps);      
   double de = ((ne-ce)/steps);         

   lastx = x;
   lasty = y;
   lastz = z;
   laste = e;

   for (int i = 0; i <= steps; i++) {   
     double newx = cx + (dx*i);
     double newy = cy + (dy*i);     
     double newz = cz + (dz*i);          
     double newe = ce + (de*i);     
      
     stepper5.moveTo(int(newx));
     stepper4.moveTo(int(newy));
     stepper1.moveTo(int(newz));
     stepper2.moveTo(int(newe));      
     while ((stepper5.distanceToGo() != 0) || (stepper4.distanceToGo() != 0) || (stepper1.distanceToGo() != 0) || (stepper2.distanceToGo() != 0)) {
       stepper5.run();
       stepper4.run();
       stepper1.run();
       stepper2.run();
     }  
   }

}
// END LINE();
// ==============================================================================================================================================================================
// START PARSE/PROCESS GCODE

void processMessage(aJsonObject *msg)
{
  int command = 0;
  aJsonObject *jsonpg = aJson.getObjectItem(msg, "G");
  if (jsonpg) {
    if (jsonpg->valueint == 1) {
      double g1x = lastx;
      double g1y = lasty;      
      double g1z = lastz;            
      double g1e = laste;                  
      //G1 PARSE
      // GCODE X
      aJsonObject *jsonpx = aJson.getObjectItem(msg, "X");
      if (jsonpx) {
        int ig1x = jsonpx->valueint;
        g1x = ig1x;
        g1x = g1x * 10;
      }     
      
      // GCODE Y  
      aJsonObject *jsonpy = aJson.getObjectItem(msg, "Y");
      if (jsonpy) {
        int ig1y = jsonpy->valueint;
        g1y = ig1y;
        g1y = g1y * 10;
      }
      
      // GCODE Z
      aJsonObject *jsonpz = aJson.getObjectItem(msg, "Z");
      if (jsonpz) {
        int ig1z = jsonpz->valueint;
        g1z = ig1z;
        g1z = g1z * 10;
      }
      
      aJsonObject *jsonpe = aJson.getObjectItem(msg, "E");
      if (jsonpe) {
        int ig1e = jsonpe->valueint;
        g1e = ig1e;
        g1e = g1e;
      } 
      
      line(g1x, g1y, g1z, g1e);          //G1 RUN
      
    }
    ///END G 1
    /////////////////////////////////////////////////////////////////
  if (jsonpg->valueint == 92) {
      //G92 PARSE
      // GCODE X
       
      aJsonObject *jsonpe = aJson.getObjectItem(msg, "E");
      if (jsonpe) {
        stepper2.setCurrentPosition(0);
        laste = 0;
        stepper2.setSpeed(motorSpeed);
        command += 1;  
      } 
    }
    ///END G92
    ///////////////////////////////////////////////////////////////
  }
   
}

// END PROCESS GCODE
// =======================================================================================================================


