/*
 * Fine Range Search - ESP32 Version
 * Find exact working pulse range
 * Tests 1400-1600µs range in 10µs steps
 *
 * Uses GPIO 5 for servo (same as gripper_main_esp32.ino)
 */

const int SERVO_PIN = 5;  // GPIO 5 on ESP32

void setup() {
  Serial.begin(115200);  // ESP32 faster baud rate
  delay(1000);

  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n=== Fine Range Search (ESP32) ===\n");
  Serial.println("Testing 1400-1600µs in 10µs steps\n");

  // Test range around 1500µs (common center point)
  for (int pulse = 1400; pulse <= 1600; pulse += 10) {
    Serial.print("Pulse ");
    Serial.print(pulse);
    Serial.print("µs - ");

    // Send for 2 seconds
    for (int i = 0; i < 100; i++) {
      digitalWrite(SERVO_PIN, HIGH);
      delayMicroseconds(pulse);
      digitalWrite(SERVO_PIN, LOW);
      delayMicroseconds(20000 - pulse);
    }

    Serial.println("Position?");
    delay(1000);
  }

  Serial.println("\n=== Analysis ===");
  Serial.println("If servo moved, the range is much smaller!");
  Serial.println("Tell me:");
  Serial.println("- Min pulse: ____µs (leftmost)");
  Serial.println("- Max pulse: ____µs (rightmost)");
  Serial.println("- Total degrees moved: ____°");
}

void loop() {
  delay(1000);
}
