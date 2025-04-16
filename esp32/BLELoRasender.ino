#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <LoRa.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// **Declaraciones de funciones**
void sendLoRaMessage(String message);
void updateDisplay();

// **OLED Configuración**
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// **LoRa Configuración**
#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DI0 26
#define LORA_BAND 915E6  

// **BLE Configuración**
#define SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

bool deviceConnected = false;
String lastSentMessage = "N/A";    
String lastReceivedMessage = "N/A";  
BLECharacteristic *pTxCharacteristic;

// **Clase para manejar conexión BLE**
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
    }
};

// **Clase para manejar recepción de datos BLE**
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        String value = String(pCharacteristic->getValue().c_str());
        if (value.length() > 0) {
            lastSentMessage = value;
            Serial.println("[BLE] Recibido: " + lastSentMessage);
            
            sendLoRaMessage(lastSentMessage);
            updateDisplay();  
        }
    }
};

void setup() {
    Serial.begin(115200);

    // **Inicializar OLED**
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Error al iniciar OLED");
        while (1);
    }
    updateDisplay();

    // **Inicializar LoRa**
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
    if (!LoRa.begin(LORA_BAND)) {
        Serial.println("Error al iniciar LoRa");
        while (1);
    }
    Serial.println("LoRa listo!");

    // **Inicializar BLE**
    BLEDevice::init("ESP32_LoRa_BLE");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxCharacteristic->setCallbacks(new MyCallbacks());

    pTxCharacteristic = pService->createCharacteristic(
        BLEUUID(CHARACTERISTIC_UUID),
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();
    Serial.println("BLE listo!");
}

void loop() {
    // **Recibir mensaje LoRa**
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        lastReceivedMessage = "";
        while (LoRa.available()) {
            lastReceivedMessage += (char)LoRa.read();
        }
        Serial.println("[LoRa] Recibido: " + lastReceivedMessage);

        // Actualizar OLED
        updateDisplay();

        // Enviar por BLE si está conectado
        if (deviceConnected) {
            pTxCharacteristic->setValue(lastReceivedMessage.c_str());
            pTxCharacteristic->notify();
            Serial.println("[BLE] Enviado LoRa → BLE");
        }
    } else {
        Serial.println("[LoRa] Esperando mensaje...");
    }

    delay(1000);
}

void sendLoRaMessage(String message) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    Serial.println("[LoRa] Enviado: " + message);
}

void updateDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.println("Sent:");
    display.println(lastSentMessage);

    display.println("\nReceived:");
    display.println(lastReceivedMessage);

    display.println("\nWaiting for messages...");

    display.display();
}
