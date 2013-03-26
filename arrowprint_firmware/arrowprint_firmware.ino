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

String version = "5.1";



/* 
 *  =======================================================================================
 *    CONFIGURATION
 *  =======================================================================================
 */
int cmd = 0;
//int hotheadtarget = 160;    //160 DEFAULT
int hotheadtarget = 10;    //160 DEFAULT

//physical movement scale (bigger == more);
double tablexsteps    = 23000;     
double tableysteps    = 23000;
double tablezsteps    = 349100;    //234000 (100mm) from top (STEPS FROM TABLE SURFACE TO TOP) 0,0,0 is FAR LEFT BOTTOM
double tableesteps  = -1000000;

//measured (1000 = 1mm)
double tablexdistance = 134.765625;    //in microns (158mm )
double tableydistance = 134.765625;    //in microns (200mm)
double tablezdistance = 61.913;    //in microns (96mm)
double tableedistance = 920.000;    //in microns (96mm)


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
double stepperXspeed = 110;

int motorpinYstep = 26;
int motorpinYdir  = 27;
double stepperYcurrPosition = 0;
double stepperYdestPosition = 0;
double stepperYtimer = 0;
int    stepperYtoggle = 0;
double stepperYspeed = 110;  //slower for more power

int motorpinZstep = 30;
int motorpinZdir  = 31;
double stepperZcurrPosition = 0;
double stepperZdestPosition = 0;
double stepperZtimer = 0;
int    stepperZtoggle = 0;
double stepperZspeed = 110;

int motorpinEstep = 28;
int motorpinEdir  = 29;
double stepperEcurrPosition = 0;
double stepperEdestPosition = 0;
double stepperEtimer = 0;
int    stepperEtoggle = 0;
double stepperEspeed = 110;


// Motor Zero switch pins
int xZeropin = 50;        //INPUT pin
int yZeropin = 51;        //INPUT pin
int zZeropin = 52;        //INPUT pin

