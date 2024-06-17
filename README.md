# iqam - Indoor Air Quality Monitor
<p align="center"><img src="/images/iaqm.jpg" width="200px"/></p>
  # Introduction #
  
  <p align="center"><img src="/images/iaqm thumbnail.jpg" /></p>
  
 * -Microcontroller: ESP32
 * -This project read temperature, humidity, pressure (in addition, it also measures VOCs gas concentration)
 * from BME680 sensor and read dust density (PM2.5,PM10,...) from Sharp Gp2y1014 sensor.
 * -Calculates Air Quality Index from dust density and from that evaluate air condition in indoor environment.
 * -Display on OLED 0.96 inch
 * -Use Blynk Cloud to send data and show on web dashboard and mobile app via firmware configuration.
 # Configuration 
  - In order to use this code for your project/device please change Blynk Firmware Configuration and Wifi credentials in the code. 
# Library
Add the following library into your Arduino IDE library (by searching it on the left panel)
1. Open Arduino IDE.
2. Goto `Sketch` > `Include Library` > `Manage Libraries...`.
3. Search `MyLibrary`.
4. Choose `MyLibrary` and click `Install`.
* Adafruit BME680 library by Adafruit
* Adafruit GFX library by Adafruit
* Adafruit SSD1306 by Adafruit
* Blynk by Volodymyr Shymanskyy
# Electronic components needed
* ESP32 microcontroller
* Oled 0.96 inch
* BME680 temperature, humidity, pressure, VOCs,... sensor
* Sharp GP2Y1014AUF dust sensor
* 150 ohm resistor
* 220uF Capacitor
* Jumper wire
* USB capble to flash 
* PCB/breadboard/miniboard

<p align="center"><img src="/images/schematic.png" /></p>
