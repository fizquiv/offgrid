#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA 21
#define SCL 22
#define OLED_RST -1

// Create an OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// Battery ADC pin and constants
#define BATTERY_ADC_PIN 35 // Pin para leer el voltaje de la batería
#define ADC_MAX 4095.0
#define ADC_VOLTAGE 3.3

// Battery voltage range
#define BATTERY_MIN_VOLTAGE 3.0 // Ajusta según las especificaciones de tu batería
#define BATTERY_MAX_VOLTAGE 4.2



void setup() {
  
  Serial.begin(115200);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(SDA, SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.display();

  // Configure ADC pin
  pinMode(BATTERY_ADC_PIN, INPUT);
}

void loop() {
  display.clearDisplay();

  // Leer el valor del ADC
  int adcValue = analogRead(BATTERY_ADC_PIN);

  // Calcular el voltaje de la batería
  float voltage = (adcValue / ADC_MAX) * ADC_VOLTAGE * 2;

  // Mapear el voltaje a un porcentaje
  float batteryPercentage = map(voltage * 100, BATTERY_MIN_VOLTAGE * 100, BATTERY_MAX_VOLTAGE * 100, 0, 100);
  batteryPercentage = constrain(batteryPercentage, 0, 100);


  // Mostrar información en el monitor serial
  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V | Battery: ");
  Serial.print(batteryPercentage);

  // Mostrar en pantalla OLED
  display.setCursor(0, 0);
  display.print("Battery Voltage:");
  display.setCursor(0, 10);
  display.print(voltage, 2);
  display.print(" V");

  display.setCursor(0, 30);
  display.print("Charge:");
  display.setCursor(0, 40);
  display.print(batteryPercentage);
  display.print("%");

  display.setCursor(45, 30);
  display.print("ADC value:");
  display.setCursor(45, 40);
  display.print(adcValue);
  display.display();

  // Esperar antes de leer nuevamente
  delay(1000);
}
