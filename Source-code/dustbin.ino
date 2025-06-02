#include <SPI.h>
#include <LoRa.h>

// Ultrasonic Sensor Pins
#define TRIG_PIN 27
#define ECHO_PIN 25
#define BIN_HEIGHT_CM 21.15

// RA-02 LoRa Pinout for ESP32
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_SS     5
#define LORA_RST   14
#define LORA_DIO0  26

String Message = "";
String DeviceID = "Bin 02";
byte LocalAddress = 0x03;             // Slave address
byte Destination_Master = 0x01;       // Master address

void sendMessage(String Outgoing, byte Destination) {
  LoRa.beginPacket();
  LoRa.write(Destination);           // Destination
  LoRa.write(LocalAddress);          // Sender
  LoRa.write(Outgoing.length());     // Length
  LoRa.print(Outgoing);              // Payload
  LoRa.endPacket();
}

void sendLoRaData(String msg) {
  Message = DeviceID + "/" + msg;
  Serial.println("Sending LoRa message: " + Message);
  sendMessage(Message, Destination_Master);
}

void receiveAckIfAvailable() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte length = LoRa.read();

    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    if (recipient != LocalAddress) {
      Serial.println("ACK not for this device.");
      return;
    }

    Serial.println("=== ACK Received ===");
    Serial.print("From: 0x"); Serial.println(sender, HEX);
    Serial.print("ACK Message: "); Serial.println(incoming);
    Serial.print("RSSI: "); Serial.println(LoRa.packetRssi());
    Serial.println("====================");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Serial initialized.");

  // Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Ultrasonic sensor pins configured.");

  // LoRa SPI
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  Serial.println("SPI initialized with custom pins.");

  // LoRa
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  Serial.println("LoRa pins set.");

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (1);
  }
  Serial.println("LoRa init succeeded.");
  Serial.print("Local address: 0x");
  Serial.println(LocalAddress, HEX);
}

long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  return pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms
}

float getDistanceCM() {
  long duration = readUltrasonic();
  float distance = duration * 0.0343 / 2.0;
  Serial.print("Measured distance (cm): ");
  Serial.println(distance);
  return distance;
}

float calculateFillPercentage(float distanceCM) {
  distanceCM = constrain(distanceCM, 0, BIN_HEIGHT_CM);
  float filled = BIN_HEIGHT_CM - distanceCM;
  float percent = (filled / BIN_HEIGHT_CM) * 100.0;
  Serial.print("Fill percentage: ");
  Serial.println(percent);
  return percent;
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

  Serial.print("Bin status: ");
  Serial.println(status);

  sendLoRaData(status);
  delay(1000); // Wait a bit before trying to receive ACK
  receiveAckIfAvailable();
  delay(2000); // Delay before next measurement
}
