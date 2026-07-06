#define ENCODER_A 5
#define ENCODER_B 18


const float ENCODER_CPR = 374.0; 

// --- VARIABLES ---
volatile long encoderTicks = 0;
unsigned long previousMillis = 0;
const long interval = 500; // Calculate RPM every 500ms
float currentRPM = 0.0;

// Interrupt Service Routine (ISR) for Encoder A
void IRAM_ATTR readEncoder() {
  // Read Pin B to determine direction
  if (digitalRead(ENCODER_B) == HIGH) {
    encoderTicks++;
  } else {
    encoderTicks--;
  }
}

void setup() {
  Serial.begin(115200);

  // Configure encoder pins with internal pull-up resistors
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // Attach interrupt to Pin A on RISING edge
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), readEncoder, RISING);
  
  Serial.println("ESP32 Encoder RPM Measurement Started.");
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking timer to calculate RPM
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read and reset ticks safely by temporarily disabling interrupts
    noInterrupts();
    long ticks = encoderTicks;
    encoderTicks = 0;
    interrupts();

    // Calculate RPM: (ticks / CPR) * (60,000ms / interval_ms)
    float revolutions = (float)ticks / ENCODER_CPR;
    float timeFactor = 60000.0 / interval;
    currentRPM = revolutions * timeFactor;

    // Print results to Serial Monitor
    Serial.print("Ticks/interval: ");
    Serial.print(ticks);
    Serial.print(" | Motor RPM: ");
    Serial.println(currentRPM, 2);
  }

  // You can freely add other non-blocking code here
}
