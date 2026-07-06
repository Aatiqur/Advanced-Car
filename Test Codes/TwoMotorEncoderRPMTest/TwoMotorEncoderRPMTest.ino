#define LEFT_ENCODER_A 32
#define LEFT_ENCODER_B 33
#define RIGHT_ENCODER_A 25
#define RIGHT_ENCODER_B 26

const float ENCODER_CPR = 374.0;
const unsigned long SAMPLE_INTERVAL_MS = 1000;

volatile long leftEncoderTicks = 0;
volatile long rightEncoderTicks = 0;

unsigned long previousMillis = 0;
float leftRPM = 0.0;
float rightRPM = 0.0;

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

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_ENCODER_A, INPUT_PULLUP);
  pinMode(LEFT_ENCODER_B, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_A, INPUT_PULLUP);
  pinMode(RIGHT_ENCODER_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_ENCODER_A), readLeftEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODER_A), readRightEncoder, RISING);

  Serial.println("Two Motor Encoder RPM Test Started.");
  Serial.println("Rotate the wheels by hand and watch both RPM values in Serial Monitor.");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= SAMPLE_INTERVAL_MS) {
    previousMillis = currentMillis;

    noInterrupts();
    long leftTicks = leftEncoderTicks;
    long rightTicks = rightEncoderTicks;
    leftEncoderTicks = 0;
    rightEncoderTicks = 0;
    interrupts();

    float revolutionsLeft = (float)leftTicks / ENCODER_CPR;
    float revolutionsRight = (float)rightTicks / ENCODER_CPR;
    float timeFactor = 60000.0 / SAMPLE_INTERVAL_MS;

    leftRPM = revolutionsLeft * timeFactor;
    rightRPM = revolutionsRight * timeFactor;

    Serial.print("Left Ticks: ");
    Serial.print(leftTicks);
    Serial.print(" | Left RPM: ");
    Serial.print(leftRPM, 2);
    Serial.print(" | Right Ticks: ");
    Serial.print(rightTicks);
    Serial.print(" | Right RPM: ");
    Serial.println(rightRPM, 2);
  }
}
