#include <Wire.h>

#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

const int MPU_SDA = 21;
const int MPU_SCL = 22;

const float ACCEL_SCALE = 16384.0; // +/-2g
const float GYRO_SCALE = 131.0;    // +/-250 deg/s

unsigned long previousMicros = 0;
float yaw = 0.0;

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission(true);
}

int16_t read16(uint8_t reg) {
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

void setup() {
  Serial.begin(115200);
  Wire.begin(MPU_SDA, MPU_SCL);
  Wire.setClock(400000);

  delay(100);
  writeRegister(MPU6050_PWR_MGMT_1, 0x00);
  delay(100);

  previousMicros = micros();

  Serial.println("MPU6050 Roll Pitch Yaw Test Started");
}

void loop() {
  int16_t accelXRaw = read16(MPU6050_ACCEL_XOUT_H);
  int16_t accelYRaw = read16(MPU6050_ACCEL_XOUT_H + 2);
  int16_t accelZRaw = read16(MPU6050_ACCEL_XOUT_H + 4);
  int16_t gyroXRaw = read16(MPU6050_ACCEL_XOUT_H + 8);
  int16_t gyroYRaw = read16(MPU6050_ACCEL_XOUT_H + 10);
  int16_t gyroZRaw = read16(MPU6050_ACCEL_XOUT_H + 12);

  float ax = accelXRaw / ACCEL_SCALE;
  float ay = accelYRaw / ACCEL_SCALE;
  float az = accelZRaw / ACCEL_SCALE;

  float gx = gyroXRaw / GYRO_SCALE;
  float gy = gyroYRaw / GYRO_SCALE;
  float gz = gyroZRaw / GYRO_SCALE;

  float roll = atan2(ay, az) * 180.0 / PI;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  unsigned long currentMicros = micros();
  float deltaTime = (currentMicros - previousMicros) / 1000000.0;
  previousMicros = currentMicros;

  yaw += gz * deltaTime;

  Serial.print("Roll: ");
  Serial.print(roll, 2);
  Serial.print(" deg | Pitch: ");
  Serial.print(pitch, 2);
  Serial.print(" deg | Yaw: ");
  Serial.print(yaw, 2);
  Serial.print(" deg");

  Serial.print(" || GX: ");
  Serial.print(gx, 2);
  Serial.print(" | GY: ");
  Serial.print(gy, 2);
  Serial.print(" | GZ: ");
  Serial.println(gz, 2);

  delay(100);
}
