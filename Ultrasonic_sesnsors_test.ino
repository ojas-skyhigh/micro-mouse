
const int r_trigPin = 5;
const int r_echoPin = 18;

const int l_trigPin = 14;
const int l_echoPin = 32;

const int f_trigPin = 21;
const int f_echoPin = 19;


//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define WALL_DIST 10.00

long duration;
float distanceCm;

void setup() {
  Serial.begin(115200); 
  pinMode(r_trigPin, OUTPUT); 
  pinMode(r_echoPin, INPUT); 
  pinMode(l_trigPin, OUTPUT); 
  pinMode(l_echoPin, INPUT); 
  pinMode(f_trigPin, OUTPUT); 
  pinMode(f_echoPin, INPUT); 
}

void loop() {

  bool l=wallLeft();
  bool r=wallRight();
  bool f=wallFront();

  Serial.print("L: ");
  Serial.print(l);
  Serial.print("  F: ");
  Serial.print(f);
  Serial.print("  R: ");
  Serial.println(r);
  delay(25);
}

bool wallLeft() {
  digitalWrite(l_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(l_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(l_trigPin, LOW);
  
  duration = pulseIn(l_echoPin, HIGH);
  
  distanceCm = duration * SOUND_SPEED/2;

  if(distanceCm < WALL_DIST)
    return true;
  return false;
}

bool wallFront() {
  digitalWrite(f_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(f_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(f_trigPin, LOW);
  
  duration = pulseIn(f_echoPin, HIGH);
  
  distanceCm = duration * SOUND_SPEED/2;

  if(distanceCm < WALL_DIST)
    return true;
  return false;
}

bool wallRight() {
  digitalWrite(r_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(r_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(r_trigPin, LOW);
  
  duration = pulseIn(r_echoPin, HIGH);
  
  distanceCm = duration * SOUND_SPEED/2;

  if(distanceCm < WALL_DIST)
    return true;
  return false;
}