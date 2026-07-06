#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 19
#define ENA 27
#define ENB 14

const int PWM_FREQ = 1000;
const int PWM_RESOLUTION = 8;

int driveSpeed = 180;

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

void forward() {
  setLeftMotor(driveSpeed);
  setRightMotor(driveSpeed);
}

void backward() {
  setLeftMotor(-driveSpeed);
  setRightMotor(-driveSpeed);
}

void turnLeft() {
  setLeftMotor(-driveSpeed);
  setRightMotor(driveSpeed);
}

void turnRight() {
  setLeftMotor(driveSpeed);
  setRightMotor(-driveSpeed);
}

void forwardLeft() {
  setLeftMotor(driveSpeed / 2);
  setRightMotor(driveSpeed);
}

void forwardRight() {
  setLeftMotor(driveSpeed);
  setRightMotor(driveSpeed / 2);
}

void backwardLeft() {
  setLeftMotor(-driveSpeed / 2);
  setRightMotor(-driveSpeed);
}

void backwardRight() {
  setLeftMotor(-driveSpeed);
  setRightMotor(-driveSpeed / 2);
}

void increaseSpeed() {
  driveSpeed += 15;
  if (driveSpeed > 255) {
    driveSpeed = 255;
  }
}

void decreaseSpeed() {
  driveSpeed -= 15;
  if (driveSpeed < 0) {
    driveSpeed = 0;
  }
}

void printHelp() {
  SerialBT.println("Commands:");
  SerialBT.println("F = forward");
  SerialBT.println("B = backward");
  SerialBT.println("L = left");
  SerialBT.println("R = right");
  SerialBT.println("G = forward-left");
  SerialBT.println("I = forward-right");
  SerialBT.println("H = backward-left");
  SerialBT.println("J = backward-right");
  SerialBT.println("S = stop");
  SerialBT.println("+ = speed up");
  SerialBT.println("- = speed down");
  SerialBT.println("0-9 = set speed level");
  SerialBT.println("V = help");
}

void setSpeedLevel(char command) {
  int level = command - '0';
  driveSpeed = map(level, 0, 9, 0, 255);
}

void handleCommand(char command) {
  switch (command) {
    case 'F':
    case 'f':
      forward();
      SerialBT.println("Forward");
      break;
    case 'B':
    case 'b':
      backward();
      SerialBT.println("Backward");
      break;
    case 'L':
    case 'l':
      turnLeft();
      SerialBT.println("Left");
      break;
    case 'R':
    case 'r':
      turnRight();
      SerialBT.println("Right");
      break;
    case 'G':
    case 'g':
      forwardLeft();
      SerialBT.println("Forward Left");
      break;
    case 'I':
    case 'i':
      forwardRight();
      SerialBT.println("Forward Right");
      break;
    case 'H':
    case 'h':
      backwardLeft();
      SerialBT.println("Backward Left");
      break;
    case 'J':
    case 'j':
      backwardRight();
      SerialBT.println("Backward Right");
      break;
    case 'S':
    case 's':
      stopMotors();
      SerialBT.println("Stop");
      break;
    case '+':
      increaseSpeed();
      SerialBT.print("Speed: ");
      SerialBT.println(driveSpeed);
      break;
    case '-':
      decreaseSpeed();
      SerialBT.print("Speed: ");
      SerialBT.println(driveSpeed);
      break;
    case 'V':
    case 'v':
      printHelp();
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      setSpeedLevel(command);
      SerialBT.print("Speed level set: ");
      SerialBT.println(driveSpeed);
      break;
    default:
      SerialBT.println("Unknown command. Send H for help.");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("AdvancedCar_BT");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(ENB, PWM_FREQ, PWM_RESOLUTION);

  stopMotors();

  Serial.println("Bluetooth car control started.");
  Serial.println("Pair with AdvancedCar_BT and send F, B, L, R, G, I, H, J, S, +, -");
  SerialBT.println("AdvancedCar_BT ready. Send V for help.");
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();
    if (command == '\n' || command == '\r') {
      return;
    }
    handleCommand(command);
  }
}
