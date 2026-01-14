
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


#define SOUND_SPEED 0.034
#define WALL_DIST 10.00

#define RIGHT_SPD 100 //larger the value, slower the speed
#define LEFT_SPD 100
#define RIGHT_TURN_SPD 512
#define LEFT_TURN_SPD 512 //larger the value, slower the turn (also affects reverse speed)

#define TARGET_LEFT_DIST 8.0   // center of corridor 
#define KP 15.0                // proportional gain (TUNE)

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

}

void loop() {
  // put your main code here, to run repeatedly:
  bool F = isWall(f_trigPin, f_echoPin); // wall detection, either ir or ultrasonic or tof idk
  bool L = isWall(l_trigPin, l_echoPin);
  bool R = isWall(r_trigPin, r_echoPin);

  switch (state) {

    case MOVING_FORWARD:
      if (!L) {
        // turnLeft();
        state = TURNING_LEFT;
      }
      else if (F) {
        // turnRight();
        state = TURNING_RIGHT;
      }
      else {
        // moveForward(l_in1, l_in2, LEFT_SPD);
        // moveForward(r_in1, r_in2, RIGHT_SPD);
        followLeftWall();
      }
      break;

    case TURNING_LEFT:
      // Stop turning when left wall appears again
      if (L) {
        stopMotors();
        moveForward(l_in1, l_in2, LEFT_SPD);
        moveForward(r_in1, r_in2, RIGHT_SPD);
        state = MOVING_FORWARD;
      } else {
        turnLeft();
      }
      break;

    case TURNING_RIGHT:
      // Stop turning when front becomes clear
      if (!F) {
        stopMotors();
        moveForward(l_in1, l_in2, LEFT_SPD);
        moveForward(r_in1, r_in2, RIGHT_SPD);
        state = MOVING_FORWARD;
      } else {
        turnRight();
      }
      break;
  }

  moveForward(l_in1, l_in2, LEFT_SPD);
  moveForward(r_in1, r_in2, RIGHT_SPD);

  delay(3000);
  reverseMotor(l_in1, l_in2, LEFT_TURN_SPD);
  reverseMotor(r_in1, r_in2, RIGHT_TURN_SPD);
  delay(3000);
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

void moveForward(int in1, int in2, int spd) {
  digitalWrite(in1, HIGH);
  Serial.println("Forward");

  ledcWrite(in2, spd);     // set a speed here
}

void reverseMotor(int in1, int in2, int spd) {
  int reverse_spd = 1024-spd;
  digitalWrite(in1, LOW);  
  Serial.println("Reverse");

  ledcWrite(in2, reverse_spd);
}

void turnLeft() {
  moveForward(r_in1, r_in2, RIGHT_TURN_SPD);
  reverseMotor(l_in1, l_in2, LEFT_TURN_SPD);
  Serial.println("Left");
}

void turnRight() {
  moveForward(l_in1, l_in2, LEFT_TURN_SPD);
  reverseMotor(r_in1, r_in2, RIGHT_TURN_SPD);
  Serial.println("Right");
}

void stopMotors() {
  digitalWrite(l_in1, LOW);
  digitalWrite(r_in1, LOW);
  Serial.println("Stopped.");

  ledcWrite(l_in2, 0);     // Fast
  ledcWrite(r_in2, 0);
}

void followLeftWall() {
  float leftDist = getDistance(l_trigPin, l_echoPin);

  float error = TARGET_LEFT_DIST - leftDist;
  float correction = KP * error;

  int leftSpeed  = LEFT_SPD - correction;
  int rightSpeed = RIGHT_SPD + correction;

  // Clamp speeds
  leftSpeed  = constrain(leftSpeed,  0, 1023);
  rightSpeed = constrain(rightSpeed, 0, 1023);

  moveForward(l_in1, l_in2, leftSpeed);
  moveForward(r_in1, r_in2, rightSpeed);
}
