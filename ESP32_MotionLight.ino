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
pose_t *poseDefault;
size_t poseLen = 0;
int poseCount = 0;

/////////////////////////////////////////////////////////
// QTPy LiPo BFF Backpack
#define BATT_MON A0
float vbat = 0.0;

/////////////////////////////////////////////////////////
// LSM6DSOX
#include <Adafruit_LSM6DSOX.h>
Adafruit_LSM6DSOX imu;
sensors_event_t accel, gyro, tempC;
int imuErr = 0;

/////////////////////////////////////////////////////////
// NeoPixel
#include <Adafruit_NeoPixel.h>
#define powerPin A1
#define pixelPin A2
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
  char temp[512];
  snprintf(
    temp,
    512, // i'm too lazy to count should be good.
    "<html><head><meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Motion Light</title></head>\
    <body background-color=#880088 font-color=#cccccc>\
    <ul name='Rotation'><li>X:%02d<\li><li>Y:%02d<\li><li>Z:%02d<\li></ul>\
    <ul name='Acceleration'><li>X:%02d<\li><li>Y:%02d<\li><li>Z:%02d<\li></ul>\
    </body>%02d</html>",
    gyro.gyro.x,
    gyro.gyro.y,
    gyro.gyro.z,
    accel.acceleration.x,
    accel.acceleration.y,
    accel.acceleration.z,
    tempC.temperature
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
  
  // LSM6DSOX
  imuErr = imu.begin_I2C();
  imu.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  imu.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
  imu.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
  imu.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);
  
  // NeoPixel
  pinMode(powerPin, OUTPUT); // set pin to OUTPUT
  digitalWrite(powerPin, HIGH); // power up
  pixel.begin(); // initialize object
  pixel.clear(); // set pixel off
  
  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // wait for connection
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  
  // Multicast Responder
  MDNS.begin("esp32");
  // url handler assignment
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

/////////////////////////////////////////////////////////
// rainbow loop
void rainbow(int wait = 10){
  // begin at red
  color_t rgbl = RED;
  pixel.setPixelColor(0, pixel.Color(rgbl.red, rgbl.green, rgbl.blue));
  // fade in
  for(int i=0; i<rgbl.lux; i++){
    pixel.setBrightness(i);  
    pixel.show();
  }
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
    // fade out
  for(int i=rgbl.lux; i>0; i--){
    pixel.setBrightness(i);  
    pixel.show();
  }
}

/////////////////////////////////////////////////////////
// loop
void loop() {
  // check battery level
  vbat = analogRead(BATT_MON);
  // Get new sensor readings
  imu.getEvent(&accel, &gyro, &tempC);
  // process clients
  server.handleClient();
  // light up pixel
  rainbow(10);
  delay(2);
}
