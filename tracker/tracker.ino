#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define IDLE 0
#define REGISTER 1

#define PRESSED 0
#define UNPRESSED 1

const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; //button 2
const char
uint32_t primary_timer;

uint8_t state;
uint8_t screen_color;

boolean in_welcome;
char select_char[] = "-"; // selection indicator variable

void setup() {
  Serial.begin(115200);               // Set up serial port
  pinMode(PIN_1, INPUT_PULLUP);
  primary_timer = micros();
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set col
  in_welcome = false;
}

void welcome() {
  in_welcome = true;
  tft.fillScreen(TFT_BLACK); //fill background
  tft.drawString("Welcome!", 0, 1, 1);
  tft.drawString("Register new item", 10, 10, 1);
  tft.drawString("Track items", 10, 20, 1);
  tft.drawString("View registered items", 10, 30, 1);
}

void loop() {
  switch (state) {
    case IDLE:
      if (!in_welcome) {
        // welcome the user
        welcome();
      }
      toggle = digitalRead(PIN_1);
      if (toggle == PRESSED) {
        tft.drawString(select_char, 10, 1);
      }
      break;
    case REGISTER:
      break;
  }
  
  primary_timer = micros();

}
