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

#define NEO_PIN        15
#define NUMPIXELS      32
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);


String skipITID = "0";

const int httpPort = 8080;
const char* ssid     = "skipit";
const char* password = "flowfactory";
const char* host = "10.0.0.6";


#define BNO055_SAMPLERATE_DELAY_MS (50)
Adafruit_BNO055 bno;

float prevMag = 0;
int color_shift = 0;
int rise = 1; // ==1 => ascend, ==-1, descend

void initSensor(void) {
  Adafruit_BNO055 bno = Adafruit_BNO055(55);
  delay(1000);
  //Serial.println("Init Sensor");
  if(!bno.begin()) {
    Serial.println("BNO-055 broke.");
    while(1);
  }
  
  delay(1000);
  bno.setExtCrystalUse(true);
  //Serial.println("BNO-055 setup.");
  delay(1000);
  
}

void setupWifi(){
  /* Serial.print("Connecting to: ");
  Serial.print(ssid);
  Serial.println();
  Serial.print("Password: ");
  Serial.print(password);
  Serial.println();
     */ 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println("WiFi connected");  
  Serial.println(WiFi.localIP());
}


void initNeo(){  
  //Serial.println("Pixels Begin");

  pixels.begin();
  delay(500);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); 
    
  }
  pixels.show(); 
  delay(30);
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

int brightnesScaling(float magnitude){
  int m = float(magnitude);
  int OldMax = 60;
  int OldMin = 10;
  int NewMax = 1000;
  int NewMin = 1;

  if(m > OldMax)
      m = OldMax;
  if(m < OldMin)
      m = OldMin;
  
  int OldRange = (OldMax - OldMin);  
  int NewRange = (NewMax - NewMin);
  
  //return (1000-(((m - OldMin) * NewRange) / OldRange) + NewMin);
  return (((m - OldMin) * NewRange) / OldRange) + NewMin;
}



void setup() {  
  Serial.begin(115200);     delay(100);
  
  Serial.println("Here we go!");
  setupWifi();     delay(500);
  initSensor();    delay(500);
  
  initNeo();     delay(500);
  color_shift = 0;
  
  //pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
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

  Serial.println(magnitude);
  //Serial.println(diff);
  //Serial.println(color_shift);

  //Serial.print(euler.x());

  //updateNeo(magnitude, diff);
  //delay(500);   
  
  //Serial.print("connecting to ");
  //Serial.println(host);
  
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



  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  //Serial.println();
  //Serial.println("closing connection");
  
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
    
