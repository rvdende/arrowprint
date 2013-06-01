/*
 * Arrow 3D Printer Firmware
 * by Rouan van der Ende - fluentart.com - code meets art
 * rouan@8bo.org - 8bitoctopus - creative studio
 * 
 * Use arduino toolkit version 1.5.2 or later
 * Make sure you use an Arduino DUE (Or Leonardo)
 * Install aJSON library
 */
#include <aJSON.h>

String version = "5.3";

int substeps = 16;

/* 
 *  =======================================================================================
 *    CONFIGURATION
 *  =======================================================================================
 */
int cmd = 0;
//int hotheadtarget = 160;    //160 DEFAULT
int hotheadtarget = 10;    //160 DEFAULT

//physical movement scale (bigger == more);
double tablexsteps    = 1920.0 * substeps;     
double tableysteps    = 2134.0 * substeps;
double tablezsteps    = 22400.0 * substeps;    //234000 (100mm) from top (STEPS FROM TABLE SURFACE TO TOP) 0,0,0 is FAR LEFT BOTTOM
double tableesteps  = 72000.000 * substeps;  //-45000 safe //66666.666

//measured (1000 = 1mm)
double tablexdistance = 180;    //in microns (158mm )
double tableydistance = 200.0625;    //in microns (200mm)
double tablezdistance = 70.0;    //in microns (96mm)
double tableedistance = 1000.000;    //in microns (96mm)


// Motor pins
// step = pin for step on controller
// dir  = pin for direction on controller
// speed is minimum delay between pulses in microseconds
int motorpinXstep = 24;
int motorpinXdir  = 25;
double stepperXcurrPosition = 0;
double stepperXdestPosition = 0;
double stepperXtimer = 0;
int    stepperXtoggle = 0;
double stepperXspeed = 2600 / substeps;

int motorpinYstep = 26;
int motorpinYdir  = 27;
double stepperYcurrPosition = 0;
double stepperYdestPosition = 0;
double stepperYtimer = 0;
int    stepperYtoggle = 0;
double stepperYspeed = 2600 / substeps;  //slower for more power

int motorpinZstep = 30;
int motorpinZdir  = 31;
double stepperZcurrPosition = 0;
double stepperZdestPosition = 0;
double stepperZtimer = 0;
int    stepperZtoggle = 0;
double stepperZspeed = 2600 / substeps;

int motorpinEstep = 28;
int motorpinEdir  = 29;
double stepperEcurrPosition = 0;
double stepperEdestPosition = 0;
double stepperEtimer = 0;
int    stepperEtoggle = 0;
double stepperEspeed = 4800 / substeps;


// Motor Zero switch pins
int xZeropin = 34;        //INPUT pin
int yZeropin = 33;        //INPUT pin
int zZeropin = 32;        //INPUT pin

//thermo control
int thermoswitch = 2;    //output switch for turning on melter
int thermo = A0;         //input for reading temperature of melter

//thermo LED
int thermoheat = 36;     // on when we are heating up
int thermordy = 37;      // on when we are hot enough to print

//NC/NO toggles
int xZeroNC  =  1;       //Normally closed/open
int yZeroNC  =  1;       //Normally closed/open
int zZeroNC  =  0;       //Normally closed/open



// =========================================================================================
// INTERNAL DO NOT CHANGE BELOW
//latest position in microns (1000 = 1mm)

double lastx = 0.0;
double lasty = 0.0;
double lastz = 0.0;
double laste = 0.0;


// =========================================================================================


void setup() {
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(115200);
  Serial.print("{ \"log\" : \"Connecting...\"}");
  //ZERO SWITCHES
  pinMode(xZeropin, INPUT);                  //X SWITCH
  pinMode(yZeropin, INPUT);                  //Y SWITCH
  pinMode(zZeropin, INPUT);                  //Z SWITCH

  //Thermo
  pinMode(thermo, INPUT);              //Analogue temperature measure
  pinMode(thermoswitch, OUTPUT);       //Heater mosfet switch
  digitalWrite(thermoswitch, LOW);     //Defaults to OFF/LOW
  
  pinMode(motorpinXstep, OUTPUT);
  pinMode(motorpinXdir, OUTPUT);  
  pinMode(motorpinYstep, OUTPUT);
  pinMode(motorpinYdir, OUTPUT);    
  pinMode(motorpinZstep, OUTPUT);
  pinMode(motorpinZdir, OUTPUT);    
  pinMode(motorpinEstep, OUTPUT);
  pinMode(motorpinEdir, OUTPUT);    

  pinMode(thermoheat, OUTPUT);
  pinMode(thermordy, OUTPUT);

  thermistor();
  zeroaxis();                                
  thermistor();
  
  
  Serial.print("{ \"log\" : \"Success\"}");
  Serial.print("{ \"version\" : \""+version+"\"}");  
  pinMode(13, OUTPUT);
  Serial.print("{ \"status\" : \"waiting\"}"); 
}

