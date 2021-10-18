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


// BLE stack definitions
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


// FreeRTOS task definitions
static QueueHandle_t xMessageQueue;
static unsigned long messageCounter = 0;

typedef struct serial_message
{
    uint32_t messageID;
    char data[20];
} serial_message_t;

typedef enum state {
  STATE_COMMAND,
  STATE_ECHO
} state_t;

static state_t state = STATE_COMMAND;


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
      strncpy(message.data, pCharacteristic->getValue().c_str(), sizeof(message.data));
      xQueueSend(xMessageQueue, (void *)&message, portMAX_DELAY);
    }
};

class SerialCommandHandler {

  public:
  static void handleCommand(void *pvParameters) {
    serial_message_t message;
    
    while (1) {
      xQueueReceive(xMessageQueue, (void *)&message, portMAX_DELAY);
  
      std::string rxValue(message.data);
      
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);
  
        Serial.println();
        Serial.println("*********");
      }


      std::string command = trim(rxValue);
      switch (state) {
        default:
        case STATE_COMMAND: {
            if (command == "ledon") {
              ledon();
            }
            if (command == "ledoff") {
              ledoff();
            }
            if (command == "echo") {
              state = STATE_ECHO;
            }
          }
          break;
        case STATE_ECHO: {
          if (command == "quit" || command == "exit") {
            state = STATE_COMMAND;
          }
          break;
        }
      }
    }
  }

  private:
  static std::string trim(std::string &value) {
    std::size_t wc = std::string::npos;
    wc = value.find_last_not_of("\r\n");
    if (wc != std::string::npos) {
      return value.substr(0, wc + 1);
    }
    return value;
  }

  static void ledon() {
    // TODO
  }

  static void ledoff() {
    // TODO
  }
};


void taskInit() {
  // Create message queue for 10 messages
  xMessageQueue = xQueueCreate(10, sizeof(serial_message_t)); 
  // Start task to handle BLE commands
  xTaskCreatePinnedToCore(
    &SerialCommandHandler::handleCommand,
    "CommandHandlder",
    4096,
    NULL,
    1,
    NULL,
    1);
}

void setup() {
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
}
