#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#if not defined (_VARIANT_ARDUINO_DUE_X_)
  #include <SoftwareSerial.h>
#endif


String skipITID = "1";


#define NEO_PIN        15
#define NUMPIXELS      32
Adafruit_NeoPixel pixels;

const int httpPort = 8080;
const char* ssid     = "skipit";
const char* password = "flowfactory";
const char* host = "10.0.0.6";

#define BNO055_SAMPLERATE_DELAY_MS (100)
Adafruit_BNO055 bno;

float prevMag = 0;
int color_shift = 0;
int rise = 1; // ==1 => ascend, ==-1, descend
//use these to set new values
/*
int rTarget = 20; 
int gTarget = 0; 
int bTarget = 0;
int rCurrent = 0;
int gCurrent = 0;
int bCurrent = 0;
*/

void initSensor(void) {
  Adafruit_BNO055 bno = Adafruit_BNO055(55);
  delay(1000);
  if(!bno.begin()) {
    //Serial.println("BNO-055 broke.");
    while(1);
  }
  
  //delay(1000);
  bno.setExtCrystalUse(true);  
}

void setupWifi(){
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    //Serial.print(".");
  }
  //Serial.println("WiFi connected");  
}

void initNeo(){  
  //Serial.println("Pixels Begin");
  pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

  pixels.begin();
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(5,5,5)); 
  }
  pixels.show(); 
  delay(10);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); 
  }
  pixels.show(); 
  //Serial.println("Pixels Initialized");
}

float difference(float a, float b){
  if(a>b)
    return (abs(a)-abs(b));
  else if(a<b)
    return (b-a);
  else
    return 0;
}

uint32_t setTransitionColor(int rTarget, int gTarget, int bTarget){
  
}

void simpleNeoUpdate(float magnitude){
  if(magnitude > 20) {
      for(int j=0; j<NUMPIXELS; j++){
        pixels.setPixelColor(j,0,10,10);
      }
      pixels.show();
  }else{
   
      for(int j=0; j<NUMPIXELS; j++){
        pixels.setPixelColor(j,0,0,0);
      }
      pixels.show();
  }
}

void delayAndDisplay(float magnitude, int delayTime, int steps){
  int r=0;
  int g=0;
  int b=0;
  
  if(magnitude > 20) {
    Serial.println("change color");
    int timeSteps = delayTime/steps;
    int rTarget = 0;
    int gTarget = 0;
    int bTarget = 20;
    for(int i=0; i<steps; i++){
      r += rTarget/steps;
      g += gTarget/steps;
      b += bTarget/steps;
      for(int j=0; j<NUMPIXELS; j++){
        pixels.setPixelColor(j,r,g,b);
      }
      pixels.show();
      delay(timeSteps);
    }
  } else{
    int timeSteps = delayTime/steps;
    int rTarget = 0;
    int gTarget = 0;
    int bTarget = 0;
    for(int i=0; i<steps; i++){
      r += rTarget/steps;
      g += gTarget/steps;
      b += bTarget/steps;
      for(int j=0; j<NUMPIXELS; j++){
        pixels.setPixelColor(j,r,g,b);
      }
      pixels.show();
      delay(timeSteps);
    }
  }
}


void setup() {  
  Serial.begin(115200);     delay(100);
  Serial.println("Here we go!");
  setupWifi();//     delay(500);
  initSensor();//    delay(500);
  //initNeo();     delay(500);
}

void loop() {
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float x,y,z;
  x = euler.x();
  y = euler.y();
  z = euler.z();  
  float magnitude = euler.x()*euler.x() + euler.y()*euler.y() + euler.z()*euler.z();
  magnitude = sqrt(magnitude);
  float diff = difference(magnitude, prevMag);
  prevMag = magnitude;

  //Serial.println(magnitude);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    //Serial.println("connection failed");
    return;
  }
  
  String url = "/data?";
  url += "skipit=" + skipITID; //skipit id
  url += "&magnitude=";
  url += magnitude;

  //Serial.print("Requesting URL: ");
  //Serial.println(url);  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
  // Read all the lines of the reply from server and print them to Serial
  /*
  while(client.available()){
    String line = client.readStringUntil('\r');
    //Serial.print(line);
  }*/
  

  //commented this out because the delay from transmission should suffice
  delay(BNO055_SAMPLERATE_DELAY_MS);


  //simpleNeoUpdate(magnitude);
  //Used to smooth led transition during acceleration moments, also serves as the delay for the sample rate for the timer
  //delayAndDisplay(magnitude, BNO055_SAMPLERATE_DELAY_MS, 10);
}