aJsonStream serial_stream(&Serial);

double lastcmd = 0;
double tempcounter = 0;

void loop() {
    double lastloop = millis();
    while (serial_stream.available()) {
        aJsonObject *msg = aJson.parse(&serial_stream);
        processMessage(msg);
        lastcmd = millis();
        aJson.deleteItem(msg);                

        tempcounter++;
        if (tempcounter > 15) {
            thermistor();
            tempcounter = 0;
        }

        Serial.print("{ \"status\" : \"waiting\"}"); 
    } 
    
    
    if (!serial_stream.available()) {
      
    }    

    
    if (lastloop > lastcmd+1000) {
        pinMode(13, HIGH);
        delay(250);        
        pinMode(13, LOW);                
        delay(250);                
        pinMode(13, HIGH);
        delay(250);        
        pinMode(13, LOW);                
        delay(250);                
        Serial.print("{ \"status\" : \"waiting\"}"); 
        pinMode(13, HIGH);
        delay(250);        
        pinMode(13, LOW);                
        delay(250);                
        pinMode(13, HIGH);
        delay(250);        
        pinMode(13, LOW);                
        delay(250);        
    }


    
}
//=======================================================================================

double hotendtemp = 0;
double hotendlasttemp = 0;
void thermistor() {
    double readtry = analogRead(thermo);
    if (readtry > 0) {
      hotendtemp = readtry;
    }
   
   Serial.print("{ \"thermistor\" : \"");
   Serial.print(hotendtemp);
   Serial.print("\"}");   
   
   if (hotendtemp > 3750) {
     digitalWrite(thermordy, HIGH);
   } else {
     digitalWrite(thermordy, LOW);
   }
   
   if (hotendlasttemp > hotendtemp) {
     digitalWrite(thermoheat, HIGH);
   } else {
     digitalWrite(thermoheat, LOW);
   }
   
   hotendlasttemp = hotendtemp;    
}

//=======================================================================================




