/***********************************************************
 * --------------Indoor Air Quality Monitor-----------------
 * # Introduction
 * -Microcontroller: ESP32
 * -This project read temperature, humidity, pressure (in addition, it also measures VOCs gas concentration)
 * from BME680 sensor and read dust density (PM2.5,PM10,...) from Sharp Gp2y1014 sensor.
 * -Calculates Air Quality Index from dust density and from that evaluate air condition in indoor environment.
 * -Display on OLED 0.96 inch
 * -Use Blynk Cloud to send data and show on web dashboard and mobile app via firmware configuration.
 * # Configuration 
 * - In order to use this code for your project/device please change Blynk Firmware Configuration and Wifi credentials in the code below. 
 *********************************************************/
//Blynk Device Firmware Configuration
#define BLYNK_TEMPLATE_ID "TMPL69vG8UEJI"
#define BLYNK_TEMPLATE_NAME "IndoorAirQualityMonitor"
#define BLYNK_AUTH_TOKEN "TDJI-9h4_qlqCkFlT3jpooJlLlrIGaF-"

#include <Wire.h>              // To use i2c communication protocol
#include <Adafruit_GFX.h>      // For OLED display
#include <Adafruit_SSD1306.h>  // For OLED display
#include <Adafruit_Sensor.h>   // For BME680 sensor
#include <Adafruit_BME680.h>   // BME680 Arduino library

#include <WiFi.h>              //For WIFI connection
#include <WiFiClient.h>        //For WIFI connection
#include <BlynkSimpleEsp32.h>  // ESP32 Blynk IoT Library
#include <string.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Unknown";       //Wifi Name
char pass[] = "Nh@t_D@t_2k3";  //Wifi Password

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  //Connection settings for OLED

Adafruit_BME680 bme;

// Dust Density Measure Configuration
int measurePin = 35;
int ledPower = 32;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
// Average dust density declaration
float dustAvg = 0;
float sum_dustDensity = 0;
int dustCount = 0;
//initialize timer object
BlynkTimer timer;
// Flag to track Blynk connection status
bool blynkConnected = false;

void setup() {
  //Device checking
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  bool status = bme.begin(0x77);
  if (!status) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }
  /** 
 *  @brief Set the oversampling rates for measurements
 *  
 * The BME680 sensor allows to define individual oversampling rates for
 * the measurements of temperature, pressure and humidity. Using an
 * oversampling rate of *osr*, the resolution of raw sensor data can be
 * increased by ld(*osr*) bits.
 * 
 * Possible oversampling rates are 1X (default), 2X, 4X, 8X, 16X or NONE (turn off reading).
 * We’ll use the default oversampling parameters, but you can change them.
 */
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  /**
 * @brief   Set the size of the IIR filter
 *
 * The sensor integrates an internal IIR filter (low pass filter) to reduce
 * short-term changes in sensor output values caused by external disturbances.
 * It effectively reduces the bandwidth of the sensor output values.
 *
 * The filter can optionally be used for pressure and temperature data that
 * are subject to many short-term changes. Using the IIR filter, increases the
 * resolution of pressure and temperature data to 20 bit. Humidity and gas
 * inside the sensor does not fluctuate rapidly and does not require such a
 * low pass filtering.
 * It accepts the filter size as a parameter: 1,3,7,15,31,63,127 or 0 (no filtering).
 * The default filter size is 3.
 *
 * Please note: If the size of the filter is 0, the filter is not used.
 */
  bme.setIIRFilterSize(BME680_FILTER_SIZE_7);

  bme.setGasHeater(200, 0);  // 320*C for 150 ms
  //Set time to send data to Blynk Cloud
  timer.setInterval(5000L, bmeSend);
  timer.setInterval(10000L, dustSend);

  pinMode(ledPower, OUTPUT);
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

  // Attempt to connect to Blynk
  WiFi.begin(ssid, pass);
  Blynk.config(auth);
}
//Send data from BME680 to Blynk Cloud
void bmeSend() {
  if (blynkConnected) {
    Blynk.virtualWrite(V0, bme.temperature);
    Blynk.virtualWrite(V1, bme.humidity);
    Blynk.virtualWrite(V2, bme.pressure / 100.0);
  }
}
//Display data from BME680 on OLED screen
void bmeDisplay() {
  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Nhiet do: ");
  display.print(String(bme.temperature));
  display.print(" ");
  display.cp437(true);
  display.write(167);
  display.print("C");
  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 11);
  display.print("Do am: ");
  display.print(String(bme.humidity));
  display.print(" %");
  // display pressure
  display.setTextSize(1);
  display.setCursor(0, 22);
  display.print("Ap suat: ");
  display.print(String(bme.pressure / 100.0));
  display.print(" hPa");
}
/**
 * @brief Dust sampling descripton
 * From datasheet, in one sampling cycle, it takes 10ms to measure. In 10ms:
 * +0.32ms: IR LED will be turned on and begin measure. But value must be read after 0.28ms.
 *  So : IR LED ON -> Delay 0.28ms ->  IR LED OFF -> Delay 0.04ms
 * +9.680ms: This time sensor does nothing. Delay 9.680ms
 *
 * We can exchange measured voltage to dust density as: 0.5V/0.1mg/m^3.
 * But in actual testing, using Linear equation, the more accurate value is calculated from this formula:
 * dustDensity = 170*(calcVoltage)-10 (microgam/m^3).
 *
 * For average dust density measuration, we get 10 dust sample each times.
 */
