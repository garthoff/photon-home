// This #include statement was automatically added by the Particle IDE.
#include "rgb-controls/rgb-controls.h"
using namespace RGBControls;

Led led(D0, D1, D2);
Color red(255, 0, 0);
Color green(0, 255, 0);
Color blue(0, 0, 255);

    // color swirl! connect an RGB LED to the PWM pins as indicated
    // in the #defines
    // public domain, enjoy!
     
//    #define REDPIN D0
 //   #define GREENPIN D1
 //   #define BLUEPIN D2
     
    
     
    void setup() {
      Particle.function("RGB",rgbcol);
 //     pinMode(REDPIN, OUTPUT);
  //    pinMode(GREENPIN, OUTPUT);
  //    pinMode(BLUEPIN, OUTPUT);
    }
     
     
    void loop() {
        /*
      int r, g, b;
     
      // fade from blue to violet
      for (r = 0; r < 256; r++) { 
        analogWrite(REDPIN, r);
        delay(FADESPEED);
      } 
      // fade from violet to red
      for (b = 255; b > 0; b--) { 
        analogWrite(BLUEPIN, b);
        delay(FADESPEED);
      } 
      // fade from red to yellow
      for (g = 0; g < 256; g++) { 
        analogWrite(GREENPIN, g);
        delay(FADESPEED);
      } 
      // fade from yellow to green
      for (r = 255; r > 0; r--) { 
        analogWrite(REDPIN, r);
        delay(FADESPEED);
      } 
      // fade from green to teal
      for (b = 0; b < 256; b++) { 
        analogWrite(BLUEPIN, b);
        delay(FADESPEED);
      } 
      // fade from teal to blue
      for (g = 255; g > 0; g--) { 
        analogWrite(GREENPIN, g);
        delay(FADESPEED);
      } 
      */
      delay(1000);
    }
    
    int rgbcol(String command) {
    /* Spark.functions always take a string as an argument and return an integer.
    Since we can pass a string, it means that we can give the program commands on how the function should be used.
    In this case, telling the function "on" will turn the LED on and telling it "off" will turn the LED off.
    Then, the function returns a value to us to let us know what happened.
    In this case, it will return 1 for the LEDs turning on, 0 for the LEDs turning off,
    and -1 if we received a totally bogus command that didn't do anything to the LEDs.
    */
    
    

    int r=-1;
    int g=-1;
    int b=-1;
    int commaIndex = command.indexOf(',');
    //  Search for the next comma just after the first
    int secondCommaIndex = command.indexOf(',', commaIndex+1);
    int len= command.length();
    String firstValue = command.substring(0, commaIndex);
    String secondValue = command.substring(commaIndex+1, secondCommaIndex);
    String thirdValue = command.substring(secondCommaIndex+1,len-1); // To the end of the string

    r=firstValue.toInt();
    g=secondValue.toInt();
    b=thirdValue.toInt();

    Color custom(r,g,b);
    
    if(r != -1)
    {
        command="cust";
    }
    


    if (command=="100") {
//        analogWrite(REDPIN, 100);
        led.setColor(red);
        return 1;
    }
    else if (command=="010") {
//        analogWrite(GREENPIN, 100);    
        led.setColor(green);
        return 1;
    }
    else if (command=="001") {
  //      analogWrite(BLUEPIN, 100);         
        led.setColor(blue);
        return 1;
    }
        else if (command=="000") {
  //      analogWrite(BLUEPIN, 100);         
        led.off();
        return 1;
    }
    else if (command=="cust") {
  //      analogWrite(BLUEPIN, 100);         
        led.setColor(custom);
        return 1;
    }
    else {
        return -1;
    }
}
