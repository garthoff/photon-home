// This #include statement was automatically added by the Particle IDE.
#include "rest_client.h"


// -----------------------------------------
// Function and Variable with Photoresistors
// -----------------------------------------
// In this example, we're going to register a Spark.variable() with the cloud so that we can read brightness levels from the photoresistor.
// We'll also register a Spark.function so that we can turn the LED on and off remotely.

// We're going to start by declaring which pins everything is plugged into.
#define PHOTON_NAME "TOTHON3"

int led = D0; // This is where your LED is plugged in. The other side goes to a resistor connected to GND.
int photoresistor = A0; // This is where your photoresistor is plugged in. The other side goes to the "power" pin (below).
int pirPin = D4;              // choose the input pin (for PIR sensor)
int power = A5; // This is the other end of your photoresistor. The other side is plugged into the "photoresistor" pin (above).
// The reason we have plugged one side into an analog pin instead of to "power" is because we want a very steady voltage to be sent to the photoresistor.
// That way, when we read the value from the other side of the photoresistor, we can accurately calculate a voltage drop.

int analogvalue; // Here we are declaring the integer variable analogvalue, which we will use later to store the value of the photoresistor.
int ledvalue;
int pirState = LOW;             // we start, assuming no motion detected
int pirVal = 0;                    // variable for reading the pin status
int calibrateTime = 10000;      // wait for the thingy to calibrate

#define NEC_BITS 32
#define NEC_HDR_MARK	9000
#define NEC_HDR_SPACE	4500
#define NEC_BIT_MARK	560
#define NEC_ONE_SPACE	1690
#define NEC_ZERO_SPACE	560
#define NEC_RPT_SPACE	2250
#define MARK_EXCESS 100

#define IRPIN D5
//you may increase this value on Arduinos with greater than 2k SRAM
#define maxLen 800

volatile  unsigned int irBuffer[maxLen]; //stores timings - volatile because changed by ISR
volatile unsigned int x = 0; //Pointer thru irBuffer - volatile because changed by ISR

int MATCH_MARK(int measured_us, int desired_us) {
  return measured_us >= (desired_us - MARK_EXCESS) && measured_us <= (desired_us + MARK_EXCESS);
}

int MATCH_SPACE(int measured_us, int desired_us) {
  return measured_us >= (desired_us - MARK_EXCESS) && measured_us <= (desired_us + MARK_EXCESS);
}

// NECs have a repeat only 4 items long
long decodeNEC(int *irData) {
  long data = 0;
  int offset = 0;
  // Initial mark
  if (!MATCH_MARK(irData[offset], NEC_HDR_MARK)) {
    Serial.println("ERR No NEC Header");
    return -1;
  }
  offset++;
  // Check for repeat
  if (x == 4 &&
    MATCH_SPACE(irData[offset], NEC_RPT_SPACE) &&
    MATCH_MARK(irData[offset+1], NEC_BIT_MARK)) {
    /*results->bits = 0;
    results->value = REPEAT;
    results->decode_type = NEC;*/
    Serial.println("NEC Repeat");
    return data;
  }
  if (x < 2 * NEC_BITS + 4) {
    Serial.println("ERR not enough data");
    return -1;
  }
  // Initial space
  if (!MATCH_SPACE(irData[offset], NEC_HDR_SPACE)) {
    Serial.println("ERR No NEC Header");
    return -1;
  }
  offset++;
  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(irData[offset], NEC_BIT_MARK)) {
      Serial.println("ERR Expected a Mark");
      return -1;
    }
    offset++;
    if (MATCH_SPACE(irData[offset], NEC_ONE_SPACE)) {
      Serial.print("1");
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(irData[offset], NEC_ZERO_SPACE)) {
      Serial.print("0");
      data <<= 1;
    }
    else {
      Serial.println("ERR Expected a Space");
      return -1;
    }
    offset++;
  }
  // Success
  /*results->bits = NEC_BITS; */ //32 bits
  return data;
  /*results->decode_type = NEC;*/
  /*return DECODED;*/
}


// Next we go into the setup function.

void setup() {

    attachInterrupt(IRPIN, rxIR_Interrupt_Handler, CHANGE);//set up ISR for receiving IR signal

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
    
 //   Particle.variable("temperature", &temperature, DOUBLE);
    
 //   Particle.variable("humidity", &humidity, DOUBLE);
    // This is saying that when we ask the cloud for "analogvalue", this will reference the variable analogvalue in this app, which is an integer variable.

    // We are also going to declare a Spark.function so that we can turn the LED on and off from the cloud.
   // Particle.function("led",ledToggle);
    // This is saying that when we ask the cloud for the function "led", it will employ the function ledToggle() from this app.
    
    // Now we'll take some readings...
    getLDRValue(analogvalue, photoresistor);
    
      // Log the start of the Photon
    Particle.publish("Photon_START",PHOTON_NAME);
    
}


// Next is the loop function...

void loop() {
  char publishString[256];
  // LDR check
  getLDRValue(analogvalue, photoresistor);
  
  //PIR check
  if (calibrated()) {
    getPIRValue();
    reportPIRData();
  }
  
//////////////////////////////////////////////////////////////

 if (x) { //if a signal is captured
    detachInterrupt(IRPIN);//stop interrupts & capture until finshed here

    int irData[x];
    for (int i = 1; i < x; i++) 
    { //now dump the times
      int timing = irBuffer[i] - irBuffer[i - 1];
      irData[i-1] = timing;
      //Serial.print(irData[i-1]);
      if (i + 1 < x) 
      {
         // Serial.print(F("\t"));
            Spark.publish(" - ",PHOTON_NAME);
      }
    }
    
    long necCode = decodeNEC(irData);

    sprintf(publishString,"%u",(long)necCode);
    Particle.publish("NEC IR",PHOTON_NAME);
    Particle.publish("NEC IR",publishString);
    
   // Serial.println(necCode, HEX);
   // Serial.println(necCode, BIN);

    x = 0;
    attachInterrupt(IRPIN, rxIR_Interrupt_Handler, CHANGE);//re-enable ISR for receiving IR signal
  }


/////////////////////////////////////////////////////////////
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
      Particle.publish(PHOTON_NAME,"MOTION");
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH) {
      pirState = LOW;
    }
  }
}


void getLDRValue(int &analogvalue, int pin)
{
    RestClient client = RestClient(IP_OPENHAB,8080);  // IP_OPENHAB e.g. 192.168.x.y
    String RestResponse;
    char publishString[256];
    RestResponse = "";
    int statusCode = 0;
  
    analogvalue = analogRead(pin);
    sprintf(publishString,"%u",(int)analogvalue);
    Particle.publish("ldr_3",publishString);
    
    // REST call to OpenHab
    client.setHeader("Content-Type: text/plain");
    statusCode = client.post("/rest/items/PHOTON3_LDR", publishString, &RestResponse);
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

void rxIR_Interrupt_Handler() {
  if (x > maxLen) return; //ignore if irBuffer is already full
  irBuffer[x++] = micros(); //just continually record the time-stamp of signal transitions

}

