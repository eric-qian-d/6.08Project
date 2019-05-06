void handle_record() {
  Serial.println("listening...");
  record_audio();
  Serial.println("sending...");
  Serial.print("\nStarting connection to server...");
  delay(300);
  WiFiClient client;
  bool conn = false;
  for (int i = 0; i < 10; i++) {
    int val = (int)client.connect("608dev.net", 80);
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
    //char holder[10010] = {0};
    Serial.println("Connected to server!");
    Serial.println(client.connected());
    int len = strlen(speech_data);
    // Make a HTTP request:
    client.print("POST /sandbox/sc/jenning/voiceapi.py"); client.print(" HTTP/1.1\r\n");
    client.print("Host: 608dev.net\r\n");
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
      Serial.println(temp_holder);
      client.print(temp_holder);
      //strcat(holder, temp_holder);
      ind += jump_size;
      memset(temp_holder, 0, sizeof(temp_holder));
    }
    client.print("\r\n");
    //Serial.print("\r\n\r\n");
    Serial.println("Through send...");
    //Serial.println(holder);
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
    strcpy(temp_transcript, response);
//    char* trans_id = strstr(response, "transcript");
//    if (trans_id != NULL) {
//      char* foll_coll = strstr(trans_id, ":");
//      char* starto = foll_coll + 2; //starting index
//      char* endo = strstr(starto + 1, "\""); //ending index
//      int transcript_len = endo - starto + 1;
//      char transcript[1000] = {0};
//      strncat(transcript, starto, transcript_len);
//      Serial.println(transcript);
//      strcpy(temp_transcript, transcript);
//    }
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
  while (button_state) {
    button_state = digitalRead(PIN_1);
  }
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

int8_t mulaw_encode(int16_t sample) {
  const uint16_t MULAW_MAX = 0x1FFF;
  const uint16_t MULAW_BIAS = 33;
  uint16_t mask = 0x1000;
  uint8_t sign = 0;
  uint8_t position = 12;
  uint8_t lsb = 0;
  if (sample < 0)
  {
    sample = -sample;
    sign = 0x80;
  }
  sample += MULAW_BIAS;
  if (sample > MULAW_MAX)
  {
    sample = MULAW_MAX;
  }
  for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
    ;
  lsb = (sample >> (position - 4)) & 0x0f;
  return (~(sign | ((position - 5) << 4) | lsb));
}
