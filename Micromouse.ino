#include "floodfill.h"

const int r_trigPin = 5;
const int r_echoPin = 18;

const int l_trigPin = 14;
const int l_echoPin = 32;

const int f_trigPin = 21;
const int f_echoPin = 19;

const int l_in2 = 27;   // speed
const int l_in1 = 26;   // direction

const int r_in2 = 13;   // speed
const int r_in1 = 25;   // direction

const int pwmFreq = 500;
const int pwmResolution = 10; 
`

#define SOUND_SPEED 0.034
#define WALL_DIST 10.00

#define ENCODER1_PIN 34
#define ENCODER2_PIN 35

#define PULSES_PER_REV 20/- 
#define MAX_RPM 10000

// -------- Encoder 1 --------
volatile uint32_t lastMicros1 = 0;
volatile uint32_t pulsePeriod1 = 0;
volatile bool firstPulse1 = true;

// -------- Encoder 2 --------
volatile uint32_t lastMicros2 = 0;
volatile uint32_t pulsePeriod2 = 0;
volatile bool firstPulse2 = true;

void IRAM_ATTR encoderISR1() {
  uint32_t now = micros();

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

long duration;
float distance;

enum MotionState {
  MOVING_FORWARD,
  TURNING_LEFT,
  TURNING_RIGHT
};

MotionState state = MOVING_FORWARD;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(l_in1, OUTPUT);
  pinMode(r_in1, OUTPUT);

  ledcAttach(l_in2, pwmFreq, pwmResolution);
  ledcAttach(r_in2, pwmFreq, pwmResolution);
  
  pinMode(r_trigPin, OUTPUT); 
  pinMode(r_echoPin, INPUT); 
  pinMode(l_trigPin, OUTPUT); 
  pinMode(l_echoPin, INPUT); 
  pinMode(f_trigPin, OUTPUT); 
  pinMode(f_echoPin, INPUT); 

  pinMode(ENCODER1_PIN, INPUT);
  pinMode(ENCODER2_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER1_PIN), encoderISR1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER2_PIN), encoderISR2, RISING);

  floodfillInit();
}

void loop() {

  bool F = isWall(f_trigPin, f_echoPin); // wall detection, either ir or ultrasonic or tof idk
  bool L = isWall(l_trigPin, l_echoPin);
  bool R = isWall(r_trigPin, r_echoPin);


  while (!floodfillAtGoal()) {
    floodfillUpdateWalls(F, L, R);

    Dir next = floodfillNextMove();

    turnTo(next);          // motor stuff to turn, also will add mpu6050
    moveOneCell();         // encoders will ensure that it moves exactly 1 cell hopefully

    floodfillSetHeading(next);
    floodfillAdvance();
  }

  // switch (state) {

  //   case MOVING_FORWARD:
  //     if (!L) {
  //       // turnLeft();
  //       state = TURNING_LEFT;
  //     }
  //     else if (F) {
  //       // turnRight();
  //       state = TURNING_RIGHT;
  //     }
  //     else {
  //       moveForward(l_in1, l_in2);
  //       moveForward(r_in1, r_in2);
  //     }
  //     break;

  //   case TURNING_LEFT:
  //     // Stop turning when left wall appears again
  //     if (L) {
  //       stopMotors();
  //       moveForward(l_in1, l_in2);
  //       moveForward(r_in1, r_in2);
  //       state = MOVING_FORWARD;
  //     } else {
  //       turnLeft();
  //     }
  //     break;

  //   case TURNING_RIGHT:
  //     // Stop turning when front becomes clear
  //     if (!F) {
  //       stopMotors();
  //       moveForward(l_in1, l_in2);
  //       moveForward(r_in1, r_in2);
  //       state = MOVING_FORWARD;
  //     } else {
  //       turnRight();
  //     }
  //     break;
  // }

  
  // moveForward(l_in1, l_in2);
  // moveForward(r_in1, r_in2);
  // // delay(5000);
  // // stopMotors();
  // delay(3000);
  // reverseMotor(l_in1, l_in2);
  // reverseMotor(r_in1, r_in2);
  // delay(3000);
  // // stopMotors();

}

bool isWall(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  
  distance = duration * SOUND_SPEED/2; // in cm

  if(distance < WALL_DIST)
    return true;
  return false;
}

// void turnTo(Dir d);
// void moveOneCell();

void moveForward(int in1, int in2) {
  digitalWrite(in1, HIGH);
  Serial.println("Forward");

  ledcWrite(in2, 0);     // Fast
}

void reverseMotor(int in1, int in2) {
  digitalWrite(in1, LOW);  
  Serial.println("Reverse");

  ledcWrite(in2, 512);
}

void turnLeft() {
  moveForward(r_in1, r_in2);
  reverseMotor(l_in1, l_in2);
  Serial.println("Left");
}

void turnRight() {
  moveForward(l_in1, l_in2);
  reverseMotor(r_in1, r_in2);
  Serial.println("Right");
}

void stopMotors() {
  digitalWrite(l_in1, LOW);
  digitalWrite(r_in1, LOW);
  Serial.println("Stopped.");

  ledcWrite(l_in2, 0);     // Fast
  ledcWrite(r_in2, 0);
}

int getEncoders() {
  static uint32_t lastPrint = 0;

  if (millis() - lastPrint >= 200) {
    lastPrint = millis();

    uint32_t period1, period2;
    uint32_t last1, last2;

    noInterrupts();
    period1 = pulsePeriod1;
    period2 = pulsePeriod2;
    last1 = lastMicros1;
    last2 = lastMicros2;
    interrupts();

    // -------- Encoder 1 RPM --------
    float rpm1 = 0;
    if (!(period1 == 0 || (micros() - last1) > 500000)) {
      rpm1 = 60e6 / (period1 * PULSES_PER_REV);
      if (rpm1 > MAX_RPM) rpm1 = MAX_RPM;
    }

    // -------- Encoder 2 RPM --------
    float rpm2 = 0;
    if (!(period2 == 0 || (micros() - last2) > 500000)) {
      rpm2 = 60e6 / (period2 * PULSES_PER_REV);
      if (rpm2 > MAX_RPM) rpm2 = MAX_RPM;
    }

    Serial.print("RPM1: ");
    Serial.print(rpm1);
    Serial.print(" | RPM2: ");
    Serial.println(rpm2);

    return rpm1, rpm2;
  }
}