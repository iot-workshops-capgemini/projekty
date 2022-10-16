/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "SerialCommandHandler.h"

// BLE stack definitions
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "0003CBBB-0000-1000-8000-00805F9B0131" // LED service UUID
#define CHARACTERISTIC_UUID_RX "0003CBB1-0000-1000-8000-00805F9B0131"
#define CHARACTERISTIC_UUID_TX "0003CBB2-0000-1000-8000-00805F9B0131"
#define LED_RED 23
#define LED_GREEN 22
#define LED_BLUE 21


// FreeRTOS task definitions
static QueueHandle_t xMessageQueue;
static unsigned long messageCounter = 0;

class MySerialCommandHandler: public SerialCommandHandler {
  public:
  MySerialCommandHandler(QueueHandle_t xMessageQueue): SerialCommandHandler(xMessageQueue) {}
  
  void rgbLedOn(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {

    int alphaPerc = map(a, 0, 0xff, 0, 100);
    uint32_t red = r*alphaPerc/100;
    uint32_t green = g*alphaPerc/100;
    uint32_t blue = b*alphaPerc/100;
    analogWrite(LED_RED, red);
    analogWrite(LED_GREEN, green);
    analogWrite(LED_BLUE, blue);
  }
};

MySerialCommandHandler *handler;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {

      serial_message_t message;
      message.messageID = ++messageCounter;
      memcpy(message.data, pCharacteristic->getValue().c_str(), sizeof(message.data));
      xQueueSend(xMessageQueue, (void *)&message, portMAX_DELAY);
    }
};

void taskInit() {
  // Create message queue for 10 messages
  xMessageQueue = xQueueCreate(10, sizeof(serial_message_t)); 
  // Start task to handle BLE commands  
  xTaskCreatePinnedToCore(
    [handler](void *data) { handler->handleCommand(data); },
    "CommandHandlder",
    4096,
    NULL,
    1,
    NULL,
    1);
  handler = new MySerialCommandHandler(xMessageQueue);
}

void setup() {
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
  Serial.begin(115200);
  taskInit();

  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_UUID_TX,
										BLECharacteristic::PROPERTY_NOTIFY
									);
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
											 CHARACTERISTIC_UUID_RX,
											BLECharacteristic::PROPERTY_WRITE
										);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
		// do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    // let the other task to process
    vTaskDelay(10);
}
