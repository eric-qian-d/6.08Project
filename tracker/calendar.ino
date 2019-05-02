
void fetch_calendar_data() {
  
  
  // next, get your location
  char calendar_request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
  char calendar_response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

  sprintf(calendar_request_buffer, "GET http://608dev.net/sandbox/sc/jenning/calendar/calendarapi.py HTTP/1.1\r\n");
  strcat(calendar_request_buffer, "Host: 608dev.net\r\n");
  strcat(calendar_request_buffer, "\r\n"); //new line from header to body
  do_http_request("608dev.net", calendar_request_buffer, calendar_response_buffer, 50, RESPONSE_TIMEOUT, true);
  Serial.println(calendar_response_buffer);
}
