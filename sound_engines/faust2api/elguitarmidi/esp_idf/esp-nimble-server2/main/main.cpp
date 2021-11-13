/*
* server example in:
* https://h2zero.github.io/esp-nimble-cpp/md_docs__new_user_guide.html
*/

#include "NimBLEDevice.h"
 
extern "C"{void app_main(void);} 
 
 
// void setup() in Arduino
void app_main(void)
{
    NimBLEDevice::init("esp-nimble-server2");
    
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService("ABCD");
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic("1234");
    
    pService->start();
    pCharacteristic->setValue("Hello BLE");
    
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("ABCD"); 
    pAdvertising->start(); 
}