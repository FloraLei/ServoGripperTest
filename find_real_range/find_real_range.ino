/*
 * Find Real Servo Range - Binary Search Style
 * Test key pulse widths to find actual working range
 */

const int SERVO_PIN = 8;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n=== Finding Real Servo Range ===\n");
  Serial.println("Testing key pulse widths...\n");

  // Test common servo ranges
  testPoint(900, "Common min");
  delay(2000);

  testPoint(1000, "");
  delay(2000);

  testPoint(1100, "");
  delay(2000);

  testPoint(1200, "");
  delay(2000);

  testPoint(1300, "");
  delay(2000);

  testPoint(1400, "");
  delay(2000);

  testPoint(1500, "Center");
  delay(2000);

  testPoint(1600, "");
  delay(2000);

  testPoint(1700, "");
  delay(2000);

  testPoint(1800, "");
  delay(2000);

  testPoint(1900, "");
  delay(2000);

  testPoint(2000, "Common max");
  delay(2000);

  testPoint(2100, "");
  delay(2000);

  testPoint(2200, "");
  delay(2000);

  Serial.println("\n=== Analysis ===");
  Serial.println("Observe the servo positions above.");
  Serial.println("Tell me:");
  Serial.println("1. Minimum pulse where servo moved most");
  Serial.println("2. Maximum pulse where servo moved most");
  Serial.println("3. How many degrees total range moved");
}

void loop() {
  delay(1000);
}

void testPoint(int pulse, const char* label) {
  Serial.print("Testing ");
  Serial.print(pulse);
  Serial.print("µs");
  if (strlen(label) > 0) {
    Serial.print(" (");
    Serial.print(label);
    Serial.print(")");
  }
  Serial.println(" - Watch servo position...");

  // Send pulses for 1.5 seconds
  for (int i = 0; i < 75; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulse);
  }
}
