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
 
class Button{
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
  if (state==0) { // Unpressed, rest state
    if (button_pressed) {
      state = 1;
      t_of_button_change = millis();
    }
  } else if (state==1) { //Tentative pressed
    if (!button_pressed) {
      state = 0;
      t_of_button_change = millis();
    } else if (millis()-t_of_button_change >= debounce_time) {
      state = 2;
      t_of_state_2 = millis();
    }
  } else if (state==2) { // Short press
    if (!button_pressed) {
      state = 4;
      t_of_button_change = millis();
    } else if (millis()-t_of_state_2 >= long_press_time) {
      state = 3;
    }
  } else if (state==3) { //Long press
    if (!button_pressed) {
      state = 4;
      t_of_button_change = millis();
    }
  } else if (state==4) { //Tentative unpressed
    if (button_pressed && millis()-t_of_state_2 < long_press_time) {
      state = 2; // Unpress was temporary, return to short press
      t_of_button_change = millis();
    } else if (button_pressed && millis()-t_of_state_2 >= long_press_time) {
      state = 3; // Unpress was temporary, return to long press
      t_of_button_change = millis();
    } else if (millis()-t_of_button_change >= debounce_time) { // A full button push is complete
      state = 0;
      if (millis()-t_of_state_2 < long_press_time) { // It is a short press
        flag = 1;
      } else {  // It is a long press
        flag = 2;
      }
    }
  }
  return flag;
}
};
 
 









int refreshOrSelectPin = 16;
int togglePin = 5;
int lastButtonPress;
int scrollPosition;
int arrayPtr = 0;
char manufactureDesc[6];

Button refreshOrSelectButton(refreshOrSelectPin);
Button toggleButton(togglePin);
BLEAdvertisedDevice devices[3];

BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      char deviceDesc[100];
      strcpy(deviceDesc, advertisedDevice.getManufacturerData().c_str());
      if (strcmp(deviceDesc, manufactureDesc)) {
        if (arrayPtr == 0) {
          devices[0] = advertisedDevice;
          arrayPtr++;
        } else {
          devices [1] = advertisedDevice;
        }
      }
    }
};

void rerender() {
  
}

void setup() {
  Serial.begin(115200);
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
  strcpy(manufactureDesc, "608aa");
}

void loop() {
  int refreshOrSelectRes = refreshOrSelectButton.update1();
  int toggleRes = toggleButton.update1();
  
  if (refreshOrSelectRes == 2) {//refresh
    arrayPtr = 0;
    BLEScanResults foundDevices = pBLEScan->start(5, false);
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    
  } else if (refreshOrSelectRes == 1) {//select the current 
    BLEAdvertisedDevice toConnect = devices[scrollPosition];
  } else if (toggleRes != 0 ) {
    scrollPosition = (scrollPosition + 1) % 2;
    rerender();
  }

  
}
