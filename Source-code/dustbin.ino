#include <SPI.h>
#include <LoRa.h>

#define TRIG_PIN 26
#define ECHO_PIN 27
#define BIN_HEIGHT_CM 27

#define ss 5
#define rst 14
#define dio0 2

String Message = "";
String DeviceID = "Bin 01";
byte LocalAddress = 0x02;
byte Destination_Master = 0x01;

void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();
  LoRa.write(Destination);
  LoRa.write(LocalAddress);
  LoRa.write(Outgoing.length());
  LoRa.print(Outgoing);
  LoRa.endPacket();
}

void sendLoRaData(String msg) {
  Message = DeviceID + "/" + msg;
  Serial.println(Message);
  sendMessage(Message, Destination_Master);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LoRa Setup
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("LoRa init succeeded.");
}

long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  return pulseIn(ECHO_PIN, HIGH, 30000);
}

float getDistanceCM() {
  long duration = readUltrasonic();
  return duration * 0.0343 / 2.0;
}

float calculateFillPercentage(float distanceCM) {
  distanceCM = constrain(distanceCM, 0, BIN_HEIGHT_CM);
  float filled = BIN_HEIGHT_CM - distanceCM;
  return (filled / BIN_HEIGHT_CM) * 100.0;
}

String getStatus(float distanceCM) {
  if (distanceCM < 10.0) return "Full";
  else if (distanceCM < 15.0) return "High";
  else if (distanceCM < 20.0) return "Mid";
  else return "Empty";
}

void loop() {
  float distance = getDistanceCM();
  float fillPercent = calculateFillPercentage(distance);

  String status = getStatus(distance);
    Serial.println(distance);
    Serial.println(fillPercent);
  Serial.println(status);
  sendLoRaData(status);
  delay(1000);
}
