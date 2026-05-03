/*
 * Interactive Pulse Width Tester
 * Type pulse width (400-2600) to test different values
 */

const int SERVO_PIN = 8;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n=== Pulse Width Tester ===\n");
  Serial.println("Instructions:");
  Serial.println("- Type a pulse width (400-2600)");
  Serial.println("- Servo will move for 1 second");
  Serial.println("- Observe servo position carefully");
  Serial.println("\nExamples:");
  Serial.println("  500    - Test 500µs");
  Serial.println("  1000   - Test 1000µs");
  Serial.println("  1500   - Test 1500µs");
  Serial.println("  2000   - Test 2000µs\n");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int pulse = input.toInt();

    if (pulse < 400 || pulse > 2600) {
      Serial.println("Invalid! Range: 400-2600µs");
      return;
    }

    Serial.print("Testing ");
    Serial.print(pulse);
    Serial.println("µs for 1 second...");

    // Send pulses for 1 second
    for (int i = 0; i < 50; i++) {
      digitalWrite(SERVO_PIN, HIGH);
      delayMicroseconds(pulse);
      digitalWrite(SERVO_PIN, LOW);
      delayMicroseconds(20000 - pulse);
    }

    Serial.println("Done! Enter next pulse width:");
  }
}
