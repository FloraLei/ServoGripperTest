/*
 * Servo Range Finder - Find actual pulse width range
 *
 * This script tests different pulse widths (400-2600µs)
 * to find where your servo actually rotates.
 *
 * IMPORTANT: Watch the servo carefully and note positions!
 */

const int SERVO_PIN = 8;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n=== Servo Range Finder ===\n");
  Serial.println("Testing pulse widths: 400-2600µs");
  Serial.println("Watch servo carefully for each pulse!");
  Serial.println("\nPulse (µs) | Servo Position");
  Serial.println("-----------|------------------\n");

  // Test in 100µs steps
  for (int pulse = 400; pulse <= 2600; pulse += 100) {
    testPulse(pulse);
    delay(1500);  // Wait between tests so you can observe
  }

  Serial.println("\n=== Analysis ===");
  Serial.println("Based on servo movement above:");
  Serial.println("- Note minimum pulse where servo starts moving");
  Serial.println("- Note maximum pulse where servo stops moving");
  Serial.println("- Calculate: PULSE_MIN and PULSE_MAX");
  Serial.println("\nUpdate code with actual values!");
}

void loop() {
  delay(1000);
}

void testPulse(int pulseMicros) {
  // Send 40 pulses (0.8 seconds)
  for (int i = 0; i < 40; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulseMicros);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulseMicros);
  }

  Serial.print(pulseMicros);
  Serial.println("     | Watch servo position");
}
