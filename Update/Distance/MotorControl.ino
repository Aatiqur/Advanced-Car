// Shared declarations for the split Distance project.
void setLeftMotor(int speedValue) {
  if (speedValue > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(ENA, speedValue);
  } else if (speedValue < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(ENA, -speedValue);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    ledcWrite(ENA, 0);
  }
}

void setRightMotor(int speedValue) {
  if (speedValue > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(ENB, speedValue);
  } else if (speedValue < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    ledcWrite(ENB, -speedValue);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    ledcWrite(ENB, 0);
  }
}

void stopMotors() {
  setLeftMotor(0);
  setRightMotor(0);
}

void driveForward() {
  setLeftMotor(DRIVE_PWM + LEFT_PWM_TRIM);
  setRightMotor(DRIVE_PWM);
}
