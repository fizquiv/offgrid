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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;

void setup() {
  Serial.begin(115200);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(SDA, SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();

  Serial.println("LoRa Receiver Test");

  //SPI LoRa pins
  SPI.begin(SCLK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(CS, RST, DIO0);

  if (!LoRa.begin(BAND)){
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();  
}

void loop(){
  int packetSize = LoRa.parsePacket();
  if(packetSize){

    Serial.print("Recieved packet ");

    //read packet
    while (LoRa.available()){
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }

    int rssi = LoRa.packetRssi();
    Serial.print( "with RSSI ");
    Serial.println(rssi);
    
   display.clearDisplay();
   display.setCursor(0,0);
   display.print("LORA RECEIVER");
   display.setCursor(0,20);
   display.print("Received packet:");
   display.setCursor(0,30);
   display.print(LoRaData);
   display.setCursor(0,40);
   display.print("RSSI:");
   display.setCursor(30,40);
   display.print(rssi);
   display.display();
  }
}