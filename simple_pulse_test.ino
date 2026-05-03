/*
 * Simple Pulse Test - Hold at one position for 5 seconds
 * Test different pulse widths one at a time
 */

const int SERVO_PIN = 8;

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(SERVO_PIN, OUTPUT);

  Serial.println("\n=== Simple Pulse Test ===\n");
  Serial.println("Each test: Hold position for 5 seconds\n");

  // Test 1: 500µs (should be 0°)
  Serial.println("Test 1: 500µs (0°)");
  sendPulse(500, 5);
  delay(2000);

  // Test 2: 1050µs (should be ~90°)
  Serial.println("Test 2: 1050µs (~90°)");
  sendPulse(1050, 5);
  delay(2000);

  // Test 3: 1550µs (should be ~180°)
  Serial.println("Test 3: 1550µs (~180°)");
  sendPulse(1550, 5);
  delay(2000);

  // Test 4: 2100µs (should be ~270°)
  Serial.println("Test 4: 2100µs (~270°)");
  sendPulse(2100, 5);
  delay(2000);

  // Test 5: 2600µs (should be full 270°)
  Serial.println("Test 5: 2600µs (full)");
  sendPulse(2600, 5);
  delay(2000);

  Serial.println("\n=== Done ===");
  Serial.println("Did servo move between positions?");
  Serial.println("How many degrees total?");
}

void loop() {
  delay(1000);
}

// Send pulse width for N seconds (N*50 pulses @ 20ms)
void sendPulse(int pulse, int seconds) {
  int pulses = seconds * 50;

  for (int i = 0; i < pulses; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pulse);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pulse);
  }

  Serial.println("→ Done holding");
}