void dustSampling() {
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin);
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (3.3 / 4096);  // Calculate Voltage from ADC pin
  dustDensity = 170 * (calcVoltage)-10;     //
  if (dustDensity < 0) { dustDensity = 0; }
  sum_dustDensity += dustDensity;
  dustCount++;
  if (dustCount == 10) {
    dustAvg = sum_dustDensity / dustCount;
    dustCount = 0;
    sum_dustDensity = 0;
  }
}
//Display Dust Density and air quality evaluation  on OLED Screen
void dustDisplay() {
  display.setTextSize(1);
  display.setCursor(0, 33);
  display.print("Bui: ");
  display.print(String(dustAvg));
  display.print(" ");
  display.cp437(true);
  display.write(230);
  display.print("g/m3");
  display.setCursor(0, 44);
  display.print("Chat luong:");
  // Evaluate indoor air quality from AQI Index Table
  if (dustAvg <= 35) { display.print("Tot"); }
  if (dustAvg >= 36 && dustAvg <= 75) { display.print("Trung binh"); }
  if (dustAvg >= 76 && dustAvg <= 115) { display.print("Kem"); }
  if (dustAvg >= 116 && dustAvg <= 150) { display.print("Xau"); }
  if (dustAvg >= 151 && dustAvg <= 250) { display.print("Rat xau"); }
  if (dustAvg >= 251) { display.print("Nguy hai"); }
}
//Send Dust Density and air quality evaluation to Blynk Cloud
void dustSend() {
  if (blynkConnected) {
    Blynk.virtualWrite(V3, dustAvg);
    if (dustAvg <= 35) {
      Blynk.virtualWrite(V4, "TỐT");
      Blynk.setProperty(V4, "color", "#8fce00");
    }
    if (dustAvg >= 36 && dustAvg <= 75) {
      Blynk.virtualWrite(V4, "TRUNG BÌNH");
      Blynk.setProperty(V4, "color", "#ffd966");
    }
    if (dustAvg >= 76 && dustAvg <= 115) {
      Blynk.virtualWrite(V4, "KÉM");
      Blynk.setProperty(V4, "color", "#ce7e00");
    }
    if (dustAvg >= 116 && dustAvg <= 150) {
      Blynk.virtualWrite(V4, "XẤU");
      Blynk.setProperty(V4, "color", "#f44336");
    }
    if (dustAvg >= 151 && dustAvg <= 250) {
      Blynk.virtualWrite(V4, "RẤT XẤU");
      Blynk.setProperty(V4, "color", "#c90076");
    }
    if (dustAvg >= 251) {
      Blynk.virtualWrite(V4, "NGUY HẠI");
      Blynk.setProperty(V4, "color", "#741b47");
    }
  }
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!blynkConnected) {
      Blynk.connect();
      if (Blynk.connected()) {
        blynkConnected = true;
      }
    }
  } else {
    blynkConnected = false;
    WiFi.reconnect();
  }
  // Always measure and display sensor data
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  display.clearDisplay();
  bmeDisplay();
  dustSampling();
  dustDisplay();
  if (blynkConnected) {
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("Wifi connected!");
    Blynk.run();
    timer.run();
  } else {
    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("Wifi disconnected!");
  }
  display.display();

  delay(1000);
}