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

int error_count=0;
bool same_error = false;


const int httpPort = 8080;
const char* ssid     = "skipit";
const char* password = "flowfactory";
const char* host = "10.0.0.6";

#define BNO055_SAMPLERATE_DELAY_MS (50)
Adafruit_BNO055 bno;

int lastSendTimeStamp;
int lastNeoUpdateTimeStamp;
int sendDelay = BNO055_SAMPLERATE_DELAY_MS;
int neoUpdateDelay = 100;

float currentMag;
int lightDuration;
int lightMax = 20;



#define NEO_PIN        15
#define NUMPIXELS      32
Adafruit_NeoPixel pixels  = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);




void initSensor(void) {
  //Serial.println("BNO-055 Begin");
  Adafruit_BNO055 bno = Adafruit_BNO055(55);
  delay(1000);
  if(!bno.begin()) {
    //Serial.println("BNO-055 broke.");
    while(1);
  }
  bno.setExtCrystalUse(true);  
  //Serial.println("BNO-055 End");
}


void initNeo(){  
  //Serial.println("Pixels Begin");
  //pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

  lightDuration = 0;
  
  pixels.begin();
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(50,0,0)); 
  }
  pixels.show(); 
  delay(20);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); 
  }
  pixels.show(); 
  //Serial.println("Pixels Initialized");
}

void setupWifi(){
  //Serial.println("WiFi Begin");
  WiFi.begin(ssid, password);
  //Serial.println("WiFi Begin 2");
  int cnt = 0;
  //Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(50);
    /*if(cnt++ > 20){
      WiFi.begin(ssid, password);
      cnt=0;
    }*/
  }
  //Serial.println("wifi connected");
}

void setup() {  
  //Serial.begin(115200);     delay(100);
  //Serial.println();
  //Serial.println();
  //Serial.println("Here we go!");
  
  initNeo();

  initSensor();//    delay(500);
  //setupWifi();//     delay(500);
  
  lastSendTimeStamp = millis();
  lastNeoUpdateTimeStamp = millis();
  currentMag = 0;
}

float getMag(){
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float magnitude = euler.x()*euler.x() + euler.y()*euler.y() + euler.z()*euler.z();
  currentMag = sqrt(magnitude); 
  return currentMag;
}

void simpleNeoUpdate(float magnitude){
  if(magnitude > 20) {
    lightDuration++;
  }else{
    lightDuration--;
  }
  if(lightDuration > lightMax){
    lightDuration = lightMax;
  }
  if(lightDuration < 0){
    lightDuration = 0;
  }
  
  for(int j=0; j<NUMPIXELS; j++){
    pixels.setPixelColor(j,0,2*lightDuration,2*lightDuration);
  }
  pixels.show();
}

void loop() {
  if(millis()-lastSendTimeStamp > sendDelay) {
    float magnitude = getMag();

    simpleNeoUpdate(magnitude);
    // Use WiFiClient class to create TCP connections
    
  lastSendTimeStamp = millis();   
    
  }
}


