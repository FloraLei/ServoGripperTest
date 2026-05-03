/*
 * Wheeltech 270° Servo Control - FIXED VERSION
 * Using direct pulse values instead of calculation
 */

const int SERVO_PIN = 8;

// Direct mapping table: angle → pulse width
// Verified working: 500µs (0°) to 2600µs (270°)
int getPulseForAngle(int angle) {
  // Linear interpolation: pulse = 500 + (angle * 2100 / 270)
  // But using explicit calculation to avoid integer issues

  if (angle == 0) return 500;
  if (angle == 45) return 850;
  if (angle == 90) return 1200;
  if (angle == 135) return 1550;
  if (angle == 180) return 1900;
  if (angle == 225) return 2250;
  if (angle == 270) return 2600;

  // For values in between, interpolate
  int lower = (angle / 45) * 45;
  int upper = lower + 45;
  int lowerPulse = getPulseForAngle(lower);
  int upperPulse = getPulseForAngle(upper);

  return lowerPulse + ((upperPulse - lowerPulse) * (angle - lower) / 45);
}

int currentAngle = 135;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n========================================");
  Serial.println("   Wheeltech 270° - FIXED");
  Serial.println("========================================");
  Serial.println("Range: 0-270°");
  Serial.println("Status: Working with verified pulse values");
  Serial.println("========================================\n");

  sendPWMPulse(135);
  delay(1000);

  printHelp();
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() == 0) return;

    if (input == "h" || input == "?") {
      printHelp();
      return;
    }

    if (input == "pos") {
      Serial.print("Angle: ");
      Serial.print(currentAngle);
      Serial.println("°");
      return;
    }

    if (input == "test") {
      runTest();
      return;
    }

    int angle = input.toInt();

    if (angle < 0 || angle > 270) {
      Serial.println("✗ Range: 0-270°");
      return;
    }

    moveToAngle(angle);
  }

  delay(10);  // Prevent loop from running too fast
}

void moveToAngle(int angle) {
  Serial.print("→ ");
  Serial.print(angle);
  Serial.print("°");

  int pulse = getPulseForAngle(angle);
  Serial.print(" (");
  Serial.print(pulse);
  Serial.print("µs)... ");

  currentAngle = angle;
  sendPWMPulse(angle);

  Serial.println("✓");
}

void sendPWMPulse(int angle) {
  int pulseMicros = getPulseForAngle(angle);

  // Send 200 pulses = 4 seconds @ 20ms period
  for (int i = 0; i < 200; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseMicros);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulseMicros);
  }
}

void runTest() {
  Serial.println("\n=== Test ===");

  moveToAngle(0);
  delay(1000);

  moveToAngle(90);
  delay(1000);

  moveToAngle(180);
  delay(1000);

  moveToAngle(270);
  delay(1000);

  Serial.println("✓ Done\n");
}

void printHelp() {
  Serial.println("Commands: 0-270 | test | pos | h\n");
}
