/*
 * Servo Reciprocal Motion Test - 0° ↔ 270°
 * Continuous back and forth motion for debugging
 */

const int SERVO_PIN = 8;
const int PULSE_MIN = 500;      // 0°
const int PULSE_MAX = 2600;     // 270°
const int ANGLE_MAX = 270;

unsigned long lastMoveTime = 0;
const int MOVE_INTERVAL = 2000;  // 2 seconds per position

int targetAngles[] = {0, 90, 180, 270, 180, 90};  // Sequence
int currentIndex = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n========================================");
  Serial.println("   Reciprocal Motion Test");
  Serial.println("========================================");
  Serial.println("Sequence: 0° → 90° → 180° → 270° → 180° → 90° → repeat");
  Serial.println("Each position: 2 seconds");
  Serial.println("\nWatch servo carefully!");
  Serial.println("========================================\n");

  // Start with 0°
  moveToAngle(0);
}

void loop() {
  unsigned long now = millis();

  // Move to next angle every MOVE_INTERVAL
  if (now - lastMoveTime >= MOVE_INTERVAL) {
    currentIndex++;
    if (currentIndex >= 6) {
      currentIndex = 0;
    }

    int angle = targetAngles[currentIndex];
    moveToAngle(angle);
    lastMoveTime = now;
  }

  delay(50);  // Small delay to prevent CPU hogging
}

void moveToAngle(int angle) {
  Serial.print("[");
  Serial.print(millis() / 1000);
  Serial.print("s] Moving to ");
  Serial.print(angle);
  Serial.print("°... ");

  sendPWMPulse(angle);

  Serial.println("Done");
}

int angleToPulse(int angle) {
  // Direct mapping: 500µs = 0°, 2600µs = 270°
  int pulse = PULSE_MIN + (angle * (PULSE_MAX - PULSE_MIN) / ANGLE_MAX);

  Serial.print("(pulse: ");
  Serial.print(pulse);
  Serial.print("µs) ");

  return pulse;
}

void sendPWMPulse(int angle) {
  int pulseMicros = angleToPulse(angle);

  // Send 100 pulses (2 seconds @ 20ms period)
  for (int i = 0; i < 100; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseMicros);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulseMicros);
  }
}
