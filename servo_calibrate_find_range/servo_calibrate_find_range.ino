/*
 * Servo Limit Finder - Interactive + INA219
 *
 * Known limits: open=130deg (1511us), close=200deg (2055us).
 * Goal : find the other limit by stepping the servo.
 *        INA219 current reading helps confirm mechanical stop:
 *        current spikes noticeably when the servo stalls against a limit.
 *
 * INA219 wiring:
 *   VCC  → 3.3V or 5V
 *   GND  → GND
 *   SDA  → A4
 *   SCL  → A5
 *   VIN+ → 5V supply positive
 *   VIN- → servo VCC (in series)
 *
 * Serial Monitor: set "Newline", baud 9600
 *
 * Keys:
 *   +   step UP   (default 5deg, fine=1deg, coarse=10deg)
 *   -   step DOWN
 *   f   fine mode   (1deg steps)
 *   n   normal mode (5deg steps)
 *   r   coarse mode (10deg steps)
 *   g   go to 135deg (known limit)
 *   0   go to 0deg
 *   z   go to 270deg
 *   p   print angle, pulse, voltage, current
 *   ?   print this menu
 */

#include <Wire.h>
#include <Adafruit_INA219.h>

const int SERVO_PIN   = 8;
const int PULSE_FOR[] = {500, 850, 1200, 1550, 1900, 2250, 2600};

// Current above this level after a step = likely hitting mechanical stop
const int STALL_CURRENT_MA = 400;

int currentAngle = 130;  // open limit
int stepSize     = 5;

Adafruit_INA219 ina219;
bool ina219Ready = false;

void setup() {
  Serial.begin(9600);
  delay(500);
  pinMode(SERVO_PIN, OUTPUT);

  Wire.begin();
  if (ina219.begin()) {
    ina219Ready = true;
    float v = ina219.getBusVoltage_V();
    Serial.print(F("[OK] INA219 ready  Bus: "));
    Serial.print(v, 2); Serial.println(F("V"));
  } else {
    Serial.println(F("[WARN] INA219 not found - continuing without current"));
  }

  printMenu();

  sendAngle(currentAngle);
  Serial.print(F("-> Parked at known limit: "));
  Serial.print(currentAngle);
  Serial.print(F("deg  ("));
  Serial.print(getPulse(currentAngle));
  Serial.println(F("us)"));
  Serial.println(F("Step with + / - to find the other limit."));
  Serial.println(F("Watch for [STALL] warning = current too high = mechanical stop."));
}

void loop() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  char key = cmd.charAt(0);

  if (key == '+') {
    currentAngle = constrain(currentAngle + stepSize, 0, 270);
    sendAngle(currentAngle);
    printStatus();
    checkStall();

  } else if (key == '-') {
    currentAngle = constrain(currentAngle - stepSize, 0, 270);
    sendAngle(currentAngle);
    printStatus();
    checkStall();

  } else if (key == 'f') {
    stepSize = 1;
    Serial.println(F("[MODE] Fine: 1deg steps"));

  } else if (key == 'n') {
    stepSize = 5;
    Serial.println(F("[MODE] Normal: 5deg steps"));

  } else if (key == 'r') {
    stepSize = 10;
    Serial.println(F("[MODE] Coarse: 10deg steps"));

  } else if (key == 'g') {
    currentAngle = 130;
    sendAngle(currentAngle);
    printStatus();
    Serial.println(F("^ Open limit (130deg)"));

  } else if (key == '0') {
    currentAngle = 0;
    sendAngle(currentAngle);
    printStatus();

  } else if (key == 'z') {
    currentAngle = 270;
    sendAngle(currentAngle);
    printStatus();

  } else if (key == 'p') {
    printStatus();

  } else if (key == '?') {
    printMenu();

  } else {
    Serial.print(F("[?] Unknown key: "));
    Serial.println(key);
  }
}

// ============ SERVO ============
int getPulse(int angle) {
  if (angle <= 0)   return 500;
  if (angle >= 270) return 2600;

  int seg = angle / 45;
  int rem = angle % 45;
  int lo  = PULSE_FOR[seg];
  int hi  = PULSE_FOR[seg + 1];
  return lo + ((hi - lo) * rem / 45);
}

void sendAngle(int angle) {
  int pw = getPulse(angle);
  for (int i = 0; i < 50; i++) {
    digitalWrite(SERVO_PIN, HIGH);
    delayMicroseconds(pw);
    digitalWrite(SERVO_PIN, LOW);
    delayMicroseconds(20000 - pw);
  }
}

// ============ INA219 ============
void printStatus() {
  Serial.print(F("Angle: ")); Serial.print(currentAngle);
  Serial.print(F("deg  |  Pulse: ")); Serial.print(getPulse(currentAngle));
  Serial.print(F("us  |  Step: ")); Serial.print(stepSize); Serial.print(F("deg"));

  if (ina219Ready) {
    float v  = ina219.getBusVoltage_V();
    float mA = ina219.getCurrent_mA();
    Serial.print(F("  |  "));
    Serial.print(v, 2); Serial.print(F("V  "));
    Serial.print(mA, 0); Serial.print(F("mA"));
  }
  Serial.println();
}

// Warn if current is high after a step — sign of stalling at a limit
void checkStall() {
  if (!ina219Ready) return;
  delay(80);  // let current settle after movement
  float mA = ina219.getCurrent_mA();
  if (mA > STALL_CURRENT_MA) {
    Serial.print(F("[STALL] "));
    Serial.print(mA, 0);
    Serial.println(F("mA  <- likely mechanical stop, step back!"));
  }
}

void printMenu() {
  Serial.println(F("\n=== Servo Limit Finder + INA219 ==="));
  Serial.println(F("  +  / -   step up / down"));
  Serial.println(F("  f        fine   (1deg)"));
  Serial.println(F("  n        normal (5deg)"));
  Serial.println(F("  r        coarse (10deg)"));
  Serial.println(F("  g        go to 130deg (open limit)"));
  Serial.println(F("  0 / z    go to 0deg / 270deg"));
  Serial.println(F("  p        print angle, pulse, V, mA"));
  Serial.println(F("  ?        this menu"));
  Serial.println(F("===================================\n"));
}
