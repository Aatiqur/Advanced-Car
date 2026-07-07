#include <Arduino.h>
#include <EEPROM.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

WebServer server(80);

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

#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

const char* WIFI_SSID = "S308 ";
const char* WIFI_PASSWORD = "s308@1234";
const char* AP_NAME = "AdvancedCar_Distance";

const float ENCODER_CPR = 374.0f;
const float WHEEL_DIAMETER_M = 0.065f;
const float WHEEL_CIRCUMFERENCE_M = PI * WHEEL_DIAMETER_M;
const float DISTANCE_PER_TICK_M = WHEEL_CIRCUMFERENCE_M / ENCODER_CPR;
const float TARGET_DISTANCE_M = 1.0f;
const int DRIVE_PWM = 180;
const int LEFT_PWM_TRIM = 18;
const int PWM_FREQ = 1000;
const int PWM_RESOLUTION = 8;

const float ACCEL_SCALE = 16384.0f;
const float GYRO_SCALE = 131.0f;
const float GRAVITY = 9.80665f;
const uint32_t EEPROM_MAGIC = 0x4B414C31;
const int EEPROM_SIZE = 128;

struct KalmanAxisState {
  float x[2];
  float P[2][2];
};

struct KalmanSettings {
  uint32_t magic;
  float qAngle;
  float qBias;
  float rMeasure;
  float rollOffsetDeg;
  float pitchOffsetDeg;
  float stillAccelThreshold;
  float stillVelocityThreshold;
  float velocityDecay;
  float gyroStillThreshold;
};

struct MotionState {
  float rollDeg;
  float pitchDeg;
  float yawDeg;
  float velocityMps;
  float distanceM;
  float accelMagnitudeMps2;
};

#define DISTANCE_SHARED_TYPES 1

volatile long leftEncoderTicks = 0;
volatile long rightEncoderTicks = 0;

bool autoRunActive = false;
unsigned long motionStartMillis = 0;

float accelXBiasG = 0.0f;
float accelYBiasG = 0.0f;
float accelZBiasG = 0.0f;
float gyroXBiasDps = 0.0f;
float gyroYBiasDps = 0.0f;
float gyroZBiasDps = 0.0f;

KalmanSettings kalmanSettings;
KalmanAxisState rollKalmanState;
KalmanAxisState pitchKalmanState;
MotionState rawMotion;
MotionState kalmanMotion;

unsigned long lastImuMicros = 0;
unsigned long lastStatusMillis = 0;

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
  loadKalmanSettings();
  resetKalmanAxis(rollKalmanState);
  resetKalmanAxis(pitchKalmanState);
  calibrateMpuBias();

  connectWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/kalman", HTTP_GET, handleKalmanSettingsGet);
  server.on("/api/kalman", HTTP_POST, handleKalmanSettings);
  server.on("/api/calibrate", HTTP_POST, handleCalibrate);
  server.on("/api/start", HTTP_POST, handleStart);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/reset", HTTP_POST, handleReset);
  server.begin();

  lastImuMicros = micros();

  Serial.println("Distance web server ready.");
  Serial.println("Open the IP shown above in a browser.");
}

void loop() {
  server.handleClient();
  updateImuEstimates();

  if (autoRunActive) {
    long leftTicks = 0;
    long rightTicks = 0;
    float encoderDistance = readAverageEncoderDistanceM(leftTicks, rightTicks);

    if (encoderDistance >= TARGET_DISTANCE_M) {
      stopAutoRun();
    }
  }

  if (millis() - lastStatusMillis >= 1000) {
    lastStatusMillis = millis();
    long leftTicks = 0;
    long rightTicks = 0;
    float encoderDistance = readAverageEncoderDistanceM(leftTicks, rightTicks);

    Serial.print("Run=");
    Serial.print(autoRunActive ? "ON" : "OFF");
    Serial.print(" Enc=");
    Serial.print(encoderDistance, 3);
    Serial.print("m Raw=");
    Serial.print(rawMotion.distanceM, 3);
    Serial.print("m Kalman=");
    Serial.print(kalmanMotion.distanceM, 3);
    Serial.print("m RawR=");
    Serial.print(rawMotion.rollDeg, 1);
    Serial.print(" RawP=");
    Serial.print(rawMotion.pitchDeg, 1);
    Serial.print(" RawY=");
    Serial.print(rawMotion.yawDeg, 1);
    Serial.print(" KalR=");
    Serial.print(kalmanMotion.rollDeg, 1);
    Serial.print(" KalP=");
    Serial.print(kalmanMotion.pitchDeg, 1);
    Serial.print(" KalY=");
    Serial.println(kalmanMotion.yawDeg, 1);
  }
}
