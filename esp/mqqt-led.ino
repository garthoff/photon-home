#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// #define PWMRANGE 255
// #define PWM_VALUE 31
//int gamma_table[PWM_VALUE+1] = {
// 0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
// 27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
//};
 
 #define PWM_VALUE 63
//int gamma_table[PWM_VALUE+1] = {
// 0, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10,
// 11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
// 61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
// 279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
//};

int gamma_table[PWM_VALUE+1] = {
 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 8, 9, 10,
 11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
 61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
 279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
};



// RGB FET
#define redPIN 12
#define greenPIN 14
#define bluePIN 16


// W FET
#define w1PIN 13

// note 
// TX GPIO2 @Serial (Serial ONE)
// RX GPIO3 @Serial 

const char *ssid = "openhardwarecoza"; // your network SSID (name)
const char *pass = "novell1234"; // your network password


// Update these with values suitable for your network.
IPAddress server(192, 168, 1, 200);

void callback(const MQTT::Publish& pub) {
 Serial.print(pub.topic());
 Serial.print(" => ");
 Serial.println(pub.payload_string());
 
 String payload = pub.payload_string();

if(String(pub.topic()) == "/openHAB/RGB_2/Color"){
 int c1 = payload.indexOf(';');
 int c2 = payload.indexOf(';',c1+1);
 
 int red = map(payload.toInt(),0,100,0,PWM_VALUE);
 red = constrain(red,0,PWM_VALUE);
 int green = map(payload.substring(c1+1,c2).toInt(),0,100,0,PWM_VALUE);
 green = constrain(green, 0, PWM_VALUE);
 int blue = map(payload.substring(c2+1).toInt(),0,100,0,PWM_VALUE);
 blue = constrain(blue,0,PWM_VALUE);
 
 red = gamma_table[red];
 green = gamma_table[green];
 blue = gamma_table[blue];
 
 
 analogWrite(redPIN, red);
 analogWrite(greenPIN, green);
 analogWrite(bluePIN, blue);

// Serial.println(red);
// Serial.println(green);
// Serial.println(blue);
 }
 else if(String(pub.topic()) == "/openHAB/RGB_2/SW1"){
 int value = map(payload.toInt(),0,100,0,PWM_VALUE);
 value = constrain(value,0,PWM_VALUE);
 value = gamma_table[value];
 analogWrite(w1PIN, value);
 //Serial.println(value);
 }
 
}

PubSubClient client(server);

void setup()
{
 
 pinMode(redPIN, OUTPUT);
 pinMode(greenPIN, OUTPUT);
 pinMode(bluePIN, OUTPUT);
 pinMode(w1PIN, OUTPUT);
 
 // Setup console
 Serial.begin(115200);
 delay(10);
 Serial.println();
 Serial.println();

 client.set_callback(callback);

 WiFi.begin(ssid, pass);


 
 while (WiFi.status() != WL_CONNECTED) {

 
 delay(500);
 Serial.print(".");
 
 }
 
 Serial.println("");
 
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
 
 
 if (client.connect("WiFi_RGB_2")) {
 client.subscribe("/openHAB/RGB_2/Color");
 client.subscribe("/openHAB/RGB_2/SW1");
 Serial.println("MQTT connected"); 
 }

 Serial.println("");
}


void loop()
{
 client.loop();
}
