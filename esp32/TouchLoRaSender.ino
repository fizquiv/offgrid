#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <LoRa.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Use -1 if your OLED doesn't have a reset pin

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Touch pin
#define TOUCH_PIN 14  // Pin 14 used for touch input

// LoRa settings
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DI0 26
#define LORA_BAND 915E6  // Frequency for LoRa 

// Variables
bool lastButtonState = HIGH; // Initial touch state (not touched)
int messageCount = 0;        // Counter for the message

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize touch pin (no need to set pinMode, touchRead handles it)
  pinMode(TOUCH_PIN, INPUT);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Initialize LoRa
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("LoRa initialization failed!");
    while (true);
  }
  Serial.println("LoRa initialized.");
  displayMessage("LoRa Ready");
}

void loop() {
  // Read touch state
  int touchValue = touchRead(TOUCH_PIN);  // Read the value from the touch pin

  // Check if the touch pin is being touched (when touch value is low)
  if (touchValue < 40 && lastButtonState == HIGH) {  // Adjust the threshold if necessary
    messageCount++;  // Increment the message count
    String message = "Count: " + String(messageCount);  // Create the message with the current count
    sendLoRaMessage(message);
    displayMessage(message);  // Display message on OLED
    Serial.println("Message sent: " + message);
  }

  lastButtonState = (touchValue < 40) ? LOW : HIGH; // Update button state
}

void sendLoRaMessage(String message) {
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

void displayMessage(String message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sent Message:");
  display.println(message);  // Display the message on OLED
  display.display();
}
