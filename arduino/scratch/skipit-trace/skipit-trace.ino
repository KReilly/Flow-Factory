#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#if not defined (_VARIANT_ARDUINO_DUE_X_)
  #include <SoftwareSerial.h>
#endif
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

String skipITID = "1";


const char* ssid     = "skipit";
const char* password = "flowfactory";
const char* host = "10.0.0.6";


#define BNO055_SAMPLERATE_DELAY_MS (100)
Adafruit_BNO055 bno;

#define NEO_PIN        15
#define NUMPIXELS      32
Adafruit_NeoPixel pixels  = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

float currentMag = 0.0;

int lightDuration = 0;
int lightMax = 20;
bool isActive = true;

int targetColorIndex = 0;
int currColorIndex = 0;
int maxBrightness = 20;
int colors[5][3] = {
  { 240, 0, 255 },
  { 0, 211, 255 },
  { 218, 38, 255 },
  { 101, 70, 255 },
  { 255, 2, 218 }
};
int colorsScaled[5][3];
int timeColorBegin = 0;
int colorDuration = 5000;
int fadeCount = 0;
//int fadeDuration = 200;
int pixelStep = 0;


int lightsTarget[NUMPIXELS][3];
int lights[NUMPIXELS][3];
int brightness = 0;


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);    

  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  //Setup BNO
  Serial.println("BNO-055 Begin");
  Adafruit_BNO055 bno = Adafruit_BNO055(55);
  delay(1000);
  if(!bno.begin()) {
    Serial.println("BNO-055 broke.");
    while(1);
  }
  bno.setExtCrystalUse(true);  
  Serial.println("BNO-055 End");

  //Setup NeoPixels
  initNeo();


  //Setup Wifi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  if(isActive == false){
    Serial.println("State Inactive!");
    WiFi.disconnect(true); 

  }else{
      Serial.println("State Active");
  }
}


void loop()
{
  if(isActive){
    float magnitude = getMag();
    simpleNeoUpdate(magnitude);
  
    WiFiClient client;
    //Serial.printf("\n[Connecting to %s ... ", host);
    if (client.connect(host, 8080))
    {
      //Serial.println("connected]");
      //Serial.println("[Sending a request]");
  
      String url = "/data?";
      url += "skipit=" + skipITID; //skipit id
      url += "&magnitude=";
      url += magnitude;
      
      client.print(String("GET ") + url+ " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n" +
                   "\r\n"
                  );
  
      //Serial.println("[Response:]");
      
      isActive = true;
      while (client.connected())
      {
        if (client.available())
        {
          String line = client.readStringUntil('\n');
          if(line.indexOf("assigned") != -1){
            isActive = true;
            break;
          }else if(line.indexOf("sleep") != -1 && line.length()==6){ //stupid hack to ensure errors sent on crashes of the code does not match!
            isActive = false;
            Serial.println("'Sleep' command matched!");
            break;
          }
          //Serial.println(line);
        }
      }
      if(client.connected())
        client.stop();
      //Serial.println("[Disconnected]");  
      delay(BNO055_SAMPLERATE_DELAY_MS);
      
      if(isActive == false){
        Serial.println("State Inactive!");
        WiFi.disconnect(true); 

      }else{
        //Serial.println("State Active");
      }
      
    }
    else
    {
      Serial.println("connection failed!!]");
      client.stop();
    }
  }else{
    //sleep
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);                   
    digitalWrite(LED_BUILTIN, HIGH);
    for(int i=0; i<NUMPIXELS; i++){
      pixels.setPixelColor(i, 0,0,0);
    }
    pixels.show();
    delay(3000);
  }
}



void initNeo(){  
  Serial.println("Pixels Begin");

  //blink red to begin
  pixels.begin();
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(50,0,0)); 
  }
  pixels.show(); 
  delay(20);
  for(int i=0;i<NUMPIXELS;i++){
    lights[i][0] = 0;
    lights[i][1] = 0;
    lights[i][2] = 0;
    pixels.setPixelColor(i, pixels.Color(lights[i][0],lights[i][1],lights[i][2])); 
  }
  pixels.show(); 
  Serial.println("Pixels Initialized");

  
  currColorIndex = random(5);
  for(int i=0; i<NUMPIXELS; i++){
      lightsTarget[i][0] = colors[currColorIndex][0];    
      lightsTarget[i][1] = colors[currColorIndex][1];
      lightsTarget[i][2] = colors[currColorIndex][2];
  }


  scaleColors();
}

void scaleColors(){
  for(int i=0; i< 5; i++){
    for(int j=0; j<3; j++){
      colorsScaled[i][j] = colors[i][j] * maxBrightness/255;
    }
  }
}

void simpleNeoUpdate(float magnitude){
  //change color target
  if(millis()-timeColorBegin > colorDuration){
    timeColorBegin = millis();
    currColorIndex = random(5);
    for(int i=0; i<NUMPIXELS; i++){
      for(int j=0; j<3; j++){
        lightsTarget[i][j] = colors[currColorIndex][j];
      }
    }
  }

  for(int i=0; i<NUMPIXELS; i++){
    for(int j=0; j<3; j++){
/*      if(lights[i][j] > lightsTarget[i][j])
        lights[i][j]--;
      else if(lights[i][j] < lightsTarget[i][j])
        lights[i][j]++;
*/
      lights[i][j]=lightsTarget[i][j];
    }
  }


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
  
  for(int i=0; i<NUMPIXELS; i++){
    int r = lights[i][0] * lightDuration/255;
    int g = lights[i][1] * lightDuration/255;
    int b = lights[i][2] * lightDuration/255;
    pixels.setPixelColor(i,r,g,b);
  }
  
  pixels.show();
}


float getMag(){
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  float magnitude = euler.x()*euler.x() + euler.y()*euler.y() + euler.z()*euler.z();
  currentMag = sqrt(magnitude);
  return currentMag;
}



