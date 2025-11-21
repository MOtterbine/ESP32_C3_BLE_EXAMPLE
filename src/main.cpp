
#include "command.h"
#include <EEPROM.h>

  // For EEPROM - Data begins at 0x200 
  #define BASE_ADDRESS_STORAGE_LOCATION 0x200
  #define EEPROM_INIT_STORAGE_CAPACITY 0x800
  #define EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_1 0
  #define EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_2 sizeof(double)+EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_1
  #define EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_3 sizeof(double)+EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_2
  #define EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_4 sizeof(double)+EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_3


  void setup() {

    Serial.begin(115200,SERIAL_8N1,RX_GPIO_NUM,TX_GPIO_NUM);
    while (!Serial);

    pDebugChannel = &Serial;
    pDebugChannel->printf("%s Starting...", BLUETOOTH_VISIBLE_NAME);

    // PWM - pulse width modulation
    ledcSetup(PWMChannel, PWMFreq, PWMResolution);
    /* Attach the LED PWM Channel to the GPIO Pin */
    ledcAttachPin(ledPin, PWMChannel);   
    pinMode(buttonPin, INPUT); // use button pin as an input
  
    // Initialize EEPROM, Total memory to use
    EEPROM.begin(EEPROM_INIT_STORAGE_CAPACITY); 
    // Avoid writing eeprom regularly
    // EEPROM Read/Write...
    // double val = PI;
    // EEPROM.writeDouble(BASE_ADDRESS_STORAGE_LOCATION + EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_1, val);
    // // EEPROM Read
    // val = 0; 
    // val = EEPROM.readDouble(BASE_ADDRESS_STORAGE_LOCATION + EEPROM_ADDR_CUSTOM_DOUBLE_VALUE_1);

    // for temp sensor (I2C)
    Wire.begin();
    
    // Setup and run Bluetooth Low Energy (BLE)
    BLEMainService.Start(BLUETOOTH_VISIBLE_NAME, UUID_SERVICE_ID_MAIN, BLEEventHandler);

    // uncomment to poll temperature
    xTaskCreate(PollData, "Poll Temperature", 2000, NULL, 1, NULL);
    xTaskCreate(PollBLE, "Poll BLE Events", 4000, NULL, 1, NULL);

  }

  void loop() {
    
    // delete this main loop
    vTaskDelete(NULL);
    //vTaskDelay(100/portTICK_PERIOD_MS);

  }
