#include <Servo.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#define HC06 Serial1

//Pins:
#define LED_PIN 13
#define ServoPin1 3
#define ServoPin2 4
#define ServoPin3 5
#define ServoPin4 6
#define ServoArmPin 7
#define SWVLPin 4
#define SWVRPin 0
#define SWM1Pin 1
#define SWM2Pin 2
#define SWM3Pin 3
#define Ultra1TriggerPin 10
#define Ultra1EchoPin 11
#define Ultra2TriggerPin 12
#define Ultra2EchoPin 14

//Konstanten:
#define ABWEICHUNG_MIN 1
#define ABWEICHUNG_MAX 20
#define CORRECT_SPEED_LEFT 25
#define CORRECT_SPEED_RIGHT 25
#define ZERO_SPEED 94
#define ARM_UP_POS 180
#define ARM_DOWN_POS 83
#define SW_PUFFER 60    //vorher 50 (nacht) oder 70 (tag)
#define ULTRA_CALIBRATE 0.03432
#define SEND_DELAY 100
#define CROSS_DURATION 150  //normal 200
#define SEARCH_DURATION 60

//#define CORRECT_SPEED 15

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servoArm;

bool blinkState = true;

float mpuPitch = 0;
float mpuRoll = 0;
float mpuYaw = 0;

MPU6050 mpu;

uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

#define PITCH   1     // defines the position within ypr[x] variable for PITCH; may vary due to sensor orientation when mounted
#define ROLL  2     // defines the position within ypr[x] variable for ROLL; may vary due to sensor orientation when mounted
#define YAW   0     // defines the position within ypr[x] variable for YAW; may vary due to sensor orientation when mounted

float winkel;
float winkelOld;
float calValue;
float calNumber;
float calNumberOld;
bool isCalibrate;
bool armUp = true;
bool debugMode = false;
unsigned long timeMil;
int disFront;
int disBack;
boolean straight = false;
boolean oldStraight = false;
uint8_t crossDirs[12] = {3, 3, 5, 0, 0, 0, 0, 0 , 0, 0, 0}; //0 = gerade, 1 = links abbiegen, 2 = rechts abbiegen
int crossCounter = 0;
boolean inCrossing = false;
int crossTime = 0;
boolean searchingLine = false;
int searchTime = 10;
int searchCount = 0;
int whiteTime;
boolean waiting;
long timeCount;
boolean ending;

int SWML_Cal;
int SWMM_Cal;
int SWMR_Cal;
int SWVL_Cal;
int SWVR_Cal;
bool calibratedSW = false;
int calibrationCounter = 0;
bool SWinCal = false;

float thisWinkel;
int statusCar = 0;
boolean followLine = false;
boolean curveNext = false;
long curveTime = 50;

int bleDat[5];
int bleSpeed;
int bleCurve;
int counter;
byte chByte = 0;
String strInput;
String strOut;

void setup()
{
  delay(1000);
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  strInput.reserve(4);    //allocate a buffer in Arduino -> very small RAM
  strOut.reserve(16);

  Serial.begin(9600);
  HC06.begin(9600);
  while (!Serial);
  mpu.initialize();
  devStatus = mpu.dmpInitialize();

  // INPUT CALIBRATED OFFSETS HERE; SPECIFIC FOR EACH UNIT AND EACH MOUNTING CONFIGURATION!!!!
  mpu.setXGyroOffset(118);
  mpu.setYGyroOffset(-44);
  mpu.setZGyroOffset(337);
  mpu.setXAccelOffset(-651);
  mpu.setYAccelOffset(670);
  mpu.setZAccelOffset(1895);

  if (devStatus == 0)
  {
    mpu.setDMPEnabled(true);
    mpuIntStatus = mpu.getIntStatus();
    packetSize = mpu.dmpGetFIFOPacketSize();
  }
  else
  {
    // ERROR!
    // 1 = initial memory load failed, 2 = DMP configuration updates failed (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed code = "));
    Serial.println(devStatus);
  }
  pinMode(LED_PIN, OUTPUT);
  pinMode(Ultra1TriggerPin, OUTPUT);
  pinMode(Ultra1EchoPin, INPUT);
  pinMode(Ultra2TriggerPin, OUTPUT);
  pinMode(Ultra2EchoPin, INPUT);

  Serial.println("Bitte Warten - Kalibrierung");
}


