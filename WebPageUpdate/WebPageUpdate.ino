/*

  this example will show
  1. how to use and ESP 32 for reading pins
  2. building a web page for a client (web browser, smartphone, smartTV) to connect to
  3. sending data from the ESP to the client to update JUST changed data
  4. sending data from the web page (like a slider or button press) to the ESP to tell the ESP to do something

  If you are not familiar with HTML, CSS page styling, and javascript, be patient, these code platforms are
  not intuitive and syntax is very inconsitent between platforms

  I know of 4 ways to update a web page
  1. send the whole page--very slow updates, causes ugly page redraws and is what you see in most examples
  2. send XML data to the web page that will update just the changed data--fast updates but older method
  3. JSON strings which are similar to XML but newer method
  4. web sockets very very fast updates, but not sure all the library support is available for ESP's

  I use JSON here (this example was originally written with XML)...

  JSON is built on the ESP using the ArduinoJson library by Benoit Blanchon
  https://github.com/bblanchon/ArduinoJson
  install it from the Arduino IDE Library Manager (search for "ArduinoJson")
  this code is written for ArduinoJson version 7.x (uses the JsonDocument class)

  compile options
  1. esp32 dev module
  2. upload speed 921600
  3. cpu speed 240 mhz
  flash speed 80 mhz
  flash mode qio
  flash size 4mb
  partition scheme default


  NOTES
1)  if your ESP fails to program press the BOOT button during programm when the IDE is 
"looking for the ESP". Also put a 10 uF elect. cap across EN and ground

2) the max size of esp strings is default to 64K, the page is > 64K, current need is around 85K. If edits are made and web page fails to load, you are probably
over the limit. turn debug on and see size of page. To increase the page size
C:\Users\Administrator\AppData\Local\Arduino15\packages\esp32\hardware\esp32\xxxxx\cores\esp32\WString.h
around line 335 change
CAPACITY_MAX =85000 (current requirement as of 11/10/2025)
//CAPACITY_MAX = 65535

  The MIT License (MIT)

  code writen by Kris Kasprzak
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  On a personal note, if you develop an application or product using this code 
  and make millions of dollars, I'm happy for you!

*/

#include <WiFi.h>         // standard library
#include <WebServer.h>    // standard library
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson used to build the JSON data sent to the web page
#include "SuperMon.h"     // .h file that stores your html page code


// #define DEBUG
// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
#define USE_INTRANET

// replace this with your homes intranet connect parameters
#define LOCAL_SSID "---------"
#define LOCAL_PASS "---------"

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "TestWebSite"
#define AP_PASS "023456789"

// start your defines for pins for sensors, outputs etc.
#define PIN_OUTPUT 26  // connected to nothing but an example of a digital write from the web page
#define PIN_FAN 27     // pin 27 and is a PWM signal to control a fan speed
#define PIN_LED 2      //On board LED
#define PIN_A0 33      // some analog input sensor
#define PIN_A1 32      // some analog input sensor

// variables to store measure data and sensor states
int BitsA0 = 0, BitsA1 = 0;
float VoltsA0 = 0, VoltsA1 = 0;
int FanSpeed = 0;
bool LED0 = false, SomeOutput = false;
uint32_t SensorUpdate = 0;
int FanRPM = 0;

// the JSON text buffer needs to be bigger than your maximum expected size
// the JSON in this example is roughly 70 characters, so 256 leaves plenty of room to add more data
// if you add a lot more data, increase this size (SendJSON will warn on the serial monitor if it's too small)
char JSONTxt[256];

// just some buffer holder for char operations
char buf[32];

// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

// gotta create a server
WebServer server(80);

void setup() {

  // standard stuff here
  Serial.begin(9600);

  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_OUTPUT, OUTPUT);


  // turn off led
  LED0 = false;
  digitalWrite(PIN_LED, LED0);

  analogWriteFrequency(PIN_FAN, 15000);
  analogReadResolution(12);

  // configure LED PWM functionalitites
  /* old ESP compiler
  ledcSetup(0, 10000, 8);
  ledcAttachPin(PIN_FAN, 0);
  ledcWrite(0, FanSpeed);
  */

  // if your web page or JSON are large, you may not get a call back from the web page
  // and the ESP will think something has locked up and reboot the ESP
  // you may consider disabling the watch dog timers
  // disableCore0WDT();
  // disableCore1WDT();

  // just an update to progress
  Serial.println("starting server");

  // if you have this #define USE_INTRANET,  you will connect to your home intranet, again makes debugging easier
#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Actual_IP = WiFi.localIP();
#endif

  // if you don't have #define USE_INTRANET, here's where you will creat and access point
  // an intranet with no internet connection. But Clients can connect to your intranet and see
  // the web page you are about to serve up
#ifndef USE_INTRANET
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(Actual_IP);
#endif

  printWifiStatus();


  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent
  server.on("/", SendWebsite);

  // upon esp getting /json string, ESP will build and send the JSON, this is how we refresh
  // just parts of the web page
  server.on("/json", SendJSON);

  // upon ESP getting /UPDATE_SLIDER string, ESP will execute the UpdateSlider function
  // same notion for the following .on calls
  // add as many as you need to process incoming strings from your web page
  // as you can imagine you will need to code some javascript in your web page to send such strings
  // this process will be documented in the SuperMon.h web page code
  server.on("/UPDATE_SLIDER", UpdateSlider);
  server.on("/BUTTON_0", ProcessButton_0);
  server.on("/BUTTON_1", ProcessButton_1);

  // finally begin the server
  server.begin();
}

