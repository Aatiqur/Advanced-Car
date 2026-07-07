// Shared declarations for the split Distance project.
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

void calibrateMpuBiasDuration(int seconds) {
  const unsigned long endMs = millis() + (unsigned long)seconds * 1000UL;
  long accelXSum = 0;
  long accelYSum = 0;
  long accelZSum = 0;
  long gyroXSum = 0;
  long gyroYSum = 0;
  long gyroZSum = 0;
  unsigned long count = 0;

  while (millis() < endMs) {
    int16_t axRaw = mpuRead16(MPU6050_ACCEL_XOUT_H);
    int16_t ayRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 2);
    int16_t azRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 4);
    int16_t gxRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 8);
    int16_t gyRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 10);
    int16_t gzRaw = mpuRead16(MPU6050_ACCEL_XOUT_H + 12);

    accelXSum += axRaw;
    accelYSum += ayRaw;
    accelZSum += azRaw;
    gyroXSum += gxRaw;
    gyroYSum += gyRaw;
    gyroZSum += gzRaw;
    count++;
    delay(10);
  }

  if (count == 0) return;

  accelXBiasG = (float)accelXSum / (float)count / ACCEL_SCALE;
  accelYBiasG = (float)accelYSum / (float)count / ACCEL_SCALE;
  accelZBiasG = (float)accelZSum / (float)count / ACCEL_SCALE;
  gyroXBiasDps = (float)gyroXSum / (float)count / GYRO_SCALE;
  gyroYBiasDps = (float)gyroYSum / (float)count / GYRO_SCALE;
  gyroZBiasDps = (float)gyroZSum / (float)count / GYRO_SCALE;

  // compute roll/pitch offsets using averaged accel G
  float axG = accelXBiasG;
  float ayG = accelYBiasG;
  float azG = accelZBiasG;
  float rollAcc = atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0f / PI;
  float pitchAcc = atan2(-axG, sqrt(ayG * ayG + azG * azG)) * 180.0f / PI;
  kalmanSettings.rollOffsetDeg = rollAcc;
  kalmanSettings.pitchOffsetDeg = pitchAcc;

  saveKalmanSettings();
}

void calibrateMpuBias() {
  const int sampleCount = 300;
  long accelXSum = 0;
  long accelYSum = 0;
  long accelZSum = 0;
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
    float rollAcc = atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0f / PI;
    float pitchAcc = atan2(-axG, sqrt(ayG * ayG + azG * azG)) * 180.0f / PI;

    accelXSum += axRaw;
    accelYSum += ayRaw;
    accelZSum += azRaw;
    gyroXSum += gxRaw;
    gyroYSum += gyRaw;
    gyroZSum += gzRaw;
    rollSum += rollAcc;
    pitchSum += pitchAcc;
    delay(3);
  }

  accelXBiasG = (float)accelXSum / sampleCount / ACCEL_SCALE;
  accelYBiasG = (float)accelYSum / sampleCount / ACCEL_SCALE;
  accelZBiasG = (float)accelZSum / sampleCount / ACCEL_SCALE;
  gyroXBiasDps = (float)gyroXSum / sampleCount / GYRO_SCALE;
  gyroYBiasDps = (float)gyroYSum / sampleCount / GYRO_SCALE;
  gyroZBiasDps = (float)gyroZSum / sampleCount / GYRO_SCALE;
  kalmanSettings.rollOffsetDeg = rollSum / sampleCount;
  kalmanSettings.pitchOffsetDeg = pitchSum / sampleCount;
  saveKalmanSettings();
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

  float axG = (float)axRaw / ACCEL_SCALE - accelXBiasG;
  float ayG = (float)ayRaw / ACCEL_SCALE - accelYBiasG;
  float azG = (float)azRaw / ACCEL_SCALE - accelZBiasG;

  float gxDps = (float)gxRaw / GYRO_SCALE - gyroXBiasDps;
  float gyDps = (float)gyRaw / GYRO_SCALE - gyroYBiasDps;
  float gzDps = (float)gzRaw / GYRO_SCALE - gyroZBiasDps;

  float rawRollAcc = atan2(ayG, sqrt(axG * axG + azG * azG)) * 180.0f / PI;
  float rawPitchAcc = atan2(-axG, sqrt(ayG * ayG + azG * azG)) * 180.0f / PI;

  rawMotion.rollDeg = rawRollAcc - kalmanSettings.rollOffsetDeg;
  rawMotion.pitchDeg = rawPitchAcc - kalmanSettings.pitchOffsetDeg;
  rawMotion.yawDeg += gzDps * dt;

  float rawLinearX = 0.0f;
  float rawLinearY = 0.0f;
  float rawLinearZ = 0.0f;
  computeGravityCompensatedAccel(axG, ayG, azG, rawMotion.rollDeg, rawMotion.pitchDeg, rawLinearX, rawLinearY, rawLinearZ);
  float rawAccelMagnitudeMps2 = vectorMagnitude(rawLinearX, rawLinearY, rawLinearZ);

  // stationary detection: combine accel and gyro thresholds to avoid integrating noise
  float gyroMag = fabs(gxDps) + fabs(gyDps) + fabs(gzDps);
  if (rawAccelMagnitudeMps2 < kalmanSettings.stillAccelThreshold && gyroMag < kalmanSettings.gyroStillThreshold) {
    rawAccelMagnitudeMps2 = 0.0f;
    if (fabs(rawMotion.velocityMps) < kalmanSettings.stillVelocityThreshold) {
      rawMotion.velocityMps = 0.0f;
    } else {
      rawMotion.velocityMps *= kalmanSettings.velocityDecay;
    }
  }
  integrateMotion(rawMotion, rawAccelMagnitudeMps2, dt);

  float filteredRollInput = rawRollAcc - kalmanSettings.rollOffsetDeg;
  float filteredPitchInput = rawPitchAcc - kalmanSettings.pitchOffsetDeg;

  kalmanMotion.rollDeg = kalmanUpdateAxis(rollKalmanState, filteredRollInput, gxDps, dt);
  kalmanMotion.pitchDeg = kalmanUpdateAxis(pitchKalmanState, filteredPitchInput, gyDps, dt);
  kalmanMotion.yawDeg += gzDps * dt;

  float kalmanLinearX = 0.0f;
  float kalmanLinearY = 0.0f;
  float kalmanLinearZ = 0.0f;
  computeGravityCompensatedAccel(axG, ayG, azG, kalmanMotion.rollDeg, kalmanMotion.pitchDeg, kalmanLinearX, kalmanLinearY, kalmanLinearZ);
  float kalmanAccelMagnitudeMps2 = vectorMagnitude(kalmanLinearX, kalmanLinearY, kalmanLinearZ);
  // apply same stationary detection to kalman motion
  float gyroMag2 = fabs(gxDps) + fabs(gyDps) + fabs(gzDps);
  if (kalmanAccelMagnitudeMps2 < kalmanSettings.stillAccelThreshold && gyroMag2 < kalmanSettings.gyroStillThreshold) {
    kalmanAccelMagnitudeMps2 = 0.0f;
    if (fabs(kalmanMotion.velocityMps) < kalmanSettings.stillVelocityThreshold) {
      kalmanMotion.velocityMps = 0.0f;
    } else {
      kalmanMotion.velocityMps *= kalmanSettings.velocityDecay;
    }
  }
  integrateMotion(kalmanMotion, kalmanAccelMagnitudeMps2, dt);

  (void)gxDps;
  (void)gyDps;
  (void)rawLinearX;
  (void)rawLinearY;
  (void)rawLinearZ;
}
