//indicators
int lInd = 12;
int rInd = 13;
int hazardButton = 7;
int lButton = 8;
int rButton = 9;
int lState = LOW;
int rState = LOW;
unsigned long previousMillis = 0;
const long interval = 500;

//headlights
int photoresistor1 = A1;
int photoresistor2 = A2;
int photoresistor3 = A3;
int transistor = 5; //2N5457 JFET 

//TOF
  #include <Wire.h>
  #include <VL53L1X.h>
  #define XSHUTL 2
  #define XSHUTR 3
  #define XSHUTRELV 4
  VL53L1X distanceSensorL;
  VL53L1X distanceSensorR;
  VL53L1X relvSensor;
  boolean ind;
  int distance1;
  int distance2;
  int timeCurr;
  int delta;
  int relv;
  int buzz = 6; //pin number make sure to assign pin mode
  unsigned long previousMillisTOF = 0;

  




void setup() {
  Serial.begin(115200);

  //indicators
  pinMode(lInd, OUTPUT);
  pinMode(rInd, OUTPUT);
  pinMode (hazardButton, INPUT);
  pinMode (lButton, INPUT);
  pinMode (rButton, INPUT);

  //headlights
  pinMode(photoresistor1, INPUT);
  pinMode(photoresistor2, INPUT);
  pinMode(photoresistor3, INPUT);
  pinMode(transistor, OUTPUT);

  //TOF
   Wire.begin();
   Wire.setClock(400000); // use 400 kHz I2C
   distanceSensorL.setTimeout(500);
   distanceSensorR.setTimeout(500);
   //Toggle XSHUT to reset the sensor
   pinMode(XSHUTR, OUTPUT);
   pinMode(XSHUTR, INPUT);
   //FIND OUT ORDER IN WHICH THE SENSORS ARE INITIALIZED (IF THAT MATTERS) SO THAT WAY YOU CAN DISTINGUISH BETWEEN INITIALIZING RELV SENSOR AND THE TWO WARNING SENSORS.
   if (!distanceSensorR.init()) // This initializes sensor and returns a boolean value
   {
     Serial.println("Failed to detect and initialize Right warning sensor!");
     while (1);
   }
   distanceSensorR.setAddress(15);
   
   pinMode(XSHUTL, OUTPUT);
   pinMode(XSHUTL, INPUT);
   if (!distanceSensorL.init())
   {
     Serial.println("Failed to detect and initialize Left warning sensor!");
     while (1);
   }
   distanceSensorL.setAddress(12);
  
   pinMode(XSHUTRELV, OUTPUT);
   pinMode(XSHUTRELV, INPUT);
   if (!relvSensor.init())
   {
     Serial.println("Failed to detect and initialize sensor!");
     while (1);
   }
   relvSensor.setAddress(9);
   
   distanceSensorL.setDistanceMode(VL53L1X::Long);
   distanceSensorL.setMeasurementTimingBudget(50000);
   distanceSensorL.startContinuous(50);
   distanceSensorR.setDistanceMode(VL53L1X::Long);
   distanceSensorR.setMeasurementTimingBudget(50000);
   distanceSensorR.startContinuous(50);
   relvSensor.setDistanceMode(VL53L1X::Long);
   relvSensor.setMeasurementTimingBudget(50000);
   relvSensor.startContinuous(50);
  
}

void loop() {

  headlight();
  indicator();
  relv = relVelocity();
  warnings(distanceSensorR);
  warnings(distanceSensorL);
  
  

}


//indicator
void indicator(){
  if(digitalRead(hazardButton)== LOW)
    hazardLights();
  else if (digitalRead(lButton)== LOW)
    blinkLeft();
  else if (digitalRead(rButton)== LOW)
    blinkRight();
  else {
    lState = LOW;
    rState = LOW;
    digitalWrite(lInd, lState);
    digitalWrite(rInd, rState);
  }
}

void blinkLeft(){
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
     // if the LED is off turn it on and vice-versa:
    if (lState == LOW) {
      lState = HIGH;
    } else {
      lState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(lInd, lState);
    
    }
}

void blinkRight() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
     // if the LED is off turn it on and vice-versa:
    if (rState == LOW) {
      rState = HIGH;
    } else {
      rState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(rInd, rState);
    
    }
}

void hazardLights(){
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
     // if the LED is off turn it on and vice-versa:
    if (lState == LOW & rState == LOW) {
      lState = HIGH;
      rState = HIGH;
    } else {
      lState = LOW;
      rState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(lInd, lState);
    digitalWrite(rInd, rState);
    
    }
}


//headlights
void headlight(){
  int light1 = analogRead(photoresistor1);  
  int light2 = analogRead(photoresistor2);  
  int light3 = analogRead(photoresistor3);

  int avgLight = (light1 + light2 + light3)/3;
  
  int transVal;
  if(avgLight>150) 
    transVal = 255;
  else if (avgLight > 30)
    transVal = 2.125*avgLight;
  else 
    transVal = 0;
  analogWrite(transistor, transVal);
}

//relative velocity
int relVelocity(){
   distance1= relvSensor.read();
   unsigned long currentMillis = millis();
   if (currentMillis - previousMillisTOF <= interval){
    previousMillisTOF = currentMillis;
    distance2 = relvSensor.read();
   }
   delta = distance2 -distance1;
   relv = 4*delta; // relv in mm/second
   return relv;
  }
  
//warning sensor
  void warnings( VL53L1X sensor){
    int distance = sensor.read(); //Write configuration bytes to initiate measurement
    float distanceInches = distance * 0.0393701;
    float distancefeet = distanceInches / 12.0;
    buzzerFunc(distancefeet);
  }

  void buzzerFunc(int dist){
    if (dist < 10  && ind == true){
       unsigned long del = 200+ 50*dist;
       tone(buzz,128,del);
    }
    else if (dist < 3){
       tone(buzz,128,200);
    }
  }


 
