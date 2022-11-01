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

enum { BLACK=0, RED=1, ORANGE=2, YELLOW=3, GREEN=4, CYAN=5, BLUE=6, PURPLE=7, MAGENTA=8, WHITE=9 };

const color_t Colors[10] = {
  {0, 0, 0, 0},
  {255, 0, 0, 255},
  {255, 128, 0, 255},
  {255, 255, 0, 255},
  {0, 255, 0, 255},
  {0, 255, 255, 255},
  {0, 0, 255, 255},
  {128, 0, 128, 255},
  {255, 0, 255, 255},
  {255, 255, 255, 255}
};

// pose ( combination of acceleration and rotation )
typedef struct {
  char Name[4]; // 4 bytes
  float RotX; // 4 bytes
  float RotY;
  float RotZ;
  float AccX;
  float AccY;
  float AccZ;
  color_t RGBL; // 4 bytes
} pose_t; // 32 bytes

// default entry
pose_t *poseDefault;
size_t poseLen = 0;
int poseCount = 0;
pose_t *poses[10];

// write default entry
bool PrefsDefaults(){
  prefs.begin("pose"); // use pose namespace
  prefs.putBytes("pose", &poses, sizeof(poses)); // 10 entries
  poseLen = prefs.getBytesLength("pose");
  char buff[poseLen];
  prefs.getBytes("pose", &buff, poseLen);
  // does the data fit the structure
  if(poseLen % sizeof(pose_t)){ // if(!0)...
    // invalid data
    return false;
  }
  poseDefault = (pose_t *) buff; // cast buffer to struct ptr
  return true;
}

/////////////////////////////////////////////////////////
// QTPy LiPo BFF Backpack
#define BATT_MON A0
float vBatt = 0.0;

float fmap(float in, float from_low, float from_high, float to_low, float to_high){
  return (in - from_low) * (to_high - to_low) / (from_high - from_low) + to_low;
}

color_t readBattery(){
  vBatt = analogRead(BATT_MON);
  vBatt = fmap(vBatt, 0, 1023, 0, 3.7);
  color_t iBatt = Colors[PURPLE];
  iBatt = (vBatt>=80 && vBatt<=100) ? Colors[GREEN] : iBatt;
  iBatt = (vBatt>=40 && vBatt<=80) ? Colors[YELLOW] : iBatt;
  iBatt = (vBatt>=20 && vBatt<=40) ? Colors[RED] : iBatt;
}

/////////////////////////////////////////////////////////
// LSM6DSOX
#include <Adafruit_LSM6DSOX.h>
Adafruit_LSM6DSOX imu;
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t tempC;
bool imuErr = false;

/////////////////////////////////////////////////////////
// NeoPixel
#include <Adafruit_NeoPixel.h>
// onboard neopixel is PIN_NEOPIXEL and NEOPIXEL_POWER
#define powerPin NEOPIXEL_POWER
#define pixelPin PIN_NEOPIXEL
#define pixelCount 1
Adafruit_NeoPixel pixel(pixelCount, pixelPin, NEO_GRB + NEO_KHZ800);
color_t RGBL = Colors[WHITE];

// fade to off
void fadeOut(){
  for(int i=255; i>0; i--){
    pixel.setBrightness(i);
    pixel.show();
    delay(10);
  }
}

//fade to full bright
void fadeIn(){
  for(int i=0; i<255; i++){
    pixel.setBrightness(i);
    pixel.show();
    delay(10);
  }
}

// cross fade color and brightness
void crossfade(color_t target){
  pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
  pixel.show();
  // increment to target color
  for(int r=RGBL.red; r<target.red; r++){
    RGBL.red=r;
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
  }
  for(int g=RGBL.green; g<target.green; g++){
    RGBL.green=g;
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
  }
  for(int b=RGBL.blue; b<target.blue; b++){
    RGBL.blue=b;
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
  }
  for(int r=RGBL.red; r>target.red; r--){
    RGBL.red=r;
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
  }
  for(int g=RGBL.green; g>target.green; g--){
    RGBL.green=g;
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
  }
  for(int b=RGBL.blue; b>target.blue; b--){
    RGBL.blue=b;
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
  }
  // update hardware
  delay(10);
}

// rainbow
void rainbow(int wait=10){
  // begin at red
  pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
  fadeIn();
  // increase green to yellow
  for( RGBL.green = 0; RGBL.green < 255; RGBL.green++ ){
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
    delay(wait);
  }
  // decrease red to green
  for( RGBL.red = 255; RGBL.red > 0; RGBL.red-- ){
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
    delay(wait);
  }
  // increase blue to cyan
  for( RGBL.blue = 0; RGBL.blue < 255; RGBL.blue++ ){
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
    delay(wait);
  }
  // decrease green to blue
  for( RGBL.green = 255; RGBL.green > 0; RGBL.green-- ){
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
    delay(wait);
  }
  // increase red to magenta
  for( RGBL.red = 0; RGBL.red < 255; RGBL.red++ ){
    pixel.setPixelColor(0, pixel.Color(RGBL.red, RGBL.green, RGBL.blue));
    pixel.show();
    delay(wait);
  }
  // decrease blue and red to purple
  for( int i = 255; i > 123; i-- ){
    pixel.setPixelColor(0, pixel.Color(i, RGBL.green, i));
    pixel.show();
    delay(wait);
  }
  fadeOut();
}

/////////////////////////////////////////////////////////
// initialization
void setup() {
  Serial.begin(115200);
  // Preferences
  if(PrefsDefaults()){
      // defaults written
  }

  // LSM6DSOX
  imuErr = imu.begin_I2C();
  if(imuErr==OK){
    imu.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
    imu.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
    imu.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
    imu.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);
  }

  // NeoPixel
  pinMode(powerPin, OUTPUT); // set pin to OUTPUT
  digitalWrite(powerPin, HIGH); // power up
  pixel.begin(); // initialize object
  pixel.clear(); // set pixel off
  pixel.setBrightness(100); // 100/0-255=~40% brightness
}

/////////////////////////////////////////////////////////
// loop
void loop() {
  // Get new sensor readings
  imu.getEvent(&accel, &gyro, &tempC);
  fadeIn();
  delay(100);
  crossfade(readBattery());
  fadeOut();
  delay(100);
  rainbow();
  delay(100);
}
