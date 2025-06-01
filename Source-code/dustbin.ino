#define TRIG_PIN 5
#define ECHO_PIN 18
#define BIN_HEIGHT_CM 25.95  // Updated height of bin in cm

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  return duration;
}

float getDistanceCM() {
  long duration = readUltrasonic();
  float distance = duration * 0.0343 / 2;
  return distance;
}

float calculateFillPercentage(float distanceCM) {
  distanceCM = constrain(distanceCM, 0, BIN_HEIGHT_CM);
  float filled = BIN_HEIGHT_CM - distanceCM;
  return (filled / BIN_HEIGHT_CM) * 100.0;
}

void loop() {
  float distance = getDistanceCM();
  float fillPercent = calculateFillPercentage(distance);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Fill: ");
  Serial.print(fillPercent);
  Serial.println(" %");

  delay(2000);
}
