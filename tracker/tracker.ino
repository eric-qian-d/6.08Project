#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <string.h>

#include "button.cpp"

#define IDLE 0
#define REGISTER 1
#define RECORD_NAME 2
#define NAME_VERIFY 3

#define SHORTPRESS 1
#define LONGPRESS 2

#define NONE 0
#define SUCCESS 1
#define FAIL 2

TFT_eSPI tft = TFT_eSPI();

char network[] = "MIT GUEST";
char password[] = "";

const char USER[] = "jenning";

//Some constants and some resources:
const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

const uint8_t PIN_1 = 16; //button 1
const uint8_t PIN_2 = 5; //button 2

uint32_t primary_timer;
uint32_t timer;
uint32_t timeout_timer;

const uint8_t TIMEOUT_PERIOD = 2500; //milliseconds

uint8_t toggle;
uint8_t state;
uint8_t screen_color;

/* MIC VARIABLES*/

const int DELAY = 1000;
const int SAMPLE_FREQ = 8000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 3;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip


/* CONSTANTS */
//Prefix to POST request:
const char PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\",\"profanityFilter\":true,\"speechContexts\":[{\"phrases\": [\"draw\", \"rectangle\",\"circle\",  \"draw rectangle\",\"draw circle\"]}]}, \"audio\": {\"content\":\"";
const char SUFFIX[] = "\"}}"; //suffix to POST request
const int AUDIO_IN = A0; //pin where microphone is connected
const char API_KEY[] = "AIzaSyD6ETx_Ammh1jgbfxG_wggm8eGa08yzQzQ"; //don't change this


/* Global variables*/
uint8_t button_state; //used for containing button state and detecting edges
int old_button_state; //used for detecting button edges
uint8_t button_state2; //used for containing button state and detecting edges
int old_button_state2; //used for detecting button edges
uint32_t time_since_sample;      // used for microsecond timing

char name_transcript[100];

char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data
const char*  SERVER = "speech.google.com";  // Server URL

uint8_t old_val;

WiFiClientSecure client; //global WiFiClient Secure object

boolean in_welcome;
char select_char[] = "-"; // selection indicator variable
char uuid[400]; // stores uuid

int refreshOrSelectPin = 16;
int togglePin = 5;
int lastButtonPress;
int scrollPosition;
int arrayPtr = 0;
char manufactureDesc[15];
int BLEconnected = 0;

Button refreshOrSelectButton(refreshOrSelectPin);
Button toggleButton(togglePin);
BLEAdvertisedDevice* devices[3];

BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      char deviceDesc[100];
      char deviceName[100];
      strcpy(deviceName, advertisedDevice.getName().c_str());
      strcpy(deviceDesc, advertisedDevice.getManufacturerData().c_str());
      if (strcmp(deviceName, manufactureDesc) == 0) {
        if (arrayPtr == 0) {
          devices[0] = new BLEAdvertisedDevice(advertisedDevice);
          arrayPtr++;
        } else {
          devices [1] = new BLEAdvertisedDevice(advertisedDevice);
        }
      }
    }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    BLEconnected = 1;
  }

  void onDisconnect(BLEClient* pclient) {
//    connected = false;
    BLEconnected = 2;
  }
};

void setup() {
  Serial.begin(115200);               // Set up serial port
  pinMode(PIN_1, INPUT_PULLUP);
  pinMode(PIN_2, INPUT_PULLUP);
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

  analogSetAttenuation(ADC_6db); //set to 6dB attenuation for 3.3V full scale reading.

  state = IDLE;
  primary_timer = millis();
  timeout_timer = millis();
  timer = millis();
  in_welcome = false;

  //BLE

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  lastButtonPress = millis();
  scrollPosition = 0;
  strcpy(manufactureDesc, "608aa");
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

  // fetch weather data
  char IP_ADDRESS[30];
  sprintf(IP_ADDRESS, "%s", WiFi.localIP().toString());
  //fetch_weather_data(IP_ADDRESS); 
}

void register_prompt() {
  tft.fillScreen(TFT_BLACK); //fill background
  tft.drawString("Registering: place", 0, 50, 1);
  tft.drawString("your item next to me!", 0, 60, 1);

}

