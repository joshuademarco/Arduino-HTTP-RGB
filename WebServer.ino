#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // 16 MHz Adafruit Trinket
#endif
#define PIN        6  // PIN to choose for LED Strip
#define NUMPIXELS 10 // NeoPixel Size
#define DELAYVAL 10 // Time (in milliseconds) to pause between pixels

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;     // network SSID (name)
char pass[] = SECRET_PASS;    // network password (use for WPA, or use as key for WEP)
int keyIndex = 0;      // network key Index number (needed only for WEP)

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
LED power;
RGB color;

StaticJsonDocument<100> jsondoc;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strcpy(power.state, "off");
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
    //Serial.println("CLIENT connected");       print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        if(c != '\r'){
            currentLine += c;
          } else if(c == '\n' && currentLine.length() != 0){
            currentLine = "";
          }

        if(currentLine.endsWith("POST /api/send")){
          char endOfHeaders[] = "\r\n\r\n";
          if (!client.find(endOfHeaders)) {
            endClient(client, "Response INVALID", false);
            return;
          }
          DeserializationError jsonerr = deserializeJson(jsondoc, client);
          if (jsonerr) {
            Serial.print(F("deserializeJson() failed: "));
            endClient(client, jsonerr.c_str(), false);
            return;
          };
          endClient(client, "", true); // Closing connection here to send a response faster
          if(jsondoc.containsKey("state")){
            strcpy(power.state, jsondoc["state"].as<char*>());
            statePower();
          } else if(jsondoc.containsKey("r")){
            color.r = jsondoc["r"].as<byte>();
            color.g = jsondoc["g"].as<byte>();
            color.b = jsondoc["b"].as<byte>();
            updateLED();
          }
        } else if(currentLine.endsWith("GET /api/get")){
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.print("{\"state\":\"");client.print(power.state);client.print("\",\"r\":");client.print(color.r);client.print(",\"g\":");client.print(color.g);client.print(",\"b\":");client.print(color.b);client.print("}<br>");
          client.println();
          delay(100);
          client.stop();
          break;
        }
      }
    }
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

void endClient(WiFiClient client, String msg, bool fbs){ 
  if(fbs == true){ // if feedback to client should contain a success or isn't defined
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    //Serial.println(msg);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.stop();
    //Serial.println("CLIENT disconnected");
    } else {
        Serial.println(msg);
        client.println("HTTP/1.1 500 ERROR");
        client.println("Content-Type: text/html");
        client.println();
        client.println(msg);
        client.println();
        client.stop();
        //Serial.println("CLIENT disconnected");
    }
}

void statePower(){
  if(strcmp(power.state, "on")){
    Serial.println(power.state);
  } else if(strcmp(power.state, "off")){
    Serial.println(power.state);
      for(int i=NUMPIXELS; i>=0; --i) { // Turn each pixel after pixel OFF (transition like)
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();   // Send the updated pixel colors to the hardware.
      delay(DELAYVAL); // Pause before next pass through loop
      }
      pixels.clear();
      pixels.show();
    }
}

void updateLED() {
  Serial.println(color.r);
  Serial.println(color.g);
  Serial.println(color.b);
  for(int i=0; i<NUMPIXELS; i++) { // Turn each pixel after pixel ON (transition like)
    pixels.setPixelColor(i, pixels.Color(color.r, color.g, color.b)); //RGB from 0,0,0 to 255,255,255; here RED...
    pixels.show();   // Send the updated pixel colors to the hardware.
    delay(DELAYVAL); // Pause before next pass through loop
    }
}
