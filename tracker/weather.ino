

void fetch_weather_data() {
  // first, get public IP address
  connectWifi();
  char ip_address[30];
  char ip_request_buffer[IN_BUFFER_SIZE];  
  sprintf(ip_request_buffer, "GET https://api.ipify.org/ HTTP/1.1\r\n");
  strcat(ip_request_buffer, "Host: api.ipify.org\r\n");
  strcat(ip_request_buffer, "\r\n"); //new line from header to body
  do_http_request("api.ipify.org", ip_request_buffer, ip_address, 30, RESPONSE_TIMEOUT, true);
  
  Serial.println(ip_address);
  
  toggle_selection = 0;
  tft.fillScreen(TFT_BLACK); //fill background
  char item_response_buffer[OUT_BUFFER_SIZE];
  char item_request_buffer[IN_BUFFER_SIZE];
  sprintf(item_request_buffer, "GET http://608dev.net/sandbox/sc/cxy99/recommend.py?IP=%s HTTP/1.1\r\n", ip_address);
  strcat(item_request_buffer, "Host: 608dev.net\r\n");
  strcat(item_request_buffer, "\r\n"); //new line from header to body
  do_http_request("608dev.net", item_request_buffer, item_response_buffer, 50, RESPONSE_TIMEOUT, true);
  tft.setCursor(2, 10, 1); // set the cursor
  tft.println(item_response_buffer);
  disconnectWifi();

//  
//  // next, get your location
//  char weather_request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
//  char weather_response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
//
//  sprintf(weather_request_buffer, "GET http://608dev.net/sandbox/sc/lyy/location.py?ip=%s HTTP/1.1\r\n", ip_address);
//  strcat(weather_request_buffer, "Host: 608dev.net\r\n");
//  strcat(weather_request_buffer, "\r\n"); //new line from header to body
//  do_http_request("608dev.net", weather_request_buffer, weather_response_buffer, 50, RESPONSE_TIMEOUT, true);
//  Serial.println(weather_response_buffer);
}