void loop() {
  //Serial.println(state);
  switch (state) {
    case IDLE: {
      if (!in_welcome) {
        welcome(); // welcome the user
      }
      toggle = refreshOrSelectButton.update1();
      if (toggle == SHORTPRESS) {
        in_welcome = false;
        register_prompt();
        state = REGISTER;
      }
    }
      break;
    case REGISTER: {
      int refreshOrSelectRes = refreshOrSelectButton.update1();
      int toggleRes = toggleButton.update1();

      if (refreshOrSelectRes == LONGPRESS) {//refresh
        Serial.println("REFERESHING");
        arrayPtr = 0;
        BLEScanResults foundDevices = pBLEScan->start(5, false);
        delay(5000);//wait for scan to terminate
        pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    
      } else if (refreshOrSelectRes == SHORTPRESS) {//select the current
        Serial.println("in the connecting block");
        BLEAdvertisedDevice* myDevice = devices[scrollPosition];
    
        Serial.println(myDevice->getServiceUUID().toString().c_str());
        strcpy(uuid, myDevice->getServiceUUID().toString().c_str());
    
        BLEClient*  pClient  = BLEDevice::createClient();
    
        pClient->setClientCallbacks(new MyClientCallback());
        
        Serial.println("ready to connect");
        pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
        Serial.println(" - Connected to server");
//        pair
    
    
      } else if (toggleRes != 0 ) {
        scrollPosition = (scrollPosition + 1) % 2;
//        rerender();
      }
    
      if (BLEconnected == SUCCESS) { 
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Success!", 0, 50, 1);
        while (millis() - timeout_timer < TIMEOUT_PERIOD);
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Press button to record item's name", 0, 50, 1);
        state = RECORD_NAME;
      } else if (BLEconnected == FAIL) { 
        timeout_timer = millis();
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Failed. Module not found. :(", 0, 50, 1);
        while (millis() - timeout_timer < TIMEOUT_PERIOD);
        state = IDLE; // go back to IDLE
      }
    }
      break;
    case RECORD_NAME: {
      int refreshOrSelectRes = refreshOrSelectButton.update1();
      if (refreshOrSelectRes == SHORTPRESS) {
        handle_record();
        state = NAME_VERIFY;
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Is", 0, 10, 1);
        tft.drawString(name_transcript, 0, 20, 1);
        tft.drawString("correct?", 0, 30, 1);
      }
    }
      break;
    
    case NAME_VERIFY:
      int refreshOrSelectRes = refreshOrSelectButton.update1();
      int toggleRes = toggleButton.update1();

      // reject the name
      if (refreshOrSelectRes == SHORTPRESS) {
        memset(name_transcript, 0, strlen(name_transcript));
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Press button to record item's name", 0, 50, 1);
        state = RECORD_NAME;
      }
      // accept the name
      if (toggleRes == SHORTPRESS) {
        // post request to server
        char body[200]; //for body;
        sprintf(body, "id=%s&name=%s", "23452rasdfasf" , name_transcript); //generate body, posting to User, 1 step
        int body_len = strlen(body); //calculate body length (for header reporting)
        sprintf(request_buffer, "POST http://608dev.net/sandbox/sc/lyy/new_test.py HTTP/1.1\r\n");
        strcat(request_buffer, "Host: 608dev.net\r\n");
        strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
        sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
        strcat(request_buffer, "\r\n"); //new line from header to body
        strcat(request_buffer, body); //body
        strcat(request_buffer, "\r\n"); //header
        Serial.println(request_buffer);
        do_http_request("608dev.net", request_buffer, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        //tft.println(response); //print the result

        tft.fillScreen(TFT_BLACK);
        tft.drawString(name_transcript, 0, 10, 1);
        tft.drawString(" has been registered", 0, 20, 1);
        memset(name_transcript, 0, strlen(name_transcript));
        state = IDLE;
      }
      break;
  }

  while (millis() - timer < 10);
  timer = millis();
}

void handle_record() {
  Serial.println("listening...");
  record_audio();
  Serial.println("sending...");
  Serial.print("\nStarting connection to server...");
  delay(300);
  bool conn = false;
  for (int i = 0; i < 10; i++) {
    int val = (int)client.connect(SERVER, 443);
    Serial.print(i); Serial.print(": "); Serial.println(val);
    if (val != 0) {
      conn = true;
      break;
    }
    Serial.print(".");
    delay(300);
  }
  if (!conn) {
    Serial.println("Connection failed!");
    return;
  } else {
    Serial.println("Connected to server!");
    Serial.println(client.connected());
    int len = strlen(speech_data);
    // Make a HTTP request:
    client.print("POST /v1/speech:recognize?key="); client.print(API_KEY); client.print(" HTTP/1.1\r\n");
    client.print("Host: speech.googleapis.com\r\n");
    client.print("Content-Type: application/json\r\n");
    client.print("cache-control: no-cache\r\n");
    client.print("Content-Length: "); client.print(len);
    client.print("\r\n\r\n");
    int ind = 0;
    int jump_size = 1000;
    char temp_holder[jump_size + 10] = {0};
    Serial.println("sending data");
    while (ind < len) {
      delay(80);//experiment with this number!
      //if (ind + jump_size < len) client.print(speech_data.substring(ind, ind + jump_size));
      strncat(temp_holder, speech_data + ind, jump_size);
      client.print(temp_holder);
      ind += jump_size;
      memset(temp_holder, 0, sizeof(temp_holder));
    }
    client.print("\r\n");
    //Serial.print("\r\n\r\n");
    Serial.println("Through send...");
    unsigned long count = millis();
    while (client.connected()) {
      Serial.println("IN!");
      String line = client.readStringUntil('\n');
      Serial.print(line);
      if (line == "\r") { //got header of response
        Serial.println("headers received");
        break;
      }
      if (millis() - count > RESPONSE_TIMEOUT) break;
    }
    Serial.println("");
    Serial.println("Response...");
    count = millis();
    while (!client.available()) {
      delay(100);
      Serial.print(".");
      if (millis() - count > RESPONSE_TIMEOUT) break;
    }
    Serial.println();
    Serial.println("-----------");
    memset(response, 0, sizeof(response));
    while (client.available()) {
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    Serial.println(response);
    char* trans_id = strstr(response, "transcript");
    if (trans_id != NULL) {
      char* foll_coll = strstr(trans_id, ":");
      char* starto = foll_coll + 2; //starting index
      char* endo = strstr(starto + 1, "\""); //ending index
      int transcript_len = endo - starto + 1;
      char transcript[100] = {0};
      strncat(transcript, starto, transcript_len);
      Serial.println(transcript);
      strcpy(name_transcript, transcript);
    }
    Serial.println("-----------");
    client.stop();
    Serial.println("done");
  }
}

//function used to record audio at sample rate for a fixed nmber of samples
void record_audio() {
  int sample_num = 0;    // counter for samples
  int enc_index = strlen(PREFIX) - 1;  // index counter for encoded samples
  float time_between_samples = 1000000 / SAMPLE_FREQ;
  int value = 0;
  char raw_samples[3];   // 8-bit raw sample data array
  memset(speech_data, 0, sizeof(speech_data));
  sprintf(speech_data, "%s", PREFIX);
  char holder[5] = {0};
  Serial.println("starting");
  uint32_t text_index = enc_index;
  uint32_t start = millis();
  time_since_sample = micros();
  button_state = digitalRead(PIN_1);
  while (!button_state && sample_num < NUM_SAMPLES) { //read in NUM_SAMPLES worth of audio data
    value = analogRead(AUDIO_IN);  //make measurement
    raw_samples[sample_num % 3] = mulaw_encode(value - 1241); //remove 1.0V offset (from 12 bit reading)
    sample_num++;
    if (sample_num % 3 == 0) {
      base64_encode(holder, raw_samples, 3);
      strncat(speech_data + text_index, holder, 4);
      text_index += 4;
    }
    // wait till next time to read
    while (micros() - time_since_sample <= time_between_samples); //wait...
    time_since_sample = micros();
    button_state = digitalRead(PIN_1);
  }
  Serial.println(millis() - start);
  sprintf(speech_data + strlen(speech_data), "%s", SUFFIX);
  Serial.println("out");
}
