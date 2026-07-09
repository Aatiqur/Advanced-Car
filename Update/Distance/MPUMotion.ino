// MPUMotion.ino — raw MPU6050: roll/pitch offset, yaw integration, gravity-compensated accel
void mpuWriteRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission(true);
}

int16_t mpuRead16(uint8_t reg) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, (uint8_t)2, (uint8_t)true);

  int16_t value = 0;
  if (Wire.available() >= 2) {
    value = (int16_t)(Wire.read() << 8) | Wire.read();
  }
  return value;
}

bool mpuInitialize() {
  Wire.begin(21, 22);
  Wire.setClock(400000);
  delay(100);
  mpuWriteRegister(MPU6050_PWR_MGMT_1, 0x00);
  delay(100);
  return true;
}

// Sample the MPU for a few seconds and learn the gyro zero-rate offset + the
// roll/pitch mounting-tilt offset (so "0" roll/pitch = level).
void calibrateMpuBiasDuration(int seconds) {
  const unsigned long endMs = millis() + (unsigned long)seconds * 1000UL;
  long gyroXSum = 0;
  long gyroYSum = 0;
  long gyroZSum = 0;
  float rollSum = 0.0f;
  float pitchSum = 0.0f;
  unsigned long count = 0;

  while (millis() < endMs) {
    int16_t axRaw = mpuRead16(MPU6050_ACCEL_XOUT_H);
    int16_t ayRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 2);
    int16_t azRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 4);
    int16_t gxRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 8);
    int16_t gyRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 10);
    int16_t gzRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 12);

    float axG = (float)axRaw / ACCEL_SCALE;
    float ayG = (float)ayRaw / ACCEL_SCALE;
    float azG = (float)azRaw / ACCEL_SCALE;
    rollSum  += atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0f / PI;
    pitchSum += atan2(-axG, sqrt(ayG * ayG + azG * azG)) * 180.0f / PI;

    gyroXSum += gxRaw;
    gyroYSum += gyRaw;
    gyroZSum += gzRaw;
    count++;
    delay(10);
  }

  if (count == 0) return;

  gyroXBiasDps = (float)gyroXSum / (float)count / GYRO_SCALE;
  gyroYBiasDps = (float)gyroYSum / (float)count / GYRO_SCALE;
  gyroZBiasDps = (float)gyroZSum / (float)count / GYRO_SCALE;
  motion.rollOffsetDeg  = rollSum  / (float)count;
  motion.pitchOffsetDeg = pitchSum / (float)count;
}

void calibrateMpuBias() {
  const int sampleCount = 300;
  long gyroXSum = 0;
  long gyroYSum = 0;
  long gyroZSum = 0;
  float rollSum = 0.0f;
  float pitchSum = 0.0f;

  for (int index = 0; index < sampleCount; index++) {
    int16_t axRaw = mpuRead16(MPU6050_ACCEL_XOUT_H);
    int16_t ayRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 2);
    int16_t azRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 4);
    int16_t gxRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 8);
    int16_t gyRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 10);
    int16_t gzRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 12);

    float axG = (float)axRaw / ACCEL_SCALE;
    float ayG = (float)ayRaw / ACCEL_SCALE;
    float azG = (float)azRaw / ACCEL_SCALE;
    rollSum  += atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0f / PI;
    pitchSum += atan2(-axG, sqrt(ayG * ayG + azG * azG)) * 180.0f / PI;

    gyroXSum += gxRaw;
    gyroYSum += gyRaw;
    gyroZSum += gzRaw;
    delay(3);
  }

  gyroXBiasDps = (float)gyroXSum / sampleCount / GYRO_SCALE;
  gyroYBiasDps = (float)gyroYSum / sampleCount / GYRO_SCALE;
  gyroZBiasDps = (float)gyroZSum / sampleCount / GYRO_SCALE;
  motion.rollOffsetDeg  = rollSum  / sampleCount;
  motion.pitchOffsetDeg = pitchSum / sampleCount;
}

void updateImuEstimates() {
  unsigned long nowMicros = micros();
  float dt = (nowMicros - lastImuMicros) / 1000000.0f;
  lastImuMicros = nowMicros;

  if (dt <= 0.0f || dt > 0.2f) {
    return;
  }

  int16_t axRaw = mpuRead16(MPU6050_ACCEL_XOUT_H);
  int16_t ayRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 2);
  int16_t azRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 4);
  int16_t gxRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 8);
  int16_t gyRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 10);
  int16_t gzRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 12);

  float axG = (float)axRaw / ACCEL_SCALE;
  float ayG = (float)ayRaw / ACCEL_SCALE;
  float azG = (float)azRaw / ACCEL_SCALE;

  float gxDps = (float)gxRaw / GYRO_SCALE - gyroXBiasDps;
  float gyDps = (float)gyRaw / GYRO_SCALE - gyroYBiasDps;
  float gzDps = (float)gzRaw / GYRO_SCALE - gyroZBiasDps;

  // Accel-derived roll/pitch in degrees
  float rollAcc  = atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0f / PI;
  float pitchAcc = atan2(-axG, sqrt(ayG * ayG + azG * azG)) * 180.0f / PI;

  // Subtract the learned mounting offsets so "0" means level
  motion.rollDeg  = rollAcc  - motion.rollOffsetDeg;
  motion.pitchDeg = pitchAcc - motion.pitchOffsetDeg;

  // Yaw: integrate the bias-corrected Z gyro (no magnetometer -> drifts over time)
  motion.yawDeg += gzDps * dt;

  // Remove gravity using the same raw (uncorrected) angles used to derive axG/ayG/azG
  float linX = 0.0f, linY = 0.0f, linZ = 0.0f;
  computeGravityCompensatedAccel(axG, ayG, azG, rollAcc, pitchAcc, linX, linY, linZ);
  float accelMag = vectorMagnitude(linX, linY, linZ);

  integrateMotion(motion, accelMag, dt);
}
