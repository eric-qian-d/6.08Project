

void fetch_weather_data(String ip) {
  char weather_request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
  char weather_response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

  char weather_output[50];
  sprintf(weather_request_buffer, "GET http://608dev.net/sandbox/sc/lyy/location.py?ip=%s HTTP/1.1\r\n", ip);
  strcat(weather_request_buffer, "Host: 608dev.net\r\n");
  strcat(weather_request_buffer, "\r\n"); //new line from header to body
  do_http_request("608dev.net", weather_request_buffer, weather_output, 50, RESPONSE_TIMEOUT, true);
  Serial.println(weather_output);

  tft.fillScreen(TFT_BLACK); //fill background
  tft.setCursor(0, 0, 1); // set the cursor
}
