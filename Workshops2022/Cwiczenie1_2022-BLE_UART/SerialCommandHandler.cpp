#include "SerialCommandHandler.h"

void SerialCommandHandler::handleCommand(void *pvParameters) {
  serial_message_t message;

  while (1) {
      xQueueReceive(xMessageQueue, (void *)&message, portMAX_DELAY);

      size_t size = sizeof(message.data);
      std::string rxValue((const char *)message.data);
      
      Serial.println("*********");
      Serial.println("Received Value:");

      if (size > 0) {           
      Serial.print("Text: ");
      Serial.println(rxValue.c_str());

      Serial.print("HEX: ");
      rxValue.clear();
      for (int i = 0; i < size; i++) {
        char toHexBuff[3];
        sprintf(toHexBuff, "%02X", message.data[i]);
        rxValue.append(toHexBuff);
      }
      Serial.println(rxValue.c_str());        
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
          } else {
            echo(rxValue);
          }
          break;
      }
    }
  }

  vTaskDelete(NULL);
}