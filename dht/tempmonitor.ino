#include "PietteTech_DHT.h"
#define DHTTYPE  DHT11              
#define DHTPIN   1                
#define DHT_SAMPLE_INTERVAL   5000  

void dht_wrapper(); 

PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

unsigned int DHTnextSampleTime;        
bool bDHTstarted;            
int n;                              
double bedroom_temp;

void setup()
{
    Serial.begin(9600);
    DHTnextSampleTime = 0;
    Spark.variable("bedroom_temp", &bedroom_temp, DOUBLE);
}

void dht_wrapper() {
    DHT.isrCallback();
}

void loop()
{
  if (millis() > DHTnextSampleTime) {
    if (!bDHTstarted) {        
        DHT.acquire();
        bDHTstarted = true;
    }

    if (!DHT.acquiring()) {

        int result = DHT.getStatus();

        Spark.publish("DEBUG", "Reading sensor ");
        switch (result) {
          case DHTLIB_OK:
              break;
          case DHTLIB_ERROR_CHECKSUM:
              Spark.publish("DEBUG", "Error: Checksum error");
              break;
          case DHTLIB_ERROR_ISR_TIMEOUT:
              Spark.publish("DEBUG", "Error: ISR time out error");
              break;
          case DHTLIB_ERROR_RESPONSE_TIMEOUT:
              Spark.publish("DEBUG", "Error: Response time out error");
              break;
          case DHTLIB_ERROR_DATA_TIMEOUT:
              Spark.publish("DEBUG", "Error: Data time out error");
              break;
          case DHTLIB_ERROR_ACQUIRING:
              Spark.publish("DEBUG", "Error: Acquiring");
              break;
          case DHTLIB_ERROR_DELTA:
              Spark.publish("DEBUG", "Error: Delta time to small");
              break;
          case DHTLIB_ERROR_NOTSTARTED:
              Spark.publish("DEBUG", "Error: Not started");
              break;
          default:
              Spark.publish("DEBUG", "Error: Unknown error");
              break;
        }

      bedroom_temp = DHT.getFahrenheit();
      Spark.publish("DEBUG", String(DHT.getFahrenheit()));

      n++;
      bDHTstarted = false;
      DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  

    }
  }
}
