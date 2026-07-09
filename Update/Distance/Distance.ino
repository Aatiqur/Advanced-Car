// Distance.ino — main sketch (encoders + raw MPU)
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

WebServer server(80);

// ---------- Pin Assignment ----------
// Left encoder        : A=32, B=33
// Right encoder       : A=25, B=26
// L298N               : IN1=16, IN2=17 (left), IN3=18, IN4=19 (right), ENA=27, ENB=14
// MPU6050 I2C         : SDA=21, SCL=22
#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 19
#define ENA 27
#define ENB 14

#define LEFT_ENCODER_A 32
#define LEFT_ENCODER_B 33
#define RIGHT_ENCODER_A 25
#define RIGHT_ENCODER_B 26

// ---------- MPU6050 ----------
#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

// ---------- WiFi ----------
const char* WIFI_SSID = "S308 ";
const char* WIFI_PASSWORD = "s308@1234";
const char* AP_NAME = "AdvancedCar_Distance";

// ---------- Drive setup ----------
const float ENCODER_CPR = 374.0f;
const float WHEEL_DIAMETER_M = 0.065f;
const float WHEEL_CIRCUMFERENCE_M = PI * WHEEL_DIAMETER_M;
const float DISTANCE_PER_TICK_M = WHEEL_CIRCUMFERENCE_M / ENCODER_CPR;
const float TARGET_DISTANCE_M = 1.0f;
const int DRIVE_PWM = 180;
const int LEFT_PWM_TRIM = 18;
const int PWM_FREQ = 1000;
const int PWM_RESOLUTION = 8;

// ---------- MPU scaling ----------
const float ACCEL_SCALE = 16384.0f; // LSB/g at +/-2g
const float GYRO_SCALE = 131.0f;    // LSB/(deg/s) at +/-250 deg/s
const float GRAVITY = 9.80665f;

// ---------- Simple motion state (raw MPU only) ----------
struct MotionState {
  float rollDeg;
  float pitchDeg;
  float yawDeg;
  float velocityMps;
  float distanceM;
  float accelMagnitudeMps2;
  float rollOffsetDeg;
  float pitchOffsetDeg;
};

#define DISTANCE_SHARED_TYPES 1

// ---------- Globals ----------
volatile long leftEncoderTicks = 0;
volatile long rightEncoderTicks = 0;

bool autoRunActive = false;
unsigned long motionStartMillis = 0;

float gyroXBiasDps = 0.0f;
float gyroYBiasDps = 0.0f;
float gyroZBiasDps = 0.0f;

MotionState motion;

unsigned long lastImuMicros = 0;
unsigned long lastStatusMillis = 0;

// ---------- Forward declarations ----------
void readLeftEncoder();
void readRightEncoder();
void resetMotionEstimates();
float readAverageEncoderDistanceM(long &leftTicksOut, long &rightTicksOut);

void beginForwardOneMeterRun() {
  resetEncoderTicks();
  resetMotionEstimates();
  driveForward();
  autoRunActive = true;
  motionStartMillis = millis();
}

void stopAutoRun() {
  stopMotors();
  autoRunActive = false;
}

void attachEncoderInterrupts() {
  attachInterrupt(digitalPinToInterrupt(LEFT_ENCODER_A), readLeftEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODER_A), readRightEncoder, RISING);
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_ENCODER_A, INPUT_PULLUP);
  pinMode(LEFT_ENCODER_B, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_A, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_B, INPUT_PULLUP);
  attachEncoderInterrupts();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(ENB, PWM_FREQ, PWM_RESOLUTION);

  stopMotors();

  mpuInitialize();
  motion = {};
  calibrateMpuBias();

  connectWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/start", HTTP_POST, handleStart);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/reset", HTTP_POST, handleReset);
  server.on("/api/calibrate", HTTP_POST, handleCalibrate);
  server.begin();

  lastImuMicros = micros();

  Serial.println("Distance web server ready.");
  Serial.println("Open the IP shown above in a browser.");
}

void loop() {
  server.handleClient();

  long leftTicks = 0;
  long rightTicks = 0;
  float encoderDistance = readAverageEncoderDistanceM(leftTicks, rightTicks);

  updateImuEstimates();

  if (autoRunActive && encoderDistance >= TARGET_DISTANCE_M) {
    stopAutoRun();
  }

  if (millis() - lastStatusMillis >= 1000) {
    lastStatusMillis = millis();

    Serial.print("Run=");
    Serial.print(autoRunActive ? "ON" : "OFF");
    Serial.print(" Enc=");
    Serial.print(encoderDistance, 3);
    Serial.print("m Roll=");
    Serial.print(motion.rollDeg, 1);
    Serial.print(" Pitch=");
    Serial.print(motion.pitchDeg, 1);
    Serial.print(" Yaw=");
    Serial.print(motion.yawDeg, 1);
    Serial.print(" ImuDist=");
    Serial.print(motion.distanceM, 3);
    Serial.print("m ImuVel=");
    Serial.print(motion.velocityMps, 3);
    Serial.println("m/s");
  }
}