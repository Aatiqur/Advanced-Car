#include <Wire.h>

#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

const int MPU_SDA = 21;
const int MPU_SCL = 22;

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission(true);
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, (uint8_t)1, (uint8_t)true);
  return Wire.available() ? Wire.read() : 0;
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

  Serial.println("MPU6050 Test Started");
  Serial.print("WHO_AM_I: 0x");
  Serial.println(readRegister(0x75), HEX);
}

void loop() {
  int16_t accelX = read16(MPU6050_ACCEL_XOUT_H);
  int16_t accelY = read16(MPU6050_ACCEL_XOUT_H + 2);
  int16_t accelZ = read16(MPU6050_ACCEL_XOUT_H + 4);
  int16_t tempRaw = read16(MPU6050_ACCEL_XOUT_H + 6);
  int16_t gyroX = read16(MPU6050_ACCEL_XOUT_H + 8);
  int16_t gyroY = read16(MPU6050_ACCEL_XOUT_H + 10);
  int16_t gyroZ = read16(MPU6050_ACCEL_XOUT_H + 12);

  float temperatureC = (tempRaw / 340.0) + 36.53;

  Serial.print("Accel X: ");
  Serial.print(accelX);
  Serial.print(" | Accel Y: ");
  Serial.print(accelY);
  Serial.print(" | Accel Z: ");
  Serial.print(accelZ);

  Serial.print(" || Gyro X: ");
  Serial.print(gyroX);
  Serial.print(" | Gyro Y: ");
  Serial.print(gyroY);
  Serial.print(" | Gyro Z: ");
  Serial.print(gyroZ);

  Serial.print(" || Temp C: ");
  Serial.println(temperatureC, 2);

  delay(500);
}
