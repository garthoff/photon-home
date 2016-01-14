// This #include statement was automatically added by the Particle IDE.
#include "rest_client.h"

// This #include statement was automatically added by the Particle IDE.
#include "rest_client.h"

// In this example, we're going to register a Spark.variable() with the cloud so that we can read brightness levels from the photoresistor.
// We'll also register a Spark.function so that we can turn the LED on and off remotely.

// We're going to start by declaring which pins everything is plugged into.
#define PHOTON_NAME "TOTHON1"

int led = D0; // This is where your LED is plugged in. The other side goes to a resistor connected to GND.

int photoresistor = A0; // This is where your photoresistor is plugged in. The other side goes to the "power" pin (below).
int power = A5; // This is the other end of your photoresistor. The other side is plugged into the "photoresistor" pin (above).
// The reason we have plugged one side into an analog pin instead of to "power" is because we want a very steady voltage to be sent to the photoresistor.
// That way, when we read the value from the other side of the photoresistor, we can accurately calculate a voltage drop.

int analogvalue; // Here we are declaring the integer variable analogvalue, which we will use later to store the value of the photoresistor.
int ledvalue;
int pirPin = D4;              // choose the input pin (for PIR sensor)
int pirState = LOW;           // we start, assuming no motion detected
int pirVal = 0;               // variable for reading the pin status
int calibrateTime = 10000;    // wait for the thingy to calibrate

// Next we go into the setup function.

void setup() {

    // First, declare all of our pins. This lets our device know which ones will be used for outputting voltage, and which ones will read incoming voltage.
    pinMode(led,OUTPUT); // Our LED pin is output (lighting up the LED)
    pinMode(photoresistor,INPUT);  // Our photoresistor pin is input (reading the photoresistor)
    pinMode(power,OUTPUT); // The pin powering the photoresistor is output (sending out consistent power)
    pinMode(pirPin, INPUT);     // declare sensor as input

    // Next, write the power of the photoresistor to be the maximum possible, so that we can use this for power.
    digitalWrite(power,HIGH);

    // We are going to declare a Spark.variable() here so that we can access the value of the photoresistor from the cloud.
    Particle.variable("analogvalue", &analogvalue, INT);
    
    // We are going to declare a Spark.variable() here so that we can access the value of the LED from the cloud.
    Particle.variable("ledvalue", &ledvalue, INT);
    // This is saying that when we ask the cloud for the function "led", it will employ the function ledToggle() from this app.
    Particle.function("led",ledToggle);
    
    
    // Now we'll take some readings...
    getLDRValue(analogvalue, photoresistor);

    // Log the start of the Photon
    Spark.publish("Photon_START",PHOTON_NAME);
    
}


void loop() {

  getLDRValue(analogvalue, photoresistor);
  
  //PIR check
  if (calibrated()) {
    getPIRValue();
    reportPIRData();
  }

  delay(5000);  
}

bool calibrated() {
  return millis() - calibrateTime > 0;
}

void getPIRValue() {
  pirVal = digitalRead(pirPin);
}

void reportPIRData() {
  if (pirVal == HIGH) {
    if (pirState == LOW) {
      // we have just turned on
      Spark.publish(PHOTON_NAME,"MOTION");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH) {
      pirState = LOW;
      Spark.publish(PHOTON_NAME,"NO_MOTION_ANYMORE");
    }
  }
}

void getLDRValue(int &analogvalue, int pin)
{
    RestClient client = RestClient(IP_OPENHAB,8080);  // IP_OPENHAB = e.g. 192.168.x.y
    String RestResponse;
    char publishString[256];
    RestResponse = "";
    int statusCode = 0;
  
    analogvalue = analogRead(pin);
    sprintf(publishString,"%u",(int)analogvalue);
    Spark.publish("ldr_1",publishString);
    
    // REST call to OpenHab
    client.setHeader("Content-Type: text/plain");
    statusCode = client.post("/rest/items/PHOTON1_LDR", publishString, &RestResponse);
}


// Finally, we will write out our ledToggle function, which is referenced by the Spark.function() called "led"
int ledToggle(String command) {
    if (command=="on") {
        ledvalue = HIGH;
        digitalWrite(led,ledvalue);
        return 1;
    }
    else if (command=="off") {
        ledvalue = LOW;
        digitalWrite(led,ledvalue);
        return 0;
    }
    else {
        return -1;
    }

}


