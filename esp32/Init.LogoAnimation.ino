#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// The text to animate
const char* logoText = "OffGrid";

// The start and target positions for each letter
int yPos[7]; // Array to store the current Y positions of each letter
const int targetY = 40; // Target Y position for all letters
int currentLetter = 0; // Tracks which letter is currently animating

// Starting X position for the first letter
int startX = 30;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  // Set initial positions for animation
  for (int i = 0; i < 7; i++) {
    if (i % 2 == 0) {
      yPos[i] = -10; // Even-indexed letters start above the screen
    } else {
      yPos[i] = SCREEN_HEIGHT + 10; // Odd-indexed letters start below the screen
    }
  }

  display.setFont(&FreeSerif9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  display.clearDisplay(); // Clear the screen for redrawing

  int currentX = startX; // X position for the first letter

  // Draw and animate letters
  for (int i = 0; i < 7; i++) {
    // Create a single-character string for the current letter
    char currentChar[2] = {logoText[i], '\0'};

    // Measure the width of the current character
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(currentChar, 0, 0, &x1, &y1, &w, &h);

    // If the letter is already in place, just draw it
    if (i < currentLetter) {
      display.setCursor(currentX, targetY);
      display.print(currentChar);
    } else if (i == currentLetter) {
      // Animate the current letter
      if (yPos[i] < targetY) {
        yPos[i] += 2; // Move towards target position
        if (yPos[i] > targetY) yPos[i] = targetY; // Prevent overshoot
      } else if (yPos[i] > targetY) {
        yPos[i] -= 2; // Move towards target position
        if (yPos[i] < targetY) yPos[i] = targetY; // Prevent overshoot
      } else {
        // Letter has reached its target position
        currentLetter++; // Move to the next letter
      }
      display.setCursor(currentX, yPos[i]);
      display.print(currentChar);
    }

    // Move the cursor for the next letter based on the current letter's width
    currentX += w + 2; // Add a small padding of 2 pixels
  }

  display.display(); // Update the display

  // Stop updating after all letters are in place
  if (currentLetter >= 7) {
    while (true); // Halt the loop
  }

  delay(9); // Control the animation speed
}