// START ZEROAXIS()
void zeroaxis() {
  /*
      Runs each motor towards the zero switch and resets its position. Must be run at start.
  */

    ////////////////////////////////////////////////////////ZERO Z - UPDOWN
    
    //slightly up while on zeroswitch.

   while (digitalRead(zZeropin) == zZeroNC) {          
    stepperZdestPosition = stepperZdestPosition + (20000 * substeps);
      while (stepperZdistanceToGo() != 0) {
        stepperZrun();
      }
   }
    
    //back down to zero
    
   while (digitalRead(zZeropin) != zZeroNC) {             
      stepperZdestPosition = stepperZdestPosition - 32;
      while (stepperZdistanceToGo() != 0) {
        stepperZrun();
      }
    }

    //top most is equal to tablezsteps
    stepperZcurrPosition = 0;
    //moves to zero (table surface)
    stepperZdestPosition = 0;      
    while (stepperZdistanceToGo() != 0) {
      stepperZrun();
    }   
    lastz = 0;    

    /////////////////////////////////////////////////////////// X LEFT RIGHT

    while (digitalRead(xZeropin) != xZeroNC) {   
      stepperXdestPosition = stepperXdestPosition - substeps;
      while (stepperXdistanceToGo() > 0) {
        stepperXrun();
      }
    }
    stepperXcurrPosition = 0;
    lastx = 0;

    //moves to zero
    stepperXdestPosition = 0;
    while (stepperXdistanceToGo() != 0) {
      stepperXrun();
    }
    lastx = 0;
    

    ////////////////////////////////////////////////////////////// Y TABLE FRONT BACK
 
    while (digitalRead(yZeropin) != yZeroNC) {   
      stepperYdestPosition = stepperYdestPosition + substeps;
      while (stepperYdistanceToGo() != 0) {
        stepperYrun();
      }      
    }
    //zero switch is on high side
    stepperYcurrPosition = tableysteps;
    
    //moves to 0
    stepperYdestPosition = 0;
    lasty = 0;
    while (stepperYdistanceToGo() != 0) {
      stepperYrun();
    }      
    
    ////////////////////////////////////////////////////////////// E zero

    stepperEdestPosition = 0;  //1 rotation
    while (stepperEdistanceToGo() != 0) {
        stepperErun();
      }      
    //moves to 0
    stepperEcurrPosition = 0;
    stepperEdestPosition = 0;
    laste = 0;
    
    //WAIT
    line(45.0, 45.0, 5.0, 0.0);
    
}
// END ZEROAXIS()
// ==============================================================================================================================================================================
// START LINE()
void line(double x, double y, double z, double e) {
   double distance = sqrt( pow((float)(x - lastx),2) + pow( (float)(y - lasty),2) + pow( (float)(z - lastz), 2) + pow( (float)(e - laste), 2) );
   double steps = round(distance*50); // distance to move * (1mm / (circum/steps per rotation))
   if (steps < 1) { steps = 1; }
  
   for (int i = 0; i <= steps; i = i + 1) {   
     double stepratio = i/steps;
     double newx = lastx + ((x-lastx) * stepratio);
     double newy = lasty + ((y-lasty) * stepratio);     
     double newz = lastz + ((z-lastz) * stepratio);          
     double newe = laste + ((e-laste) * stepratio);  

     double newxsteps = tablexsteps*(newx/tablexdistance);
     double newysteps = tableysteps*(newy/tableydistance);  
     double newzsteps = tablezsteps*(newz/tablezdistance);      
     double newesteps = tableesteps*(newe/tableedistance); 
     
     newxsteps = newxsteps/2;
     newysteps = newysteps/2;
     newzsteps = newzsteps/2;
     newesteps = newesteps/2;     
      
     stepperXdestPosition = round(newxsteps)*2;
     stepperYdestPosition = round(newysteps)*2;
     stepperZdestPosition = round(newzsteps)*2;
     stepperEdestPosition = round(newesteps)*2;   
     
     while ((stepperXdistanceToGo() != 0) || (stepperYdistanceToGo() != 0) || (stepperZdistanceToGo() != 0) || (stepperEdistanceToGo() != 0)) {
         stepperXrun();
         stepperYrun();
         stepperZrun();
         stepperErun();      
       } 
   }
   
   lastx = x;
   lasty = y;
   lastz = z;
   laste = e;
  
}
// END LINE();
// ==============================================================================================================================================================================
// START PARSE/PROCESS GCODE

void processMessage(aJsonObject *msg)
{
  
  int command = 0;
  aJsonObject *jsonpg = aJson.getObjectItem(msg, "cmd");
  if (jsonpg) {
    String cmd = jsonpg->valuestring;
    if (cmd == "G1") {   
      double g1x = lastx;
      double g1y = lasty;      
      double g1z = lastz;            
      double g1e = laste;                  
      //G1 PARSE

      /*
      aJsonObject *jsonpx = aJson.getObjectItem(msg, "x");
      if (jsonpx) {  g1x = jsonpx->valuefloat;   }     
      
      aJsonObject *jsonpy = aJson.getObjectItem(msg, "y");
      if (jsonpy) {  g1y = jsonpy->valuefloat;   }
      
      aJsonObject *jsonpz = aJson.getObjectItem(msg, "z");
      if (jsonpz) {  g1z = jsonpz->valuefloat;   }
      
      aJsonObject *jsonpe = aJson.getObjectItem(msg, "e");
      if (jsonpe) {  g1e = jsonpe->valuefloat;   
        Serial.print("EVAL:");
        Serial.println(g1e);
      }       
      */
      aJsonObject *jsonpx = aJson.getObjectItem(msg, "x");
      if (jsonpx) {  
        char* tempx = jsonpx->valuestring;
        g1x = atof(tempx);                 
      }     
      
      aJsonObject *jsonpy = aJson.getObjectItem(msg, "y");
      if (jsonpy) {  
        char* tempy = jsonpy->valuestring;
        g1y = atof(tempy);   
      }
      
      aJsonObject *jsonpz = aJson.getObjectItem(msg, "z");
      if (jsonpz) {  
        char* tempz = jsonpz->valuestring;
        g1z = atof(tempz);   
      }
      
      aJsonObject *jsonpe = aJson.getObjectItem(msg, "e");
      if (jsonpe) {  
        char* tempe = jsonpe->valuestring;
        g1e = atof(tempe);   
      }       
      
      line(g1x, g1y, g1z, g1e);          //G1 RUN
      
    }
    ///END G 1
    /////////////////////////////////////////////////////////////////
  if (cmd == "G92") {   
      aJsonObject *jsonpe = aJson.getObjectItem(msg, "e");
      if (jsonpe) {
        stepperEcurrPosition = 0.0;
        stepperEdestPosition = 0.0;
        laste = 0.0; 
      } 
    }///END G92
    
    ///////////////////////////////////////////////////////////////
  }

}