void loop() {

  // you main loop that measures, processes, runs code, etc.
  // note that handling the "on" strings from the web page are NOT in the loop
  // that processing is in individual functions all managed by the wifi lib

  // in my example here every 50 ms, i measure some analog sensor data (my finger dragging over the pins
  // and process accordingly
  // analog input can be from temperature sensors, light sensors, digital pin sensors, etc.
  if ((millis() - SensorUpdate) >= 50) {
    //Serial.println("Reading Sensors");
    SensorUpdate = millis();
    BitsA0 = analogRead(PIN_A0);
    BitsA1 = analogRead(PIN_A1);

    // standard converion to go from 12 bit resolution reads to volts on an ESP
    VoltsA0 = BitsA0 * 3.3f / 4095.0f;
    VoltsA1 = BitsA1 * 3.3f / 4095.0f;
  }

  // no matter what you must call this handleClient repeatidly--otherwise the web page
  // will not get instructions to do something
  server.handleClient();
}


// function managed by an .on method to handle slider actions on the web page
// this example will get the passed string called VALUE and conver to a pwm value
// and control the fan speed
void UpdateSlider() {

  // man I hate strings, but wifi lib uses them...
  String t_state = server.arg("VALUE");

  // conver the string sent from the web page to an int
  FanSpeed = t_state.toInt();
  Serial.print("UpdateSlider");
  Serial.println(FanSpeed);
  // now set the PWM duty cycle
  // old ESP compiler
  // ledcWrite(0, FanSpeed);

  // latest ESP compiler
  Serial.println(FanSpeed);
  analogWrite(PIN_FAN, FanSpeed);


  // YOU MUST SEND SOMETHING BACK TO THE WEB PAGE--BASICALLY TO KEEP IT LIVE

  // option 1: send no information back, but at least keep the page live
  // just send nothing back
  // server.send(200, "text/plain", ""); //Send web page

  // option 2: send something back immediately, maybe a pass/fail indication, maybe a measured value
  // here is how you send data back immediately and NOT through the general JSON page update code
  // my simple example guesses at fan speed--ideally measure it and send back real data
  // i avoid strings at all caost, hence all the code to start with "" in the buffer and build a
  // simple piece of data
  FanRPM = map(FanSpeed, 0, 255, 0, 2400);
  strcpy(buf, "");
  sprintf(buf, "%d", FanRPM);

  // now send it back
  server.send(200, "text/plain", buf);  //Send web page
}

// now process button_0 press from the web site. Typical applications are the used on the web client can
// turn on / off a light, a fan, disable something etc
void ProcessButton_0() {

  LED0 = !LED0;
  digitalWrite(PIN_LED, LED0);
  Serial.print("Button 0 ");
  Serial.println(LED0);
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the JSON manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main JSON page update
  // code
  server.send(200, "text/plain", "");  //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */
}

// same notion for processing button_1
void ProcessButton_1() {

  // just a simple way to toggle a LED on/off. Much better ways to do this
  Serial.println("Button 1 press");
  SomeOutput = !SomeOutput;

  digitalWrite(PIN_OUTPUT, SomeOutput);
  Serial.print("Button 1 = ");
  Serial.println(SomeOutput);
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or send all data via JSON use this method
  // sending feeback
  server.send(200, "text/plain", "");  //Send web page

  // if you want to send feed back immediataly
  // note you must have proper code in the java script to read this data stream
  /*
    if (some_process) {
    server.send(200, "text/plain", "SUCCESS"); //Send web page
    }
    else {
    server.send(200, "text/plain", "FAIL"); //Send web page
    }
  */
}


// code to send the main web page
// PAGE_MAIN is a large char defined in SuperMon.h
void SendWebsite() {

  Serial.println("sending web page");

  server.send(200, "text/html", PAGE_MAIN);
}

// code to send the sensor data to the web page as JSON
// the web page calls this every 100 ms (see process() in SuperMon.h)
// the JSON we build looks like this:
// {"B0":1234,"V0":1.0,"B1":2345,"V1":1.9,"LED":false,"SWITCH":true}
// I avoid string data types at all cost, so the JSON is written into a char buffer
void SendJSON() {

  // Serial.println("sending json");

  // create a JSON document. In ArduinoJson 7 the document grows as needed, so no size is required here
  // it is created locally so the memory is released when this function exits
  JsonDocument doc;

  // send bitsA0 as a number
  doc["B0"] = BitsA0;
  // send Volts0 rounded to 1 decimal place. JSON numbers are sent as-is, so the web page
  // also formats the value to 1 decimal place for display
  doc["V0"] = round(VoltsA0 * 10) / 10.0;

  // send bits1
  doc["B1"] = BitsA1;
  // send Volts1
  doc["V1"] = round(VoltsA1 * 10) / 10.0;

  // show led0 status. JSON supports true/false natively, so no need to convert to "1" or "0"
  doc["LED"] = LED0;

  // show the switch status
  doc["SWITCH"] = SomeOutput;

  // if the buffer is too small, the JSON gets truncated (and is invalid), so check that it will fit
  // measureJson() returns the number of characters needed (not counting the null terminator)
  if (measureJson(doc) >= sizeof(JSONTxt)) {
    Serial.println("JSON buffer is too small, increase the size of char JSONTxt[]");
  }

  // write the JSON document into the char buffer (serializeJson always adds the null terminator)
  serializeJson(doc, JSONTxt, sizeof(JSONTxt));

// wanna see what the JSON looks like?
// print it to the serial monitor, then pad and adjust char JSONTxt[256]; above if needed
#ifdef DEBUG
  Serial.println(JSONTxt);
#endif
  // send the JSON to the web page. application/json is the standard content type for JSON data
  server.send(200, "application/json", JSONTxt);
}

// I think I got this code from the wifi example
void printWifiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Open http://");
  Serial.println(ip);
}

// end of code
