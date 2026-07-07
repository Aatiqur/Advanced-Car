#include <Wire.h>
#include <math.h>

#define MPU9250_ADDR 0x68

// ===============================
// KALMAN STATES
// ===============================
float x_roll[2] = { 0, 0 };
float P_roll[2][2] = { { 1, 0 }, { 0, 1 } };

float x_pitch[2] = { 0, 0 };
float P_pitch[2][2] = { { 1, 0 }, { 0, 1 } };

// Tuned for stable hover
float Q[2][2] = { { 0.01, 0 }, { 0, 0.003 } };
float R = 0.7;

// ===============================
// CALIBRATION VARIABLES
// ===============================
float gyroBiasX = 0;
float gyroBiasY = 0;

float rollOffset = 0;
float pitchOffset = 0;

bool calibrated = false;
unsigned long startCalTime;
int calCount = 0;

// Timing
unsigned long previousTime;
float dt;

// ===============================
void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9);

  // Wake MPU
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  // Set Gyro ±250°/s  (FS_SEL = 0)
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  // Set Accel ±2g (AFS_SEL = 0)
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x1C);
  Wire.write(0x00);
  Wire.endTransmission();

  startCalTime = millis();
  previousTime = millis();

  Serial.println("Keep sensor flat & still for 10 seconds...");
  Serial.println("RollKalman,PitchKalman,RollRaw,PitchRaw,Min,Max");
}

// ===============================
void readMPU(float &ax, float &ay, float &az,
             float &gx, float &gy, float &gz) {
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDR, 14, true);

  int16_t rawAx = Wire.read() << 8 | Wire.read();
  int16_t rawAy = Wire.read() << 8 | Wire.read();
  int16_t rawAz = Wire.read() << 8 | Wire.read();
  Wire.read();
  Wire.read();  // skip temp
  int16_t rawGx = Wire.read() << 8 | Wire.read();
  int16_t rawGy = Wire.read() << 8 | Wire.read();
  int16_t rawGz = Wire.read() << 8 | Wire.read();

  // Scaling for ±2g
  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;

  // Scaling for ±250°/s
  gx = rawGx / 131.0;
  gy = rawGy / 131.0;
  gz = rawGz / 131.0;
}

// ===============================
float kalmanUpdate(float x[2], float P[2][2],
                   float accAngle, float gyroRate, float dt) {
  float A[2][2] = { { 1, -dt }, { 0, 1 } };
  float B[2] = { dt, 0 };

  float x_pred[2];

  // Prediction
  x_pred[0] = A[0][0] * x[0] + A[0][1] * x[1] + B[0] * gyroRate;
  x_pred[1] = A[1][0] * x[0] + A[1][1] * x[1];

  x[0] = x_pred[0];
  x[1] = x_pred[1];

  float AP[2][2];
  float P_pred[2][2];

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      AP[i][j] = A[i][0] * P[0][j] + A[i][1] * P[1][j];
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      P_pred[i][j] = AP[i][0] * A[j][0] + AP[i][1] * A[j][1] + Q[i][j];
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      P[i][j] = P_pred[i][j];
    }
  }

  // Measurement
  float y = accAngle - x[0];
  float S = P[0][0] + R;

  float K0 = P[0][0] / S;
  float K1 = P[1][0] / S;

  // Update
  x[0] = x[0] + K0 * y;
  x[1] = x[1] + K1 * y;

  float P00 = P[0][0];
  float P01 = P[0][1];

  P[0][0] -= K0 * P00;
  P[0][1] -= K0 * P01;
  P[1][0] -= K1 * P00;
  P[1][1] -= K1 * P01;

  return x[0];
}

// ===============================
void loop() {
  unsigned long currentTime = millis();
  dt = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  float ax, ay, az, gx, gy, gz;
  readMPU(ax, ay, az, gx, gy, gz);

  float rollAcc = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
  float pitchAcc = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  // ===== 10s CALIBRATION =====
  if (!calibrated) {
    gyroBiasX += gx;
    gyroBiasY += gy;
    rollOffset += rollAcc;
    pitchOffset += pitchAcc;
    calCount++;

    if (millis() - startCalTime >= 10000) {
      gyroBiasX /= calCount;
      gyroBiasY /= calCount;
      rollOffset /= calCount;
      pitchOffset /= calCount;
      calibrated = true;
      Serial.println("Calibration Finished!");
    }
    return;
  }

  // Remove gyro bias
  gx -= gyroBiasX;
  gy -= gyroBiasY;

  // Zero level reference
  rollAcc -= rollOffset;
  pitchAcc -= pitchOffset;

  float rollKalman = kalmanUpdate(x_roll, P_roll, rollAcc, gx, dt);
  float pitchKalman = kalmanUpdate(x_pitch, P_pitch, pitchAcc, gy, dt);

  // Serial Plotter Output
  Serial.print(rollKalman);
  Serial.print(",");
  Serial.print(pitchKalman);
  Serial.print(",");
  Serial.print(rollAcc);
  Serial.print(",");
  Serial.print(pitchAcc);
  Serial.print(",");
  Serial.print(-90);
  Serial.print(",");
  Serial.println(90);

  delay(5);
}