//thermo control
int thermoswitch = 2;    //output switch for turning on melter
int thermo = A0;         //input for reading temperature of melter

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
        if (tempcounter > 5) {
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

void thermistor() {
   Serial.print("{ \"thermistor\" : \"");
   Serial.print(analogRead(thermo));
   Serial.print("\"}");   
}

//=======================================================================================


void zeroplane() {
      /////////////////////////////////////////////////////////// X LEFT RIGHT

    while (digitalRead(xZeropin) != xZeroNC) {   
      stepperXdestPosition = stepperXdestPosition - 32;
      while (stepperXdistanceToGo() != 0) {
        stepperXrun();
      }
    }
    
    stepperXcurrPosition = -2000;
    //moves to zero
    stepperXdestPosition = 0;
    lastx = 0;
    while (stepperXdistanceToGo() != 0) {
      stepperXrun();
    }
    ////////////////////////////////////////////////////////////// Y TABLE FRONT BACK
    while (digitalRead(yZeropin) != yZeroNC) {   
      stepperYdestPosition = stepperYdestPosition + 32;
      while (stepperYdistanceToGo() != 0) {
        stepperYrun();
      }      
    }
    //moves to 0
    stepperYcurrPosition = tableysteps+10000;
    stepperYdestPosition = 0;
    lasty = 0;
    while (stepperYdistanceToGo() != 0) {
      stepperYrun();
    }      
}

// START ZEROAXIS()
void zeroaxis() {
  /*
      Runs each motor towards the zero switch and resets its position. Must be run at start.
  */

    ////////////////////////////////////////////////////////ZERO Z - UPDOWN
    Serial.print("{ \"log\" : \"Attempting to zero Z axis\"}");

    while (digitalRead(zZeropin) != zZeroNC) {             
      stepperZdestPosition = stepperZdestPosition + 16;
      while (stepperZdistanceToGo() != 0) {
        stepperZrun();
      }
    }

    stepperZcurrPosition = tablezsteps + 6400;
 
    //moves to zero
    stepperZdestPosition = 0;      
    lastz = 0;
    while (stepperZdistanceToGo() != 0) {
      stepperZrun();
    }   
    /////////////////////////////////////////////////////////// X LEFT RIGHT
    Serial.print("{ \"log\" : \"Attempting to zero X axis\"}");    
    while (digitalRead(xZeropin) != xZeroNC) {   
      stepperXdestPosition = stepperXdestPosition - 32;
      while (stepperXdistanceToGo() != 0) {
        stepperXrun();
      }
    }
    
    stepperXcurrPosition = -2000;
    //moves to zero
    stepperXdestPosition = 0;
    lastx = 0;
    while (stepperXdistanceToGo() != 0) {
      stepperXrun();
    }
    
    Serial.print("{ \"log\" : \"X axis zero success\"}");
    ////////////////////////////////////////////////////////////// Y TABLE FRONT BACK
    Serial.print("{ \"log\" : \"Attempting to zero Y axis\"}");   
    while (digitalRead(yZeropin) != yZeroNC) {   
      stepperYdestPosition = stepperYdestPosition + 32;
      while (stepperYdistanceToGo() != 0) {
        stepperYrun();
      }      
    }
    //moves to 0
    stepperYcurrPosition = tableysteps+10000;
    stepperYdestPosition = 0;
    lasty = 0;
    while (stepperYdistanceToGo() != 0) {
      stepperYrun();
    }      
    ////////////////////////////////////////////////////////////// E zero
    Serial.print("{ \"log\" : \"Attempting to test E axis\"}");   
    stepperEdestPosition = 0;  //1 rotation
    while (stepperEdistanceToGo() != 0) {
        stepperErun();
      }      
    //moves to 0
    stepperEcurrPosition = 0;
    stepperEdestPosition = 0;
    laste = 0;
    Serial.print("{ \"log\" : \"Extruder stepper axis zero success\"}");  
    
}
// END ZEROAXIS()
// ==============================================================================================================================================================================
// START LINE()
void line(double x, double y, double z, double e) {

  //DISTANCESTEPS
   double distance = sqrt( pow((float)(x - lastx),2) + pow( (float)(y - lasty),2) + pow( (float)(z - lastz), 2) + pow( (float)(e - laste), 2));
   double steps = round(distance*500);
   
        //calculate new step positions from micron position
   double nx = tablexsteps*(x/tablexdistance);
   double ny = tableysteps*(y/tableydistance);  
   double nz = tablezsteps*(z/tablezdistance);      
   double ne = tableesteps*(e/tableedistance);                

   double cx = float(stepperXcurrPosition);
   double cy = float(stepperYcurrPosition);   
   double cz = float(stepperZcurrPosition);      
   double ce = float(stepperEcurrPosition);         

  
   double dx = ((nx-cx)/steps);
   double dy = ((ny-cy)/steps);   
   double dz = ((nz-cz)/steps);      
   double de = ((ne-ce)/steps);         

   
   //distance
   /*
   double deltax = lastx - x;
   double deltay = lasty - y;
   double deltaz = lastz - z;
   double deltae = laste - e;   
   Serial.print("{ \"deltax\" : \"");
   Serial.print(deltax);
   Serial.print("\", \"deltay\" : \"");   
   Serial.print(deltay);   
   Serial.print("\", \"deltaz\" : \"");   
   Serial.print(deltaz);   
   Serial.print("\", \"deltae\" : \"");   
   Serial.print(deltae);      
   Serial.println("\"}");
   */
   //
  
   lastx = x;
   lasty = y;
   lastz = z;
   laste = e;
    
   for (int i = 0; i <= steps; i++) {   
     double newx = cx + (dx*i);
     double newy = cy + (dy*i);     
     double newz = cz + (dz*i);          
     double newe = ce + (de*i);     
      
     stepperXdestPosition = round(newx);
     stepperYdestPosition = round(newy);
     stepperZdestPosition = round(newz);
     stepperEdestPosition = round(newe);     
     
     
     while ((stepperXdistanceToGo() != 0) || (stepperYdistanceToGo() != 0) || (stepperZdistanceToGo() != 0) || (stepperEdistanceToGo() != 0)) {
       ///GET THESE INTO THE MAIN LOOP
       // NEED TO REFACTOR EVENT LOOP
       
         stepperXrun();
         stepperYrun();
         stepperZrun();
         stepperErun();
       
       
       } 
   }
  
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
        zeroplane();
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
        stepperEcurrPosition = 0.0f;
        stepperEdestPosition = 0.0f;
        laste = 0.0f; 
      } 
    }///END G92
    
    ///////////////////////////////////////////////////////////////
  }

}

