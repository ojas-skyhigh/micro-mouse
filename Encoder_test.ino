#define ENCODER1_PIN 34
#define ENCODER2_PIN 35

#define MOTOR_1 1
#define MOTOR_2 2

#define PULSES_PER_REV 20
#define WHEEL_DIAMETER_CM 6.5
#define WHEEL_CIRC_CM (3.14159 * WHEEL_DIAMETER_CM)

#define MAX_RPM 10000

// -------- Encoder 1 --------
volatile uint32_t lastMicros1 = 0;
volatile uint32_t pulsePeriod1 = 0;
volatile bool firstPulse1 = true;
volatile uint32_t totalPulses1 = 0;

// -------- Encoder 2 --------
volatile uint32_t lastMicros2 = 0;
volatile uint32_t pulsePeriod2 = 0;
volatile bool firstPulse2 = true;
volatile uint32_t totalPulses2 = 0;

// -------- ISR: Encoder 1 --------
void IRAM_ATTR encoderISR1() {
  uint32_t now = micros();
  totalPulses1++;

  if (firstPulse1) {
    lastMicros1 = now;
    firstPulse1 = false;
    return;
  }

  uint32_t period = now - lastMicros1;
  lastMicros1 = now;

  if (period > 50) {
    pulsePeriod1 = period;
  }
}

// -------- ISR: Encoder 2 --------
void IRAM_ATTR encoderISR2() {
  uint32_t now = micros();
  totalPulses2++;

  if (firstPulse2) {
    lastMicros2 = now;
    firstPulse2 = false;
    return;
  }

  uint32_t period = now - lastMicros2;
  lastMicros2 = now;

  if (period > 50) {
    pulsePeriod2 = period;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER1_PIN, INPUT);
  pinMode(ENCODER2_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER1_PIN), encoderISR1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER2_PIN), encoderISR2, RISING);
}

void loop() {
  // Example usage
  Serial.print("RPM1: ");
  Serial.print(getRPM(MOTOR_1));
  Serial.print(" | Dist1(cm): ");
  Serial.print(getDistanceCm(MOTOR_1));

  Serial.print(" || RPM2: ");
  Serial.print(getRPM(MOTOR_2));
  Serial.print(" | Dist2(cm): ");
  Serial.println(getDistanceCm(MOTOR_2));

  delay(200);
}

//
// ================= FUNCTIONS =================
//

// -------- Get RPM --------
float getRPM(uint8_t motor) {
  uint32_t period, last;

  noInterrupts();
  if (motor == MOTOR_1) {
    period = pulsePeriod1;
    last   = lastMicros1;
  } else {
    period = pulsePeriod2;
    last   = lastMicros2;
  }
  interrupts();

  if (period == 0 || (micros() - last) > 500000) {
    return 0.0;
  }

  float rpm = 60e6 / (period * PULSES_PER_REV);
  if (rpm > MAX_RPM) rpm = MAX_RPM;

  return rpm;
}

// -------- Get Distance (cm) --------
float getDistanceCm(uint8_t motor) {
  uint32_t pulses;

  noInterrupts();
  pulses = (motor == MOTOR_1) ? totalPulses1 : totalPulses2;
  interrupts();

  return (pulses / (float)PULSES_PER_REV) * WHEEL_CIRC_CM;
}

// -------- Reset Distance --------
void resetDistance(uint8_t motor) {
  noInterrupts();
  if (motor == MOTOR_1) {
    totalPulses1 = 0;
  } else {
    totalPulses2 = 0;
  }
  interrupts();
}



