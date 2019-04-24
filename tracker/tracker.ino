#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; //button 2
uint32_t primary_timer;

uint8_t state;
uint8_t screen_color;

void setup() {
  Serial.begin(115200);               // Set up serial port
  pinMode(PIN_1, INPUT_PULLUP);
  primary_timer = micros();
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set col

}

void loop() {
  
  while (micros() > primary_timer && micros() - primary_timer < sample_period); //prevents rollover glitch every 71 minutes...not super needed
  primary_timer = micros();

}
