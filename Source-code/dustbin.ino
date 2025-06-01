// Define GPIO pins
#define TRIG_PIN 5   
#define ECHO_PIN 18 
long duration;
float distance;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Send 10us HIGH pulse to trigger the sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo time in microseconds (with 25 ms timeout = max ~4.3 m)
  duration = pulseIn(ECHO_PIN, HIGH, 25000); // timeout in microseconds

  if (duration == 0) {
    Serial.println("No object detected (too close or out of range)");
  } else {
    // Convert time to distance in cm
    distance = duration * 0.034 / 2;
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  delay(500); // Wait half a second between readings
}
