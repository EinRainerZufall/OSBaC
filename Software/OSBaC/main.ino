// Im Fehlerfall eine Endlos Schleife
void error_mode(String str){
  digitalWrite(status_led, HIGH);
  Serial.print("Error: ");
  Serial.println(str);
  Serial.println("Error Modus aktiv! Parameter anpassen und Controller neustarten!");

  while(1){
    digitalWrite(error_led, HIGH);
    delay(150);
    digitalWrite(error_led, LOW);
    delay(100);
  }
}


// prüfen ob ein string nur aus Zahlen besteht
bool isValidNumber(String str){
  if(str == ""){
    return false;
  }
  // ToDO: Wenn das erste Zeichen ein minus ist dann weg damit -> aber danach eine kontrolle ob der str leer ist!
  for(byte i = 0; i < str.length(); i++){
    if(!isDigit(str.charAt(i))){
      return false;
    }
  }
  return true;
}


// Wie die Orginale map Funktion mit uint64_t
uint64_t map_uint64_t(uint64_t x, uint64_t in_min, uint64_t in_max, uint64_t out_min, uint64_t out_max) {
    const uint64_t run = in_max - in_min;
    if(run == 0){
        log_e("map(): Invalid input range, min == max");
        return -1; // AVR returns -1, SAM returns 0
    }
    const uint64_t rise = out_max - out_min;
    const uint64_t delta = x - in_min;
    return (delta * rise) / run + out_min;
}


// Die loop
void loop(){
  unsigned long curTime = millis();

  // Status LED ausschalten
  if(curTime >= (status_led_off_time * 1000) && disableLED){
    digitalWrite(status_led, HIGH);
    Serial.println("Disable status LED");
    disableLED = false;
  }

  #ifdef update_interval_time
    // Nach Updates suchen
    if(curTime >= (update_interval_time * 24 * 60 * 60 * 1000)){
      //ToDo: evtl mal eine Update routine schreiben
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
