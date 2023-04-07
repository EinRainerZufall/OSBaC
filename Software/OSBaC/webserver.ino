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
  //int argCount = server.args();
  
  uint16_t sen1 = analogRead(sensor1_pin);
  uint16_t sen2 = analogRead(sensor2_pin);

  String sen1_status = "offen";
  String sen2_status = "offen";

  #ifdef BLIND_MODE
  String mode = "Rollo Modus";
  #else
  String mode = "Vorhang Modus";
  #endif

  if(sen1 < sensor1_A_Wert) sen1_status = "geschlossen";

  if(sen2 < sensor2_A_Wert) sen2_status = "geschlossen";

  String webSite =  "<!DOCTYPE html>\n"
                    "<html lang=\"de\">\n"
                    "<h1>OSBaC</h1>\n"
                    "<table align=\"left\" border=\"1\" cellpadding=\"1\" cellspacing=\"1\" style=\"width:270px\">\n"
                    "<tbody>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Name</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:130px\">" DEVICE_NAME "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">IP</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + WiFi.localIP().toString() + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Version</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + Build_Version + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Mode</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + mode + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Sensor 1 Status</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + sen1_status+ "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Sensor 1 Wert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + sen1 + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Sensor 1 Ausl&ouml;sewert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + sensor1_A_Wert + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Sensor 2 Status</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + sen2_status + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Sensor 2 Wert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + sen2 + "</td>\n"
                    "</tr>\n"
                    "<tr>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; width:170px\">Sensor 2 Ausl&ouml;sewert</td>\n"
                    "<td style=\"background-color:#eeeeee; border-color:#000000; text-align:center; width:123px\">" + sensor2_A_Wert + "</td>\n"
                    "</tr>\n"
                    "</tbody>\n"
                    "</table>\n"
                    "</html>";

  server.send(200, "text/html", webSite);

}


// api/v1 handle
void apiV1Handle(){
  server.send(200, "text/plain", "Test");  
}
