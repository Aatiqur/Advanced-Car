// Encoder.ino — quad-channel tick counting, average distance
void IRAM_ATTR readLeftEncoder() {
  if (digitalRead(LEFT_ENCODER_B) == HIGH) {
    leftEncoderTicks++;
  } else {
    leftEncoderTicks--;
  }
}

void IRAM_ATTR readRightEncoder() {
  if (digitalRead(RIGHT_ENCODER_B) == HIGH) {
    rightEncoderTicks++;
  } else {
    rightEncoderTicks--;
  }
}

void resetEncoderTicks() {
  noInterrupts();
  leftEncoderTicks = 0;
  rightEncoderTicks = 0;
  interrupts();
}

float readAverageEncoderDistanceM(long &leftTicksOut, long &rightTicksOut) {
  noInterrupts();
  leftTicksOut = leftEncoderTicks;
  rightTicksOut = rightEncoderTicks;
  interrupts();

  float leftDistance = fabs((float)leftTicksOut) * DISTANCE_PER_TICK_M;
  float rightDistance = fabs((float)rightTicksOut) * DISTANCE_PER_TICK_M;
  return (leftDistance + rightDistance) * 0.5f;
}
