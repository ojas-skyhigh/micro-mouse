const int l_in2 = 27;   // speed
const int l_in1 = 26;   // direction

const int r_in2 = 13;   // speed
const int r_in1 = 25;   // direction

const int pwmFreq = 500;
const int pwmResolution = 10; 

void setup() {
  Serial.begin(115200);

  pinMode(l_in1, OUTPUT);
  pinMode(r_in1, OUTPUT);

  ledcAttach(l_in2, pwmFreq, pwmResolution);
  ledcAttach(r_in2, pwmFreq, pwmResolution);

}

void loop() {

  digitalWrite(l_in1, HIGH);
  digitalWrite(r_in1, HIGH);
  Serial.println("Forward");

  ledcWrite(l_in2, 0);     // Fast
  ledcWrite(r_in2, 0);  
  // delay(1000);

  // ledcWrite(l_in2, 100);   // Slow
  // ledcWrite(r_in2, 100);
  // delay(1000);

  // ledcWrite(l_in2, 200);   // Slower
  // ledcWrite(r_in2, 200);
  // delay(1000);

  // ledcWrite(l_in2, 300);   // Slower x2
  // ledcWrite(r_in2, 300);
  // delay(1000);

  // ledcWrite(l_in2, 1023);  // Stop
  // ledcWrite(r_in2, 1023);
  // delay(1000);

  // // -------- Reverse --------
  // digitalWrite(l_in1, LOW);
  // digitalWrite(r_in1, LOW);

  // Serial.println("Reverse");

  // ledcWrite(l_in2, 1023);   // Fast
  // ledcWrite(r_in2, 1023);
  // delay(1000);

  // ledcWrite(l_in2, 600);   // Slow
  // ledcWrite(r_in2, 600);
  // delay(1000);

  // ledcWrite(l_in2, 300);   // Slower
  // ledcWrite(r_in2, 300);
  // delay(1000);

  // ledcWrite(l_in2, 0);     // Stop
  // ledcWrite(r_in2, 0);
  // delay(1000);
}

void moveForward() {

}
