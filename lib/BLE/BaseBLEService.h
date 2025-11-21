#ifndef BASEBLESERVICE_H
#define BASEBLESERVICE_H

#include <List>
#include <ArduinoBLE.h>


// callback signature
typedef void (*BLECallback_ptr)(const char * pCharacteristic);

char pWorkingBuffer[64];

class BaseBLEService {

protected:
  BLEService *pMainService;
  bool buttonChanged = false;
  int dutyCycle = 100;

  const BLECharacteristic * GetCharacteristicByID(const char * _uuid)
  {
    const BLECharacteristic * pTmpChar = NULL;
    for (const auto& obj : serviceCharacteristics) {
      if(strcmp(obj.uuid(), _uuid) == 0)
      {
        return &obj;
      }
    }
    return NULL;
  }

  BLEStringCharacteristic *pStrCharacteristic = __null;
  BLEByteCharacteristic *pByteCharacteristic = __null;

public:

  /// @brief Starts BLE service for this device.
  /// @param _deviceName Device name as advertised through Bluetooth LE 
  /// @param _eventHandler Connected event handler
  virtual void Start(String _deviceName, const char * serviceUUIDStr, BLEDeviceEventHandler _eventHandler = NULL)
  {
  
    if(_eventHandler != NULL)
    {
      BLE.setEventHandler(BLEDeviceEvent::BLEConnected, _eventHandler);
    }
 
    BLEService mainSvc(serviceUUIDStr); // create main service
    pMainService = &mainSvc;

    CreateMainCharacteristics(pMainService);
 
    // begin initialization
    if (!BLE.begin()) {
      while (1);
    }

    BLE.addService(*pMainService);
    BLE.setAdvertisedService(*pMainService);
    BLE.setAdvertisedServiceUuid(serviceUUIDStr);
    BLE.setDeviceName(_deviceName.c_str());
    BLE.setLocalName(_deviceName.c_str());
    BLE.advertise();

  }

  virtual void PollBLE()
  {

    // poll for Bluetooth® Low Energy events
    BLE.poll();
    
  }

protected:

  std::list<BLECharacteristic> serviceCharacteristics;

  /// @brief Adds all characteristics in serviceCharacteristics list to the BLE service
  /// @param _bleService 
  virtual void CreateMainCharacteristics(BLEService * _bleService) 
  {

    // serviceCharacteristics list is populated in derived override
    // add all the characteristics in the list to the service
    for (const auto& obj : serviceCharacteristics) {
      _bleService->addCharacteristic(((BLECharacteristic&)obj));
    }

  }

};

#endif