// END PROCESS GCODE
// =======================================================================================================================

///////========================vvvvvvvv STEPPER CONTROLLERS vvvvvv==================

double stepperXdistanceToGo() { return abs(stepperXdestPosition - stepperXcurrPosition); }

void stepperXrun() {
  double dir = 0;
  
  if (stepperXdistanceToGo() >= 1) {
    //SET DIRECTION
    if (stepperXdestPosition > stepperXcurrPosition) {
      digitalWrite(motorpinXdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinXdir, LOW); 
      dir = -1;
    }

    if (micros() < (stepperXtimer-stepperXspeed)) {
      //rollover bug... fix
      stepperXtimer = micros();
    }
    
    //PULSE AT SPEED LIMIT
    if (micros() >= (stepperXtimer+stepperXspeed)) {
      digitalWrite(motorpinXstep, stepperXtoggle);
      stepperXtimer = micros();
      stepperXtoggle = !stepperXtoggle;
      stepperXcurrPosition += dir;
    }
    

  }
}

double stepperYdistanceToGo() { return abs(stepperYdestPosition - stepperYcurrPosition); }

void stepperYrun() {
  double dir = 0;
  if (stepperYdistanceToGo() >= 1) {
    //SET DIRECTION
    if (stepperYdestPosition > stepperYcurrPosition) {
      digitalWrite(motorpinYdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinYdir, LOW); 
      dir = -1;
    }
    if (micros() < (stepperYtimer-stepperYspeed)) {
      //rollover bug... fix
      stepperYtimer = micros();
    }      
    //PULSE AT SPEED LIMIT
    if (micros() >= (stepperYtimer+stepperYspeed)) {
      digitalWrite(motorpinYstep, stepperYtoggle);
      stepperYtimer = micros();
      stepperYtoggle = !stepperYtoggle;
      stepperYcurrPosition += dir;
    }
  
  }
}

double stepperZdistanceToGo() { return abs(stepperZdestPosition - stepperZcurrPosition); }

void stepperZrun() {
  double dir = 0;
  if (stepperZdistanceToGo() >= 1) {
    //SET DIRECTION
    if (stepperZdestPosition > stepperZcurrPosition) {
      digitalWrite(motorpinZdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinZdir, LOW); 
      dir = -1;
    }
    if (micros() < (stepperZtimer-stepperZspeed)) {
      //rollover bug... fix
      stepperZtimer = micros();
    }       
    //PULSE AT SPEED LIMIT
    if (micros() >= (stepperZtimer+stepperZspeed)) {
      digitalWrite(motorpinZstep, stepperZtoggle);
      stepperZtimer = micros();
      stepperZtoggle = !stepperZtoggle;
      stepperZcurrPosition += dir;
    }
  }
}

double stepperEdistanceToGo() { return abs(stepperEdestPosition - stepperEcurrPosition); }

void stepperErun() {
  double dir = 0;
  
  if (stepperEdistanceToGo() >= 1) {
    //SET DIRECTION
    if (stepperEdestPosition > stepperEcurrPosition) {
      digitalWrite(motorpinEdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinEdir, LOW); 
      dir = -1;
    }
    if (micros() < (stepperEtimer-stepperEspeed)) {
      //rollover bug... fix
      stepperEtimer = micros();
    }       
    //PULSE AT SPEED LIMIT
    if (micros() >= (stepperEtimer+stepperEspeed)) {
      digitalWrite(motorpinEstep, stepperEtoggle);
      stepperEtimer = micros();
      stepperEtoggle = !stepperEtoggle;
      stepperEcurrPosition += dir;
    }
  }
}

///////===================^^^^^^^^^^^^^ STEPPER CONTROLLERS ^^^^^^^^^^^^^============
