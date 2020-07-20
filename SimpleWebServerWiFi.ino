#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // 16 MHz Adafruit Trinket
#endif
#define PIN        6  // PIN to choose for LED Strip
#define NUMPIXELS 119 // NeoPixel Size
#define DELAYVAL 10 // Time (in milliseconds) to pause between pixels

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID     // network SSID (name)
char pass[] = SECRET_PASS;    // network password (use for WPA, or use as key for WEP)
int keyIndex = 0      // network key Index number (needed only for WEP)
char json[] = "";
struct led { // LED status buffer
  char state[6]; 
};
struct rgb { // RGB Color buffer
  byte r;
  byte g;
  byte b;
};
typedef led LED; // Defining
typedef rgb RGB;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.clear(); // Set all pixel colors to 'off'
  Serial.begin(9600);      // initialize serial communication
  pinMode(LED_BUILTIN, OUTPUT);      // set the LED pin mode
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    digitalWrite(LED_BUILTIN, LOW);
    // don't continue
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    delay(2000);
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // connected now, print out the status
  digitalWrite(LED_BUILTIN, HIGH);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
 }


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if a client connects,
    Serial.println("new Client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          currentLine = "";
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          } else if (c == '\u007D'){
            StaticJsonDocument<256> jsondoc;
            String i = currentLine + "}";
            i.toCharArray(json, currentLine.length());
            DeserializationError err = deserializeJson(jsondoc, i);
            if (err) {
              Serial.print("deserializeJson() failed with code ");
              Serial.println(err.c_str());
              client.println("HTTP/1.1 500 ERROR");
              client.println("Content-type:text/html");
              client.println();
              client.println(err.c_str());
              client.println();
              client.stop();
              break;
              } else {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                // The HTTP response ends with another blank line:
                client.println();
                delay(100);
                client.stop();
                Serial.println("client disonnected");
                }
              LED lights;
              RGB color;
              if(!jsondoc["state"].isNull()){ // If the POST does contain a state
                const char* test = jsondoc["state"];
                strcpy(lights.state, jsondoc["state"]);
              } else if(jsondoc["r"] || jsondoc["g"] || jsondoc["b"]) {
                  color.r = jsondoc["r"];
                  color.g = jsondoc["g"];
                  color.b = jsondoc["b"];                
              }
              updateLights(lights, color);
              break;
        } else if (c != '\r') {  // if anything but a carriage return character,
          currentLine += c;      // add to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}


void updateLights(LED lights, RGB color) {
  Serial.println(lights.state);
  Serial.println(color.r);
  Serial.println(color.g);
  Serial.println(color.b);
  if (String(lights.state) == "on") {
      for(int i=0; i<NUMPIXELS; i++) { // Turn each pixel after pixel ON (transition like)
        pixels.setPixelColor(i, pixels.Color(color.r, color.g, color.b)); //RGB from 0,0,0 to 255,255,255; here RED...
        pixels.show();   // Send the updated pixel colors to the hardware.
        delay(DELAYVAL); // Pause before next pass through loop
          }
        } else if (String(lights.state) == "off") {
          for(int i=NUMPIXELS; i>=0; --i) { // Turn each pixel after pixel OFF (transition like)
            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
            pixels.show();   // Send the updated pixel colors to the hardware.
            delay(DELAYVAL); // Pause before next pass through loop
          }
          delay(100);
          pixels.clear();
          pixels.show();
          delay(100);
          }
}
