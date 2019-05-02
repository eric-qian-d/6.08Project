/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <vector>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <string.h>





TFT_eSPI tft = TFT_eSPI();

class Button {
  public:
    uint32_t t_of_state_2;
    uint32_t t_of_button_change;
    uint32_t debounce_time;
    uint32_t long_press_time;
    uint8_t pin;
    uint8_t flag;
    bool button_pressed;
    uint8_t state; // This is public for the sake of convenience
    Button(int p) {
      flag = 0;
      state = 0;
      pin = p;
      t_of_state_2 = millis(); //init
      t_of_button_change = millis(); //init
      debounce_time = 10;
      long_press_time = 1000;
      button_pressed = 0;
    }
    void read() {
      uint8_t button_state = digitalRead(pin);
      button_pressed = !button_state;
    }
    int update1() {
      read();
      flag = 0;
      if (state == 0) { // Unpressed, rest state
        if (button_pressed) {
          state = 1;
          t_of_button_change = millis();
        }
      } else if (state == 1) { //Tentative pressed
        if (!button_pressed) {
          state = 0;
          t_of_button_change = millis();
        } else if (millis() - t_of_button_change >= debounce_time) {
          state = 2;
          t_of_state_2 = millis();
        }
      } else if (state == 2) { // Short press
        if (!button_pressed) {
          state = 4;
          t_of_button_change = millis();
        } else if (millis() - t_of_state_2 >= long_press_time) {
          state = 3;
        }
      } else if (state == 3) { //Long press
        if (!button_pressed) {
          state = 4;
          t_of_button_change = millis();
        }
      } else if (state == 4) { //Tentative unpressed
        if (button_pressed && millis() - t_of_state_2 < long_press_time) {
          state = 2; // Unpress was temporary, return to short press
          t_of_button_change = millis();
        } else if (button_pressed && millis() - t_of_state_2 >= long_press_time) {
          state = 3; // Unpress was temporary, return to long press
          t_of_button_change = millis();
        } else if (millis() - t_of_button_change >= debounce_time) { // A full button push is complete
          state = 0;
          if (millis() - t_of_state_2 < long_press_time) { // It is a short press
            flag = 1;
          } else {  // It is a long press
            flag = 2;
          }
        }
      }
      return flag;
    }
};







static BLEUUID TRACK_SERVICE_UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID TRACK_CHARACTERISTIC_UUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

static BLEUUID PAIR_SERVICE_UUID("3fafc201-1fb5-459e-8fcc-c5c9c331914c");
static BLEUUID PAIR_CHARACTERISTIC_UUID("ceb5483e-36e1-4688-b7f5-ea07361b26a3");

int refreshOrSelectPin = 16;
int togglePin = 5;
int buzzerPin = 22;
int lastButtonPress;
int scrollPosition;
int arrayPtr = 0;
int trackPtr = 0;
char manufactureDesc[15];
bool paired = false;
bool connected = false;
bool beep = false;
char uuid[400];

Button refreshOrSelectButton(refreshOrSelectPin);
Button toggleButton(togglePin);
BLEAdvertisedDevice* devices[5]; // can have a list of 4 devices that are advertised at any given time
BLEClient* trackedDevices[5]; // can have a list of 4 devices that are advertised at any given time

BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.println(advertisedDevice.toString().c_str());
      char deviceDesc[100];
      char deviceName[100];
      strcpy(deviceName, advertisedDevice.getName().c_str());
      //TODO: NO LONGER SETTING MAN DATA - NEED TO FIGURE OUT HOW TO IDENTIFY OUR DEVICES
      strcpy(deviceDesc, advertisedDevice.getManufacturerData().c_str());
      Serial.println(deviceName);
      if ((7<= strlen(deviceName)) && (strncmp(manufactureDesc, deviceName, 7) == 0)) {
        Serial.println("GREAAT SUCESS");
//        BLEClient*  pClient  = BLEDevice::createClient();
//        pClient->setClientCallbacks(new MyClientCallback());
//        Serial.println("ready to connect");
//        pClient->connect(&advertisedDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
//        Serial.println(" - Connected to server");
        if (arrayPtr < 4) {
          devices[arrayPtr] = new BLEAdvertisedDevice(advertisedDevice);
          arrayPtr++;
        }
      }
    }
};

void rerender() {
  tft.fillScreen(TFT_BLACK); //fill background
  for(int i = 0; i < arrayPtr; i++) {
    char deviceName[20];
    strcpy(deviceName, devices[i]->getName().c_str());
    tft.drawString(deviceName, 10, 10 * i, 1);
    if (i == scrollPosition) {
      tft.drawString(">", 0, 10*i, 1);
    }
  }

}


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.println("onConnect");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    beep = true;
  }
};

void setup() {
  Serial.begin(115200);

  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set col

  
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  lastButtonPress = millis();
  scrollPosition = 0;
  pinMode(togglePin, INPUT_PULLUP);
  pinMode(refreshOrSelectPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  ledcSetup(0,1E5,12);
  ledcAttachPin(buzzerPin,0);
  strcpy(manufactureDesc, "MYESP32");
}

void loop() {
  int refreshOrSelectRes = refreshOrSelectButton.update1();
  int toggleRes = toggleButton.update1();
//  Serial.print(refreshOrSelectRes);
//  Serial.println(toggleRes);

  if (refreshOrSelectRes == 2) {//refresh
    Serial.println("REFERESHING");
    arrayPtr = 0;
    BLEScanResults foundDevices = pBLEScan->start(5, false);
    delay(5000);//wait for scan to terminate
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    rerender();

  } else if (refreshOrSelectRes == 1) {//select the current
    Serial.println("in the connecting block");
    BLEAdvertisedDevice* myDevice = devices[scrollPosition];

    Serial.println(myDevice->getServiceUUID().toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();

    pClient->setClientCallbacks(new MyClientCallback());
    
    Serial.println("ready to connect");
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    strcpy(uuid, myDevice->getServiceUUID().toString().c_str());
    paired = true;

        // Obtain a reference to the service we are after in the remote BLE server.
        BLERemoteService* pRemoteService = pClient->getService(TRACK_SERVICE_UUID);
        if (pRemoteService == nullptr) {
          Serial.print("Failed to find our service UUID: ");
          pClient->disconnect();
        }
        Serial.println(" - Found our service");
    
    
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(TRACK_CHARACTERISTIC_UUID);
        if (pRemoteCharacteristic == nullptr) {
          Serial.print("Failed to find our characteristic UUID: ");
          pClient->disconnect();
        }
        Serial.println(" - Found our characteristic");
    
        // Read the value of the characteristic.
        if(pRemoteCharacteristic->canWrite()) {
          pRemoteCharacteristic->writeValue("true", false);
          Serial.print("Set changed");
        }

  } else if (toggleRes != 0 ) {
    scrollPosition = (scrollPosition + 1) % (arrayPtr);
    rerender();
  }

  if (beep) {
    Serial.println("BEEEEEP");
    digitalWrite(buzzerPin, HIGH);
    ledcWriteTone(0,800);
//    ledcWriteNote(0,NOTE_C,1);
  }


}
