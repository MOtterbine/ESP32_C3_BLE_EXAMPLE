#ifndef EXAMPLEBLESERVICE_H
#define EXAMPLEBLESERVICE_H

#include <BaseBLEService.h>
#include <Temperature_LM75_Derived.h>
#include <mutex>
#include <ArduinoBLE.h>


/*****************MAIN BLE SERVICE *****************/
#define UUID_SERVICE_ID_MAIN "C1B24045-41AF-425A-97CC-758CB1FAEB08"

#define UUID_CHARACTERISTIC_LED_SWITCH "E7D73222-8BDF-49B5-BAFE-287E72D351A8"
#define UUID_CHARACTERISTIC_BUTTON "65928536-7969-4255-B57D-FBD7E36F1363"
#define UUID_CHARACTERISTIC_TEMPERATURE "0883F6A5-20F9-4735-B658-EC39C77E82D8"
#define UUID_CHARACTERISTIC_MESSAGE "68F5676B-6203-4A03-AC76-733D251062E2"
#define UUID_CHARACTERISTIC_PWM "2E46BE34-77C5-42D5-B40F-AE9F487DD1E6"

 
#define PWMFreq 5000 /* 5 KHz */
#define PWMChannel 0
#define PWMResolution 10
#define MAX_DUTY_CYCLE (int)(pow(2, PWMResolution) - 1)

#define ledPin 10 // = 10 set ledPin to on-board LED
#define buttonPin 2 // set buttonPin to digital pin 4


/// @brief ensures messages don't gets stepped on
std::mutex msgMutex;

enum UserCommands
{
  PWM_OFF = 0x00,
  PWM_ON = 0x01
};

class ExampleBLEService : protected BaseBLEService{

  private:
    Generic_LM75 temperature;
    byte PWMValue = 100;
  public: 
  /// @brief Starts BLE service for this device.
  /// @param _deviceName Device name as advertised through Bluetooth LE 
  /// @param serviceUUIDStr Main service uuid will be advertised 
  /// @param _eventHandler Connected event handler
  void Start(String _deviceName, const char * serviceUUIDStr, BLEDeviceEventHandler _eventHandler = NULL) override
  {
  
    BaseBLEService::Start(_deviceName, serviceUUIDStr, _eventHandler);

    SetMessage("");
    SetLED(0);
    ledcWrite(PWMChannel, 0);
    SetPWM(0);
    SetTemperature(0);

  }

    /***************************** Device Message ******************************/
  const String GetMessage()
  {
    return ((BLEStringCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_MESSAGE))->value();
  }
  void SetMessage(const char * _val)
  {
    std::lock_guard<std::mutex> guard(msgMutex);
    pStrCharacteristic = ((BLEStringCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_MESSAGE));
    pStrCharacteristic->writeValue(_val);
  }

  /***************************** Temperature ******************************/
  double GetTemperature()
  {
    return ((BLEDoubleCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_TEMPERATURE))->value();
  }
  void SetTemperature(double _val)
  {
    ((BLEDoubleCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_TEMPERATURE))->writeValue(_val);
  }

