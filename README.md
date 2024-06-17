# iqam - Indoor Air Quality Monitor
  # Introduction
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
-Adafruit BME680 library by Adafruit
-Adafruit GFX library by Adafruit
-Adafruit SSD1306 by Adafruit
-Blynk by Volodymyr Shymanskyy
# Electronic components needed
1.ESP32 microcontroller
2.Oled 0.96 inch
3.BME680 temperature, humidity, pressure, VOCs,... sensor
4.Sharp GP2Y1014AUF dust sensor
5.150 ohm resistor
6.220uF Capacitor
7.Jumper wire
8.USB capble to flash 
9.PCB/breadboard/miniboard
