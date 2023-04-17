bool connectToWifi(){
  WiFi.mode(WIFI_STA);
  //WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);//<- ToDo: if there is no wifi credentials start AP
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // Punkte schreiben bis WiFi da ist
  int counter = 0;
  while(WiFi.status() != WL_CONNECTED){
    delay(250);
    Serial.print(".");
    counter++;
    if(counter>80) error_mode("Keine Verbindung mit dem WLAN möglich");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // mDNS starten wenn Aktiv
  #ifdef mDNS_mode
    if(MDNS.begin(mDNS_mode)){
     Serial.println("MDNS responder started");
    }
  #endif

  return true;
}


void startOTA(){
  #ifdef port_OTA
    ArduinoOTA.setPort(port_OTA);
  #endif
  #ifdef hostname_OTA
    ArduinoOTA.setHostname(hostname_OTA);
  #endif
  #ifdef password_OTA_clear
    ArduinoOTA.setPassword(password_OTA_clear);
  #endif
  #ifdef password_OTA_hash
    ArduinoOTA.setPasswordHash(password_OTA_hash);
  #endif

  // OTA handle
  ArduinoOTA
    .onStart([](){
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([](){
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total){
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error){
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();  
}


// 404 Handle
void handleNotFound(){
  #ifdef debug_mode
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for(uint8_t i = 0; i < server.args(); i++){
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
  #else
    server.send(404);
  #endif
}


// main web handle
void webHandler(){
  //unsigned long startTime = micros(); // zum testen
  int argCount = server.args();
  
  // evtl. die Funktion analogReadMilliVolts benutzen
  uint16_t sen1 = analogRead(sensor1_pin);
  uint16_t sen2 = analogRead(sensor2_pin);

  uint64_t allSeconds = esp_timer_get_time() / 1000000;
  int runHours = (allSeconds % 86400) / 3600;
  int runDays = allSeconds / 86400;
  int secsRemaining = allSeconds % 3600;
  int runMinutes = secsRemaining / 60;
  int runSeconds = secsRemaining % 60;

  const String refreshPeriod = "1000"; // in ms

  String sen1_status = "offen";
  String sen2_status = "offen";

  String key_out = "&#8194;";

  #ifdef BLIND_MODE
  String mode = "Rollo Modus";
  #else
  String mode = "Vorhang Modus";
  #endif

  if(sen1 < sensor1_A_Wert) sen1_status = "geschlossen";
  if(sen2 < sensor2_A_Wert) sen2_status = "geschlossen";

  if(server.hasArg("apikey") && server.arg("apikey") == apikey) key_out = apikey;

  String webSite =  "<!DOCTYPE html>\n"
                    "<html lang=\"de\">\n"
                    "<head>\n"
                    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                    "<meta charset=\"utf-8\">\n"
                    "<script>\n"
                    "function refresh(refreshPeriod){setTimeout('location.reload(true)', refreshPeriod);}\n"
                    "window.onload = refresh(" + refreshPeriod + ");\n"
                    "</script>\n"
                    "</head>\n"
                    "<h1>OSBaC</h1>\n"
                    "<table align=\"left\" border=\"1\" cellpadding=\"1\" cellspacing=\"1\" style=\"width:270px\">\n"
                    "<tbody>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Name</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" DEVICE_NAME "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">IP</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + WiFi.localIP().toString() + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Version</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + Build_Version + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Mode</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + mode + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Aktuelle Position</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + cur_pos + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Sensor 1 Status</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + sen1_status + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Sensor 1 Wert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + sen1 + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Sensor 1 Ausl&ouml;sewert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + settings.getUInt("sensor1_A_Wert") + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Sensor 2 Status</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + sen2_status + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Sensor 2 Wert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + sen2 + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Sensor 2 Ausl&ouml;sewert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + settings.getUInt("sensor2_A_Wert") + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Motor Geschwindigkeit</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + settings.getUInt("movement_speed") + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Endposition</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + settings.getULong64("end_pos") + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:230px\">Laufzeit</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" + runDays + "d " + runHours + "h " + runMinutes + "m " + runSeconds + "s</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td colspan=\"2\" style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + key_out + "</td>\n"
                    "</tr>\n"
                    "</tbody>\n"
                    "</table>\n"
                    "</html>";

  server.send(200, "text/html", webSite);

  #ifdef debug_mode
    for(int i = 0; i < argCount; i++){
      String argName = server.argName(i);
      Serial.print(argName);
      Serial.print(" = ");
      if(server.hasArg(argName)){
        Serial.println(server.arg(argName));
      }else{
        Serial.print(i);
        Serial.println(" fehlt");
      }
    }
  #endif

  //unsigned long endTime = micros(); // zum testen
  //Serial.print("execution was "); // immer noch zum testen
  //Serial.print(endTime-startTime); // siehe eine Zeile weiter oben
  //Serial.print("µs "); // siehe immer noch nach oben
  //Serial.println(server.client().remoteIP().toString());
}


// api/v1 handle
// apikey -> muss den apikey haben
// setsto -> hält die aktuelle Bewegung an
// setpos -> 0 - 100, setzt die position
//             - 0 heist keine Lichtdurchlässigkeit
//             - 100 heist volle Lichtdurchlässigkeit
// getpos -> gibt die aktuelle position aus
// getmov -> gibt die Bewegungsrichtung zurück
//            - 0 -> pos wird kleiner, Rollo nach unten
//            - 1 -> pos wird größer, Rollo nach oben
//            - 2 -> Keine bewegung
// getnam -> gibt den Namen zurück
// getver -> gibt die Version zurück
//
// Achtung nur benutzen wenn man weiß was man macht!!!
// setsen1awert -> setzt denn Auslösewert für Sensor 1
// setsen2awert -> setzt denn Auslösewert für Sensor 2
// setmotorendpos -> setzt die Endposition vom Motor
// setmotorspeed -> setzt die geschwindigkeit der Motor
void apiV1Handle(){
  int argCount = server.args();
  String message;

  if(!server.hasArg("apikey")){
    server.send(403);
    return;
  }else if(server.arg("apikey") != apikey){
    server.send(403);
    return;
  }

  // Stop Aktion
  if(server.hasArg("setsto")){
    message += "setsto=1\n";
    
    if(motor_stop()){
      message += "ACK\n";
    }else{
      message += "NAK\n";
    }

    message += "----------\n";

    server.send(200, "text/plain", message);
    return;
  }

  // Denn Auslösewert für Sensor 1 setzen
  if(server.hasArg("setsen1awert")){
    message += "setsen1awert:";
    message += server.arg("setsen1awert");
    message += "\n";

    //ist es eine Zahl?
    if(!isValidNumber(server.arg("setsen1awert"))){
      message += "Zahlencheck:NAK\n";
      message += "----------\n";
      server.send(200, "text/plain", message);
      return;
      // mach ma da jetzt noch was?
    }else{
      message += "Zahlencheck:ACK\n";
    }

    if(settings.putUInt("sensor1_A_Wert", server.arg("setsen1awert").toInt()) > 1){
      message += "ACK\n";
    }else{
      message += "NAK\n";
    }

    message += "setsen2awert:";
    message += settings.getUInt("sensor1_A_Wert");
    message += "\n";
    message += "----------\n";

    server.send(200, "text/plain", message);
    return;
  }

  // Denn Auslösewert für Sensor 2 setzen
  if(server.hasArg("setsen2awert")){
    message += "setsen2awert:";
    message += server.arg("setsen2awert");
    message += "\n";

    //ist es eine Zahl?
    if(!isValidNumber(server.arg("setsen2awert"))){
      message += "Zahlencheck:NAK\n";
      message += "----------\n";
      server.send(200, "text/plain", message);
      return;
      // mach ma da jetzt noch was?
    }else{
      message += "Zahlencheck:ACK\n";
    }

    if(settings.putUInt("sensor2_A_Wert", server.arg("setsen2awert").toInt()) > 1){
      message += "ACK\n";
    }else{
      message += "NAK\n";
    }
    
    message += "setsen2awert:";
    message += settings.getUInt("sensor2_A_Wert");
    message += "\n";
    message += "----------\n";

    server.send(200, "text/plain", message);
    return;
  }

  // Die Motor geschwindigkeit prüfen und schreiben
  if(server.hasArg("setmotorspeed")){
    message += "setmotorspeed:";
    message += server.arg("setmotorspeed");
    message += "\n";

    //ist es eine Zahl?
    if(!isValidNumber(server.arg("setmotorspeed"))){
      message += "Zahlencheck:NAK\n";
      message += "----------\n";
      server.send(200, "text/plain", message);
      return;
      // mach ma da jetzt noch was?
    }else{
      message += "Zahlencheck:ACK\n";
    }

    // Werte Bereich prüfen!
    if(server.arg("setmotorspeed").toInt() <= 0){
      message += "Wertebereich:NAK\n";
      message += "----------\n";
      server.send(200, "text/plain", message);
      return;
      // mach ma da jetzt noch was?
    }else{
      message += "Wertebereich:ACK\n";
    }

    if(settings.putUInt("movement_speed", server.arg("setmotorspeed").toInt()) > 1){
      message += "ACK\n";
    }else{
      message += "NAK\n";
    }
    
    message += "setmotorspeed:";
    message += settings.getUInt("movement_speed");
    message += "\n";
    message += "----------\n";

    server.send(200, "text/plain", message);
    return;
  }


  // die Endposition vom Motor setzen
  if(server.hasArg("setmotorendpos")){
    message += "setmotorendpos:";
    message += server.arg("setmotorendpos");
    message += "\n";

    //ist es eine Zahl?
    if(!isValidNumber(server.arg("setmotorendpos"))){
      message += "Zahlencheck:NAK\n";
      message += "----------\n";
      server.send(200, "text/plain", message);
      return;
      // mach ma da jetzt noch was?
    }else{
      message += "Zahlencheck:ACK\n";
    }

    char *endptr;
    uint64_t temp = strtoull(server.arg("setmotorendpos").c_str(), &endptr, 10);

    // Werte Bereich prüfen!
    if(temp <= 0){
      message += "Wertebereich:NAK\n";
      message += "----------\n";
      server.send(200, "text/plain", message);
      return;
      // mach ma da jetzt noch was?
    }else{
      message += "Wertebereich:ACK\n";
    }

    if(settings.putULong64("end_pos", temp) > 1){
      message += "ACK\n";
    }else{
      message += "NAK\n";
    }
    
    message += "setmotorendpos:";
    message += settings.getULong64("end_pos");
    message += "\n";
    message += "----------\n";

    server.send(200, "text/plain", message);
    return;
  }


  // Motor bewegung überprüfen und beauftragen
  if(server.hasArg("setpos")){
    String temp = server.arg("setpos");

    message += "setpos:";
    message += temp;
    message += "\n";

    //ist es eine Zahl?
    if(!isValidNumber(temp)){
      message += "Zahlencheck:NAK\n";
      server.send(200, "text/plain", message);
      return;
    // ist es zwischen 0 und 100?
    }else if(temp.toInt() > 100 || temp.toInt() < 0){
      message += "Zahlencheck:NAK\n";
      server.send(200, "text/plain", message);
      return;
    }

    message += "Zahlencheck:ACK\n";

    if(motor_move_check(temp.toInt())){
      message += "ACK\n";
    }else{
      message += "NAK\n";
    }

    message += "----------\n";
  }

  // Positions Abfrage
  if(server.hasArg("getpos")){
    message += "curpos:";
    message += cur_pos;
    message += "\n";
    //message += "tarpos:";
    //message += "0"; //ToDo:
    //message += "\n";
    message += "----------\n";
  }

  // Bewegungs Abfrage
  if(server.hasArg("getmov")){
    message += "getmov:";
    message += "2"; //ToDo:
    message += "\n";
    message += "----------\n";
  }

  // Names Abfrage
  if(server.hasArg("getnam")){
    message += "getnam:";
    message += DEVICE_NAME;
    message += "\n";
    message += "----------\n";
  }

  // Versions Abfrage
  if(server.hasArg("getver")){
    message += "getver:";
    message += Build_Version;
    message += "\n";
    message += "----------\n";
  }
  
  server.send(200, "text/plain", message);  
}
