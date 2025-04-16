// OLED libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <LoRa.h>

// OLED display size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Define LoRa pins
#define MOSI 27
#define SCLK 5
#define CS 18
#define DIO0 26
#define RST 23
#define MISO 19

// Define OLED pins
#define SDA 21
#define SCL 22
#define OLED_RST -1

// Define 915MHz band
#define BAND 915E6

// Packet counter
int counter = 0;

// Initialize the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup() {
  Serial.begin(115200);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Correct function call
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Halt execution
  }
  display.clearDisplay();
  display.setTextColor(WHITE); // Correct function name
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("LoRa Sender");
  display.display();

  Serial.println("LoRa Sender Test");

  // Initialize SPI and LoRa
  SPI.begin(SCLK, MISO, MOSI, CS); // SPI pins
  LoRa.setPins(CS, RST, DIO0);     // LoRa pins

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed.");
    while (1); // Halt execution if LoRa initialization fails
  }

  Serial.println("LoRa Initializing OK!");
  display.setCursor(0, 10);
  display.println("LoRa Initializing OK!");
  display.display();
  delay(2000);
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter); // Correct function name

  // LoRa packet sending
  LoRa.beginPacket();
  LoRa.print("Rocco ");
  LoRa.print(counter);
  LoRa.endPacket();

  // Update OLED display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("LoRa Sender");
  display.setCursor(0, 20);
  display.println("LoRa packet sent.");
  display.setCursor(0, 30);
  display.print("Counter:");
  display.setCursor(50, 30);
  display.print(counter);
  display.display();

  counter++;
  delay(10000);
}
