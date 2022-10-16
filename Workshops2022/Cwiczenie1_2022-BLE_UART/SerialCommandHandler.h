#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

#define LCD_COLUMNS_IN_ROW 16
#define LCD_MAX_CAPACITY   32

typedef struct serial_message
{
    uint32_t messageID;
    char data[LCD_MAX_CAPACITY];
} serial_message_t;

typedef enum state {
  STATE_COMMAND,
  STATE_ECHO
} state_t;

static state_t state = STATE_COMMAND;

class SerialCommandHandler {
  private:
  QueueHandle_t xMessageQueue;
  state_t state = STATE_COMMAND;

  public:
  SerialCommandHandler(QueueHandle_t xMessageQueue): xMessageQueue(xMessageQueue) {}
  void handleCommand(void *pvParameters);

  static std::string trim(std::string &value) {
    std::size_t wc = std::string::npos;
    wc = value.find_last_not_of("\r\n");
    if (wc != std::string::npos) {
      return value.substr(0, wc + 1);
    }
    return value;
  }

  private:
  virtual void ledon();
  virtual void ledoff();
  virtual void echo(std::string &txValue);
};

#endif