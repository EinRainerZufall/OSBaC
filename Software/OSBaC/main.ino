void loop(){
  unsigned long curTime = millis();

  // switch status LED off
  if(curTime >= (status_led_off_time * 1000) && disableLED){
    digitalWrite(status_led, LOW);
    Serial.println("Disable status LED");
    disableLED = false;
  }

  #ifdef update_interval_time
    // check for updates
    if(curTime >= (update_interval_time * 24 * 60 * 60 * 1000)){
      //ToDO: write update routine!
      Serial.println("Check for updates!");
      delay(10);
      Serial.println("no updates found!");
    }
  #endif
  
  server.handleClient();

  #ifdef enable_OTA_mode
      ArduinoOTA.handle();
  #endif
   
}
