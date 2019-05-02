/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/
#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

int man_code = 0x02E5;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

static BLEUUID TRACK_SERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID TRACK_CHARACTERISTIC_UUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static BLEUUID PAIR_SERVICE_UUID("3fafc201-1fb5-459e-8fcc-c5c9c331914c");
static BLEUUID PAIR_CHARACTERISTIC_UUID("ceb5483e-36e1-4688-b7f5-ea07361b26a3");

BLEAdvertising *pAdvertising;
BLEAdvertisementData advert;

int ledPin = 19;
int buzzerPin = 
boolean tracked = false;
boolean paired = false;
boolean connected = false;


class WriteCB: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {

      if ((pCharacteristic->getUUID()).equals(TRACK_CHARACTERISTIC_UUID)) {
        std::string value = pCharacteristic->getValue();
        if (value == "false") {
          tracked = false;
        }
        if (value == "true") {
          tracked = true;
        }
        
        if (value.length() > 0) {
          Serial.println("*********");
          Serial.print("New value: ");
          for (int i = 0; i < value.length(); i++)
            Serial.print(value[i]);
  
          Serial.println();
          Serial.println("*********");
        }
      }

      if ((pCharacteristic->getUUID()).equals(PAIR_CHARACTERISTIC_UUID)) {
        std::string value = pCharacteristic->getValue();
        if (value == "false") {
          paired = false;
        }
        if (value == "true") {
          paired = true;
        }
      } 
    }
};

class ConnectCB: public BLEServerCallbacks{
  void onConnect(BLEServer* pServer) {
    connected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    connected = false;
  }
};

void setManData(String c, int c_size, BLEAdvertisementData &adv, int m_code) {
  
  String s;
  char b2 = (char)(m_code >> 8);
  m_code <<= 8;
  char b1 = (char)(m_code >> 8);
  s.concat(b1);
  s.concat(b2);
  s.concat(c);

  adv.setManufacturerData(s.c_str());
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("MYESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(TRACK_SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         TRACK_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pCharacteristic->setCallbacks(new WriteCB());

  pServer->setCallbacks(new ConnectCB());
  
  pService->start();
  
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(TRACK_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
//  
//  advert.setName("608aa");
//  advert.setCompleteServices(SERVICE_UUID);
//  pAdvertising->setAdvertisementData(advert);
//  BLEAdvertisementData scan_response;
//  setManData("608aa", strlen("608aa"), scan_response, man_code);
//  pAdvertising->setScanResponseData(scan_response);

  pAdvertising->start();
  
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  pinMode(ledPin, OUTPUT); //LED setup
  ledcSetup(0,1E5,12); //buzzer setup
  ledcAttachPin(22,0);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(paired) {
    Serial.println("being paired");
    digitalWrite(ledPin, HIGH);
    ledcWriteTone(0,800);
  } else if(tracked && !connected) {
    Serial.println("we have an issue");
    //code for tracking 
    digitalWrite(ledPin, HIGH);
    ledcWriteTone(0,800);
  } else {
    digitalWrite(ledPin, LOW);
    ledcWriteNote(0,NOTE_C,1);  
  }
  Serial.print(paired);
  Serial.print(tracked);
  Serial.println(connected);
  delay(1000);
}
