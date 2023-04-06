bool connectToWifi(){
  WiFi.mode(WIFI_STA);
  //WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);//<- ToDo: if there is no wifi credentials start AP
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // wait until the wifi is connected <- ToDo: 
  while(WiFi.status() != WL_CONNECTED){
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // start mDNS if enabled
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


// if the handle is not found
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
  // usedMode = true -> Blind
  // usedMode = false -> Curtain

  int argCount = server.args();

  server.send(200, "text/plain", "TEST 123 !");

}
