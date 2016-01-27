#include "DS18B20/Particle-OneWire.h"
// This #include statement was automatically added by the Particle IDE.
#include "DS18B20/DS18B20.h"



// This #include statement was automatically added by the Particle IDE.
#include "rest_client.h"



// -----------------------------------------
// Function and Variable with Photoresistors
// -----------------------------------------
// In this example, we're going to register a Spark.variable() with the cloud so that we can read brightness levels from the photoresistor.
// We'll also register a Spark.function so that we can turn the LED on and off remotely.

// We're going to start by declaring which pins everything is plugged into.
#define PHOTON_NAME "TOTHON1"



int photoresistor = A0; // This is where your photoresistor is plugged in. The other side goes to the "power" pin (below).
int power = A5; // This is the other end of your photoresistor. The other side is plugged into the "photoresistor" pin (above).
// The reason we have plugged one side into an analog pin instead of to "power" is because we want a very steady voltage to be sent to the photoresistor.
// That way, when we read the value from the other side of the photoresistor, we can accurately calculate a voltage drop.

int analogvalue; // Here we are declaring the integer variable analogvalue, which we will use later to store the value of the photoresistor.

    RestClient client = RestClient("192.168.1.200",8080);


///
DS18B20 ds18b20 = DS18B20(D4); //Sets Pin D2 for Water Temp Sensor
int led = D7;
char szInfo[64];
float pubTemp;
double celsius;
double fahrenheit;
unsigned int Metric_Publish_Rate = 5000;
unsigned int MetricnextPublishTime;
int DS18B20nextSampleTime;
int DS18B20_SAMPLE_INTERVAL = 2500;
int dsAttempts = 0;


// Next we go into the setup function.

void setup() {

    // First, declare all of our pins. This lets our device know which ones will be used for outputting voltage, and which ones will read incoming voltage.
   
    pinMode(photoresistor,INPUT);  // Our photoresistor pin is input (reading the photoresistor)
    pinMode(power,OUTPUT); // The pin powering the photoresistor is output (sending out consistent power)
   

    // Next, write the power of the photoresistor to be the maximum possible, so that we can use this for power.
    digitalWrite(power,HIGH);

    // We are going to declare a Spark.variable() here so that we can access the value of the photoresistor from the cloud.
     Spark.variable("analogvalue", &analogvalue, INT);
    
    // We are going to declare a Spark.variable() here so that we can access the value of the LED from the cloud.

    // Now we'll take some readings...
    getLDRValue(analogvalue, photoresistor);

    Time.zone(-5);
    Particle.syncTime();
    pinMode(D4, INPUT);
    Particle.variable("tempHotWater", &fahrenheit, DOUBLE);
    
    // Log the start of the Photon
    Spark.publish("Photon_START",PHOTON_NAME);
    
}


// Next is the loop function...

void loop() {
     if (millis() > 1000){
  // LDR check
  getLDRValue(analogvalue, photoresistor);
     }
  if (millis() > DS18B20nextSampleTime){
  getTemp();
  }

  if (millis() > MetricnextPublishTime){
  //  Particle.publish("Publishing now.");
    publishData();
  }

   // Particle.process();
}


void getLDRValue(int &analogvalue, int pin)
{
   // RestClient client = RestClient("192.168.1.200",8080);
    String RestResponse;
    char publishString[256];
    RestResponse = "";
    int statusCode = 0;
  
    analogvalue = analogRead(pin);
    sprintf(publishString,"%u",(int)analogvalue);
//    Spark.publish("ldr_1",publishString);
    
    // REST call to OpenHab
    client.setHeader("Content-Type: text/plain");
    statusCode = client.post("/rest/items/PHOTON1_LDR", publishString, &RestResponse);
}



void publishData(){
    String RestResponse;
    char publishString[256];
    RestResponse = "";
    int statusCode = 0;
    
  if(!ds18b20.crcCheck()){
    return;
  }
  sprintf(szInfo, "%2.2f", (fahrenheit-32)*5/9);
  //Particle.publish("dsTmp", szInfo, PRIVATE);
  
      // REST call to OpenHab
  client.setHeader("Content-Type: text/plain");
  statusCode = client.post("/rest/items/PHOTON1_TMP", szInfo, &RestResponse);
    
  MetricnextPublishTime = millis() + Metric_Publish_Rate;
}

void getTemp(){
    if(!ds18b20.search()){
      ds18b20.resetsearch();
      celsius = ds18b20.getTemperature();
      Serial.println(celsius);
      while (!ds18b20.crcCheck() && dsAttempts < 4){
        Particle.publish("Caught bad value.");
        dsAttempts++;
        Serial.print("Attempts to Read: ");
        Serial.println(dsAttempts);
        if (dsAttempts == 3){
          delay(1000);
        }
        ds18b20.resetsearch();
        celsius = ds18b20.getTemperature();
        continue;
      }
      dsAttempts = 0;
      fahrenheit = ds18b20.convertToFahrenheit(celsius);
      DS18B20nextSampleTime = millis() + DS18B20_SAMPLE_INTERVAL;
      Serial.println(fahrenheit);
    }
}

