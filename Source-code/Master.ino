#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RA-02 LoRa Pinout for ESP32
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_SS     5
#define LORA_RST   14
#define LORA_DIO0  26

byte LocalAddress = 0x01;  // Master address

void setup() {
  Serial.begin(115200);
  Serial.println("Serial initialized.");

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 OLED init failed!");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Master Ready");
  display.display();
  Serial.println("OLED initialized.");

  // LoRa SPI & init
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  Serial.println("SPI and LoRa pins set.");

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Init Fail");
    display.display();
    while (true);
  }

  Serial.println("LoRa init succeeded.");
  Serial.print("Local address: 0x");
  Serial.println(LocalAddress, HEX);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Read packet metadata
    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte length = LoRa.read();

    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    // Check destination
    if (recipient != LocalAddress) {
      Serial.println("Message not for this device.");
      return;
    }

    Serial.println("=== Incoming LoRa Message ===");
    Serial.print("From: 0x"); Serial.println(sender, HEX);
    Serial.print("Message: "); Serial.println(incoming);
    Serial.print("RSSI: "); Serial.println(LoRa.packetRssi());
    Serial.println("==============================");

    // Parse and display message
    int sepIndex = incoming.indexOf('/');
    if (sepIndex == -1) {
      Serial.println("Invalid message format.");
      return;
    }

    String binDetail = incoming.substring(0, sepIndex);
    String status = incoming.substring(sepIndex + 1);

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(binDetail);

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print("Status: ");
    display.print(status);
    display.display();
    sendAck(sender);
  }
}

void sendAck(byte recipient) {
  String ackMsg = "ACK from Master";
  LoRa.beginPacket();
  LoRa.write(recipient);       // To sender
  LoRa.write(LocalAddress);    // From master
  LoRa.write(ackMsg.length());
  LoRa.print(ackMsg);
  LoRa.endPacket();
  Serial.println("ACK sent.");
}