  /******************************** PWM *********************************/
  byte GetPWM()
  {

    pByteCharacteristic = ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_PWM));
    int val = (100*(pByteCharacteristic->value())/MAX_DUTY_CYCLE);
 
    return val;
  }
  /// @brief Set PWM
  /// @param _val Percent duty cycle
  void SetPWM(byte _val)
  {

    pByteCharacteristic = ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_PWM));
    if(_val > 100) _val = 100;
    int val = (_val*MAX_DUTY_CYCLE)/100;
    ledcWrite(PWMChannel, val);
    pByteCharacteristic->writeValue(_val);
  }

  /******************************** LED (binary) *********************************/
  byte GetLED()
  {
    return ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_LED_SWITCH))->value();
  }
  void SetLED(byte _val)
  {
    pByteCharacteristic = ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_LED_SWITCH));
    pByteCharacteristic->writeValue(_val);
  }

  /******************************** Button *********************************/
  byte GetButton()
  {
    return ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_BUTTON))->value();
  }

  void SetButton(byte _val)
  {
    ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_BUTTON))->writeValue(_val);
  }

  /// @brief Adds all characteristics in serviceCharacteristics list to the BLE service
  /// @param _bleService 
  void CreateMainCharacteristics(BLEService * _bleService) override
  {

    serviceCharacteristics.clear();

    // Create a list of characteristicts
    BLEByteCharacteristic ledCharacteristic(UUID_CHARACTERISTIC_LED_SWITCH, BLERead | BLEWrite| BLENotify);
    serviceCharacteristics.insert(serviceCharacteristics.begin(),  ledCharacteristic);
    
    BLEByteCharacteristic ledPWMCharacteristic(UUID_CHARACTERISTIC_PWM, BLERead | BLEWrite | BLENotify);
    serviceCharacteristics.insert(serviceCharacteristics.end(),  ledPWMCharacteristic);

    BLEByteCharacteristic buttonCharacteristic(UUID_CHARACTERISTIC_BUTTON, BLERead | BLENotify);
    serviceCharacteristics.insert(serviceCharacteristics.end(),  buttonCharacteristic);

    BLEDoubleCharacteristic tempCharacteristic(UUID_CHARACTERISTIC_TEMPERATURE, BLERead | BLENotify);
    serviceCharacteristics.insert(serviceCharacteristics.end(),  tempCharacteristic);

    BLEStringCharacteristic messageCharacteristic(UUID_CHARACTERISTIC_MESSAGE, BLERead | BLENotify, 64);
    serviceCharacteristics.insert(serviceCharacteristics.end(),  messageCharacteristic);

    BaseBLEService::CreateMainCharacteristics(_bleService);

  }
  

  void PollData()
  {
    try
    {
      // double temp = temperature.readTemperatureC();
      // SetTemperature((temp*1.8)+32);
      SetTemperature(temperature.readTemperatureF());
    }
    catch(const std::exception& e)
    {
      //std::cerr << e.what() << '\n';
    }

  }

  void PollBLE() override
  {

    BaseBLEService::PollBLE();
    
    // read the current button pin state
    char buttonValue = digitalRead(buttonPin);

    pByteCharacteristic = ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_BUTTON));
    // has the value changed since the last read
    buttonChanged = pByteCharacteristic->value() != buttonValue;

    // ****************** Button **********************
    if (buttonChanged) {
      pByteCharacteristic->writeValue(buttonValue);
      // only toggle the LED value on press
      if(buttonValue == 1)
      {
        if(GetLED() == 0) SetLED(0x01);
        else SetLED(0x00);
      }
      //  sprintf(pWorkingBuffer, "Button %s", buttonValue==0?"Release":"Press");
   //   SetMessage(pWorkingBuffer);
    }

    pByteCharacteristic = ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_PWM));
    if (pByteCharacteristic->written()) {

      if(pByteCharacteristic->value() > 100) pByteCharacteristic->writeValue(100);
      PWMValue = pByteCharacteristic->value();
      int val = (PWMValue*MAX_DUTY_CYCLE)/100;
      ledcWrite(PWMChannel, val);

      sprintf(pWorkingBuffer, "PWM Set: %d%%", pByteCharacteristic->value());
      SetMessage(pWorkingBuffer);
      if(PWMValue == 0)
      {
          SetLED(0x00);
      } else SetLED(0x01);

    }

    pByteCharacteristic = ((BLEByteCharacteristic*)GetCharacteristicByID(UUID_CHARACTERISTIC_LED_SWITCH));
    if (pByteCharacteristic->written() || buttonChanged) {

      switch(pByteCharacteristic->value())
      {
        case UserCommands::PWM_OFF:
          //ledcWrite(PWMChannel, 0);
          SetPWM(0);
          SetMessage("PWM Off: 0%");
          break;
        case UserCommands::PWM_ON:
          //ledcWrite(PWMChannel, MAX_DUTY_CYCLE);
          SetPWM(PWMValue);
          sprintf(pWorkingBuffer, "PWM On: %d%%", PWMValue);
          SetMessage(pWorkingBuffer);
          break;
      }
    }
  }

} BLEMainService;

void BLEEventHandler(BLEDevice dev)
{ 
  
  if(strlen(dev.deviceName().c_str()) > 0)
  {
    sprintf(pWorkingBuffer, "%S connected", dev.deviceName().c_str());
    BLEMainService.SetMessage(pWorkingBuffer); 
    return;
  }
  BLEMainService.SetMessage("Client Connected"); 
};

#endif
