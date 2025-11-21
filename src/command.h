#ifndef COMMANDS_H
#define COMMANDS_H

#include <map>
#include <ExampleBLEService.h>

#define FIRMWARE_VERSION  "1.0.0"
#define BLUETOOTH_VISIBLE_NAME "BLE Example"

#define TX_GPIO_NUM 21
#define RX_GPIO_NUM 20


  uint tempVal = 0;
  String _message;

  String inputBuffer;

  /// @brief Serial channel used to send debug information
  Stream * pDebugChannel = __null;
  char EOT = '>';
  // pointer to a void function
  typedef void (*void_ptr)(void);


  void PollData(void * params)
  {
    for(;;) // never to leave
    {
      BLEMainService.PollData();
      vTaskDelay(300/portTICK_PERIOD_MS); 
    }
    vTaskDelete(NULL);
  }


  void PollBLE(void * params)
  {
    for(;;)
    {
      BLEMainService.PollBLE();
      vTaskDelay(80/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
  }



#endif