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

// LoRa pins
#define ss 5
#define rst 14
#define dio0 2

byte LocalAddress = 0x01; // Master address

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Master Ready");
  display.display();

  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Init Fail");
    display.display();
    while (true);
  }

  Serial.println("LoRa Receiver ready.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte length = LoRa.read();

    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    if (recipient != LocalAddress) return;

    Serial.println("Received: " + incoming);

    // Expected format: "Bin 01/Full"
    int sepIndex = incoming.indexOf('/');
    if (sepIndex == -1) {
      Serial.println("Invalid format");
      return;
    }

    String binDetail = incoming.substring(0, sepIndex);     // "Bin 01"
    String status = incoming.substring(sepIndex + 1);       // "Full"

    // Display on OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(binDetail);   // First line: Bin detail

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print("Status: ");
    display.println(status);      // Second line: Status

    display.display();
  }
}
