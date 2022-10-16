#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#define RGBA_MESSAGE_SIZE   4

typedef struct serial_message
{
    uint32_t messageID;
    uint8_t data[RGBA_MESSAGE_SIZE];
} serial_message_t;

class SerialCommandHandler {
  private:
  QueueHandle_t xMessageQueue;

  public:
  SerialCommandHandler(QueueHandle_t xMessageQueue): xMessageQueue(xMessageQueue) {}
  void handleCommand(void *pvParameters);

  private:
  virtual void rgbLedOn(uint8_t r, uint8_t g, uint8_t , uint8_t a);
};

#endif