// END PROCESS GCODE
// =======================================================================================================================

///////========================vvvvvvvv STEPPER CONTROLLERS vvvvvv==================

double stepperXdistanceToGo() { return stepperXdestPosition - stepperXcurrPosition; }

void stepperXrun() {
  double dir = 0;
  stepperXdestPosition = round(stepperXdestPosition);
  stepperXcurrPosition = round(stepperXcurrPosition);
  if (stepperXdestPosition != stepperXcurrPosition) {
    //SET DIRECTION
    if (stepperXdestPosition > stepperXcurrPosition) {
      digitalWrite(motorpinXdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinXdir, LOW); 
      dir = -1;
    }
    //PULSE AT SPEED LIMIT
    if (micros() > (stepperXtimer+stepperXspeed)) {
      digitalWrite(motorpinXstep, stepperXtoggle);
      stepperXtimer = micros();
      stepperXtoggle = !stepperXtoggle;
      stepperXcurrPosition += dir;
    }
  }
}

double stepperYdistanceToGo() { return stepperYdestPosition - stepperYcurrPosition; }

void stepperYrun() {
  double dir = 0;
  stepperYdestPosition = round(stepperYdestPosition);
  stepperYcurrPosition = round(stepperYcurrPosition);  
  if (stepperYdestPosition != stepperYcurrPosition) {
    //SET DIRECTION
    if (stepperYdestPosition > stepperYcurrPosition) {
      digitalWrite(motorpinYdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinYdir, LOW); 
      dir = -1;
    }
    //PULSE AT SPEED LIMIT
    if (micros() > (stepperYtimer+stepperYspeed)) {
      digitalWrite(motorpinYstep, stepperYtoggle);
      stepperYtimer = micros();
      stepperYtoggle = !stepperYtoggle;
      stepperYcurrPosition += dir;
    }
  }
}

double stepperZdistanceToGo() { return stepperZdestPosition - stepperZcurrPosition; }

void stepperZrun() {
  double dir = 0;
  stepperZdestPosition = round(stepperZdestPosition);
  stepperZcurrPosition = round(stepperZcurrPosition);  
  if (stepperZdestPosition != stepperZcurrPosition) {
    //SET DIRECTION
    if (stepperZdestPosition > stepperZcurrPosition) {
      digitalWrite(motorpinZdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinZdir, LOW); 
      dir = -1;
    }
    //PULSE AT SPEED LIMIT
    if (micros() > (stepperZtimer+stepperZspeed)) {
      digitalWrite(motorpinZstep, stepperZtoggle);
      stepperZtimer = micros();
      stepperZtoggle = !stepperZtoggle;
      stepperZcurrPosition += dir;
    }
  }
}

double stepperEdistanceToGo() { return stepperEdestPosition - stepperEcurrPosition; }

void stepperErun() {
  double dir = 0;
  stepperEdestPosition = round(stepperEdestPosition);
  stepperEcurrPosition = round(stepperEcurrPosition);    
  if (stepperEdestPosition != stepperEcurrPosition) {
    //SET DIRECTION
    if (stepperEdestPosition > stepperEcurrPosition) {
      digitalWrite(motorpinEdir, HIGH);
      dir = 1;
    } else { 
      digitalWrite(motorpinEdir, LOW); 
      dir = -1;
    }
    //PULSE AT SPEED LIMIT
    if (micros() > (stepperEtimer+stepperEspeed)) {
      digitalWrite(motorpinEstep, stepperEtoggle);
      stepperEtimer = micros();
      stepperEtoggle = !stepperEtoggle;
      stepperEcurrPosition += dir;
    }
  }
}

///////===================^^^^^^^^^^^^^ STEPPER CONTROLLERS ^^^^^^^^^^^^^============
