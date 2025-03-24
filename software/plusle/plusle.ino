#define AIN1 11
#define AIN2 10
#define BIN1 6
#define BIN2 5
#define CH_A A3 // Channel A (forward/backward speed)
#define CH_B A4 // Channel B (left/right steering and speed)

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(CH_A, INPUT);
  pinMode(CH_B, INPUT);
}

void loop() {
  int pwmA = pulseIn(CH_A, HIGH);
  int pwmB = pulseIn(CH_B, HIGH);

  int baseSpeed = 0; // Base forward/backward speed
  int steerAdjustment = 0; // Adjustment for turning
  int speedA = 0;
  int speedB = 0;

  // Motor control for forward/backward movement
  if (pwmA > 1600) { // Threshold for forward
    baseSpeed = -map(pwmA, 1600, 2020, 0, 255); // Adjust speed
  } else if (pwmA < 1400) { // Threshold for backward
    baseSpeed = map(pwmA, 980, 1400, 255, 0); // Adjust speed
  }

  // Motor control for left/right steering
  if (pwmB > 1600) { // Threshold for right
    steerAdjustment = map(pwmB, 1600, 2020, 0, 255); // Adjust speed
  } else if (pwmB < 1400) { // Threshold for left
    steerAdjustment = -map(pwmB, 980, 1400, 255, 0); // Adjust speed
  }

  // Combine base speed with steering adjustment
  speedA = baseSpeed + steerAdjustment;
  speedB = baseSpeed - steerAdjustment;

  // Apply motor speeds
  if (speedA > 0) {
    analogWrite(AIN1, speedA);
    analogWrite(AIN2, 0);
  } else if (speedA < 0) {
    analogWrite(AIN1, 0);
    analogWrite(AIN2, -speedA);
  } else {
    analogWrite(AIN1, 255);
    analogWrite(AIN2, 255);
  }

  if (speedB > 0) {
    analogWrite(BIN1, speedB);
    analogWrite(BIN2, 0);
  } else if (speedB < 0) {
    analogWrite(BIN1, 0);
    analogWrite(BIN2, -speedB);
  } else {
    analogWrite(BIN1, 255);
    analogWrite(BIN2, 255);
  }

  delay(5); // Small delay for stability
}
