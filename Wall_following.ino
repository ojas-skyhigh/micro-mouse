
const int r_trigPin = 5;
const int r_echoPin = 18;

const int l_trigPin = 14;
const int l_echoPin = 32;

const int f_trigPin = 21;
const int f_echoPin = 19;


const int IN1 = 13;
const int IN2 = 25;
const int IN3 = 26;
const int IN4 = 27;

const int ENA = 12;   // PWM left
const int ENB = 15;   // PWM

// -------- PWM CONFIG --------
const int pwmFreq = 1000;
const int pwmResolution = 8;   // 0–255


#define SOUND_SPEED 0.034
#define WALL_DIST 15.00 //Calibrate

#define RIGHT_SPD 100 
#define LEFT_SPD 100

#define RIGHT_TURN_SPD 90
#define LEFT_TURN_SPD 90
#define TURN_DELAY 300
#define EMERGENCY_STOP_DIST 4
#define TARGET_LEFT_DIST 7.80   // center of corridor 
#define KP 8              // proportional gain (TUNE)

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
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcAttach(ENA, pwmFreq, pwmResolution);
  ledcAttach(ENB, pwmFreq, pwmResolution);  
  
  pinMode(r_trigPin, OUTPUT); 
  pinMode(r_echoPin, INPUT); 
  pinMode(l_trigPin, OUTPUT); 
  pinMode(l_echoPin, INPUT); 
  pinMode(f_trigPin, OUTPUT); 
  pinMode(f_echoPin, INPUT); 

  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  bool F = isWall(f_trigPin, f_echoPin); // wall detection, either ir or ultrasonic or tof idk
  bool L = isWall(l_trigPin, l_echoPin);
  bool R = isWall(r_trigPin, r_echoPin);

  // if (emergencyStop()) {
  //   delay(50);
  //   return;
  // }

  switch (state) {
    case MOVING_FORWARD:
      if (!L) {
        delay(TURN_DELAY);
        state = TURNING_LEFT;
      }
      else if (F) {
        state = TURNING_RIGHT;
      }
      else {
        moveForward(IN1, IN2, ENA, LEFT_SPD);
        moveForward(IN3, IN4, ENB, RIGHT_SPD);
        // followLeftWall();
      }
      break;

    case TURNING_LEFT:
      // Stop turning when left wall appears again
      if (L) {
        stopMotors();
        moveForward(IN1, IN2, ENA, LEFT_SPD);
        moveForward(IN3, IN4, ENB, RIGHT_SPD);
        state = MOVING_FORWARD;
      } else {
        stopMotors();
        turnLeft();
        // delay(400);
      }
      break;  

    case TURNING_RIGHT:
      // Stop turning when front becomes clear
      if (!F) {
        stopMotors();
        moveForward(IN1, IN2, ENA, LEFT_SPD);
        moveForward(IN3, IN4, ENB, RIGHT_SPD);
        state = MOVING_FORWARD;

      } else {
        stopMotors();
        turnRight();
        // delay(400);
      }
      break;
  }


}


bool isWall(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 25000); // 25ms timeout
  if (duration == 0) return false;
  
  distance = duration * SOUND_SPEED/2; // in cm

  if(distance < WALL_DIST)
    return true;
  return false;
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 25000); // 25ms timeout
  if (duration == 0) return 100.0;
  distance = duration * SOUND_SPEED/2; // in cm
  return distance;
}

void moveForward(int in1, int in2, int en, int spd) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  Serial.println("Forward");
  ledcWrite(en, spd);
}

void reverseMotor(int in1, int in2, int en, int spd) {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  Serial.println("Reverse");

  ledcWrite(en, spd);
}

void turnLeft() {
  reverseMotor(IN3, IN4, ENB, RIGHT_TURN_SPD);
  moveForward(IN1, IN2, ENA, LEFT_TURN_SPD);
  Serial.println("Left");
}

void turnRight() {
  reverseMotor(IN1, IN2, ENA, LEFT_TURN_SPD);
  moveForward(IN3, IN4, ENB, RIGHT_TURN_SPD);
  Serial.println("Right");
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);

  Serial.println("Stop");
}

void followLeftWall() {
  float leftDist = getDistance(l_trigPin, l_echoPin);

  float error = TARGET_LEFT_DIST - leftDist;
  float correction = KP * error;

  int leftSpeed  = LEFT_SPD - correction;
  int rightSpeed = RIGHT_SPD + correction;

  // Clamp speeds
  leftSpeed  = constrain(leftSpeed,  0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  moveForward(IN1, IN2, ENA, leftSpeed);
  moveForward(IN3, IN4, ENB, rightSpeed);
}

bool emergencyStop() {
  float frontDist = getDistance(f_trigPin, f_echoPin);

  if (frontDist <= EMERGENCY_STOP_DIST) {
    stopMotors();
    Serial.println("EMERGENCY STOP !!!");
    return true;
  }
  return false;
}
