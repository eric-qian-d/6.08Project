#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define IDLE 0
#define REGISTER 1
#define RECORD_NAME 2

#define PRESSED 0
#define UNPRESSED 1

#define SUCCESS 0
#define FAIL 1

char network[] = "MIT GUEST";
char password[] = "";

const char USER[] = "jenning";

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host

const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; //button 2

uint32_t primary_timer;
uint32_t timeout_timer;

const uint8_t TIMEOUT_PERIOD = 2500; //milliseconds

uint8_t toggle;
uint8_t pair_status;
uint8_t state;
uint8_t screen_color;

boolean in_welcome;
char select_char[] = "-"; // selection indicator variable

void setup() {
  Serial.begin(115200);               // Set up serial port
  pinMode(PIN_1, INPUT_PULLUP);
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set col
  WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  state = IDLE;
  primary_timer = millis();
  timeout_timer = millis();
  in_welcome = false;
}

void welcome() {
  in_welcome = true;
  tft.fillScreen(TFT_BLACK); //fill background
  tft.drawString("Welcome!", 0, 1, 1);
  tft.drawString("Register new item", 10, 10, 1);
  tft.drawString("Track items", 10, 20, 1);
  tft.drawString("View registered", 10, 30, 1);
  tft.drawString("items", 10, 40, 1);
  tft.drawString(select_char, 2, 10, 1);
}

void register_prompt() {
  tft.fillScreen(TFT_BLACK); //fill background
  tft.drawString("Registering: place", 0, 50, 1);
  tft.drawString("your item next to me!", 0, 60, 1);

}

void loop() {
  switch (state) {
    case IDLE:
      if (!in_welcome) {
        welcome(); // welcome the user
      }
      toggle = digitalRead(PIN_1);
      if (toggle == PRESSED) {
        in_welcome = false;
        register_prompt();
        state = REGISTER;
      }
      break;
    case REGISTER:
      if (toggle == PRESSED) { // press button, cancel and go back to IDLE
        state = IDLE;
      } else if (pair_status == SUCCESS) { // this needs to be defined
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Success!", 0, 50, 1);
        while (millis() - timeout_timer < TIMEOUT_PERIOD);
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Speak item's name", 0, 50, 1);
        state = RECORD_NAME;
      } else if (pair_status == FAIL) { // this needs to be defined
        timeout_timer = millis();
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Failed. Module not found. :(", 0, 50, 1);
        while (millis() - timeout_timer < TIMEOUT_PERIOD);
        state = IDLE; // go back to IDLE
      }
      break;
    case RECORD_NAME:
      // detect name and print it to ask user if it's correct
      // if correct, add the name and appropriate info to database
      // go back to IDLE
      break;

  }
  

}
