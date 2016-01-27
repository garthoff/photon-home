#include "DS18B20/Particle-OneWire.h" // load the OneWire library
#include "DS18B20/DS18B20.h" // load the DS18B20 libarary

DS18B20 ds18b20 = DS18B20(D2); //Sets Pin D2 for Water Temp Sensor, why we used D2
int led = D7; // no obvious reason why that's here. Useful for testing as it's on-board.
char szInfo[64]; 
float pubTemp; 
double celsius; //
double fahrenheit;
unsigned int Metric_Publish_Rate = 30000;
unsigned int MetricnextPublishTime;
int DS18B20nextSampleTime;
int DS18B20_SAMPLE_INTERVAL = 2500;
int dsAttempts = 0;

void setup() {
    Time.zone(-5);
    Particle.syncTime();
    pinMode(D2, INPUT);
    // Exposes the fahrenheit variable to the cloud as tempHotWater.
    Particle.variable("tempHotWater", &fahrenheit, DOUBLE);
    Serial.begin(115200);
}

void loop() {

if (millis() > DS18B20nextSampleTime){
  getTemp();
  }

  if (millis() > MetricnextPublishTime){
    Serial.println("Publishing now.");
    publishData();
  }
}

// Function that will SSE publish to the cloud
void publishData(){
  if(!ds18b20.crcCheck()){
    return;
  }
  sprintf(szInfo, "%2.2f", fahrenheit);
  // publish a private event with name "dsTmp" and data "szInfo"
  Particle.publish("dsTmp", szInfo, PRIVATE); 
  MetricnextPublishTime = millis() + Metric_Publish_Rate;
}

// This is what gets the temperature from the sensor.
void getTemp(){
    if(!ds18b20.search()){
      ds18b20.resetsearch();
      celsius = ds18b20.getTemperature();
      Serial.println(celsius);
      while (!ds18b20.crcCheck() && dsAttempts < 4){
        Serial.println("Caught bad value.");
        dsAttempts++;
        Serial.print("Attempts to Read: ");
        Serial.println(dsAttempts);
        if (dsAttempts == 3){
          delay(1000);
        }
        ds18b20.resetsearch();
        celsius = ds18b20.getTemperature(); // gets temperature in Celcius
        continue;
      }
      dsAttempts = 0;
      fahrenheit = ds18b20.convertToFahrenheit(celsius); // converts to Fahrenheit
      DS18B20nextSampleTime = millis() + DS18B20_SAMPLE_INTERVAL;
      Serial.println(fahrenheit);
    }
}
