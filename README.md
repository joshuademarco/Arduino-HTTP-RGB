# Arduino-HTTP-RGB
The files `WebServer.ino` and `arduino_secrets.h` enable to receive and process HTTP POST's containing RGB data. I programmed this code in combination with WS2812B / 2M / 60P LED strips. A full documentation and project description can be found here: (coming soon...)
The code is compatible with [Homebridge](https://homebridge.io/).

## What am I using?
- [Arduino Uno Wifi rev 2](https://store.arduino.cc/arduino-uno-wifi-rev2)
- [BTF-Lighting WS2812B / 5M / 60P / IP30](https://www.btf-lighting.com/products/ws2812b-led-pixel-strip-30-60-74-96-100-144-pixels-leds-m)

## How should the POST look like?
The Header doesn't have any influence on any function. So basically every METHOD could be sent as long as it contains the information in the BODY / DATA.
In Homebridge, find a plugin that enables u to send HTTP. Something like this worked fine [homebridge-http-switch-unlimited](https://github.com/downloard/homebridge-http-switch-unlimited). For testing purposes u can use easy tools like **curl**. Ur command could look like this: `curl 192.168.x.x -X POST -d '{"state":"on","r":1,"g":0,"b":20}' --verbose`

**Example**
```
{
  state: "on",
  r: 0,
  g: 0,
  b: 0
}
```

Until now only following key's r implemented:
`state` can be eighter on or off
`r` defines the value of RED in RGB
`g` defines the value of GREEN in RGB
`b` defines the value of BLUE in RGB

## How does it work?
Make sure the Arduino is connected to the described PIN in the script. Please feel free to change the settings to ur needs. The same for the WIFI credentials in `arduino_secrets.h`. First run the programm with the Arduino connected to the PC to see the Serial Monitor output to make sure the connection to the WIFI is successful. **Causion** If u'r using an external power source for ur LED strip, make sure u disconnect it first from the arduino. Two connected power sources may harm ur device if not handled correctly. The status of the connection to ur WIFI will be indicated with the BUILTIN_LED.
Once all is set up and workign fine, send a POST to the Arduino's IP. the POST must contain a `state `,`r`,`g` and `b` key. 

## Why turn the LED off with state and not with RGB = 0
The LED strip is sensitive to DATA and setting the RGB to 0 will not turn it off correctly. There is a `clear()` function which will be called with `"state"="off"`.  
