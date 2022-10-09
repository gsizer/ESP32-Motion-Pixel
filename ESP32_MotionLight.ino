/////////////////////////////////////////////////////////
// configuration
#include <Preferences.h>
Preferences prefs;

// colors
typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t lux;
} color_t;

const color_t BLACK = {0, 0, 0, 0};
const color_t PURPLE = {128, 0, 128, 255};
const color_t RED = {255, 0, 0, 255};
const color_t ORANGE = {255, 128, 0, 255};
const color_t YELLOW = {255, 255, 0, 255};
const color_t GREEN = {0, 255, 0, 255};
const color_t CYAN = {0, 255, 255, 255};
const color_t BLUE = {0, 0, 255, 255};
const color_t MAGENTA = {255, 0, 255, 255};
const color_t WHITE = {255, 255, 255, 255};

// pose ( combination of acceleration and rotation within a given error value )
typedef struct {
  char Name[16];
  float RotX;
  float RotY;
  float RotZ;
  float AccX;
  float AccY;
  float AccZ;
  float ErrM;
  color_t RGBL;
} pose_t;

// default entry
pose_t *poseDefault = {};
size_t poseLen = 0;
int poseCount = 0;

/////////////////////////////////////////////////////////
// mpu6050
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
Adafruit_MPU6050 mpu;
sensors_event_t accelerometer, gyroscope, temperature;

/////////////////////////////////////////////////////////
// NeoPixel
#define powerPin A1
#include <Adafruit_NeoPixel.h>
#define pixelPin A0
#define pixelCount 1
Adafruit_NeoPixel pixel(pixelCount, pixelPin, NEO_GRB + NEO_KHZ800);

/////////////////////////////////////////////////////////
// web server
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
const char *ssid = "printer";
const char *password = "lostinspace";
WebServer server(80);

void handleRoot(){
  char temp[400];
  snprintf(
    temp,
    400,
    "<html><head><meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Motion Light</title></head>\
    <body background-color=#880088 font-color=#cccccc>\
    <ul name='Rotation'><li>X:%02d<\li><li>Y:%02d<\li><li>Z:%02d<\li></ul>\
    <ul name='Acceleration'><li>X:%02d<\li><li>Y:%02d<\li><li>Z:%02d<\li></ul>\
    </body></html>",
    gyroscope.gyro.x,
    gyroscope.gyro.y,
    gyroscope.gyro.z,
    accelerometer.acceleration.x,
    accelerometer.acceleration.y,
    accelerometer.acceleration.z
  );
  server.send(200, "text/html", temp);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/////////////////////////////////////////////////////////
// initialization
void setup() {
  // Preferences
  prefs.begin("pose"); // use pose namespace
  prefs.putBytes("pose", &poseDefault, sizeof(poseDefault));
  poseLen = prefs.getBytesLength("pose");
  char buff[poseLen];
  prefs.getBytes("pose", buff, poseLen);
  // does the data fit the structure
  if(poseLen % sizeof(pose_t)){ // if(!0)...
    // invalid data
    return;
  }
  poseDefault = (pose_t *) buff; // cast buffer to struct ptr
  // MPU6050
  if(mpu.begin()){
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }
  // NeoPixel
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
  pixel.begin(); // initialize object
  pixel.clear(); // set pixel off
  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // wait for connection
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  // multicast responder
  MDNS.begin("esp32");
  // url handler assignment
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

/////////////////////////////////////////////////////////
// toggle given pin
void toggle(int pin){
  pinMode(pin, OUTPUT); // set pin to output
  digitalWrite(pin, !digitalRead(pin)); // invert current pin value
}

/////////////////////////////////////////////////////////
// rainbow loop
void rainbow(int wait = 10){
  // begin at red
  color_t rgbl = { 255, 0, 0, 255 };
  pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
  pixel.setBrightness(rgbl.lux);  
  pixel.show();
  delay(wait*10);
  // increase green to yellow
  for( rgbl.green = 0; rgbl.green < 255; rgbl.green++ ){
    pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
    pixel.show();
    delay(wait);
  }
  // decrease red to green
  for( rgbl.red = 255; rgbl.red > 0; rgbl.red-- ){
    pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
    pixel.show();
    delay(wait);
  }
  // increase blue to cyan
  for( rgbl.blue = 0; rgbl.blue < 255; rgbl.blue++ ){
    pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
    pixel.show();
    delay(wait);
  }
  // decrease green to blue
  for( rgbl.green = 255; rgbl.green > 0; rgbl.green-- ){
    pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
    pixel.show();
    delay(wait);
  }
  // increase red to magenta
  for( rgbl.red = 0; rgbl.red < 255; rgbl.red++ ){
    pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
    pixel.show();
    delay(wait);
  }
  // decrease blue to red
  for( rgbl.blue = 255; rgbl.blue > 0; rgbl.blue-- ){
    pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
    pixel.show();
    delay(wait);
  }
}

/////////////////////////////////////////////////////////
// loop
void loop() {
  // put your main code here, to run repeatedly:
  // Get new sensor events with the readings
  mpu.getEvent(&accelerometer, &gyroscope, &temperature);
  server.handleClient();
  rainbow(10);
  delay(2);
}
