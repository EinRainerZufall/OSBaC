void loop(){
  unsigned long curTime = millis();

  // Status LED ausschalten
  if(curTime >= (status_led_off_time * 1000) && disableLED){
    digitalWrite(status_led, LOW);
    Serial.println("Disable status LED");
    disableLED = false;
  }

  #ifdef update_interval_time
    // Nach Updates suchen
    if(curTime >= (update_interval_time * 24 * 60 * 60 * 1000)){
      //ToDO: evtl mal eine Update routine schreiben
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
