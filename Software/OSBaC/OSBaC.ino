//*********
// SETTINGS
//*********

// Denn Namen des Gerätes
#define DEVICE_NAME "Rollo Links Schlafzimmer"

// einen Modus wählen
#define BLIND_MODE
//#define CURTAIN_MODE


//#define* mDNS_mode "ESP32"                           // Wenn man einen mDNS will

//#define* update_interval_time 30                     // Update interval in Tagen, noch nicht verfügbar

// OTA update settings
#define enable_OTA_mode                             // für OTA Updates
#define OTA_port 3232                               // der Standart Port ist 3232
#define OTA_hostname "myESP32"                      // Der Standardt name ist myESP3232, gefolgt immer von "-[MAC]"

// Es ist empfohlen, ein Kennwort festzulegen, wenn die OTA Funktion verwenden wird. Ein Kennwort im Klartext oder in gehashter Form ist möglich.
//#define password_OTA_clear "admin"                  // Klartext
//#define password_OTA_hash "21232f297a57a5a743894a0e4a801fc3"// gehashte Form (admin)

const char* ssid = "FRITZ!Box 6690 LL";
const char* password = "38358216546848753105";

const int status_led_off_time = 60;                 // Zeit bis sich die Statusled ausschaltet in Sekunden

const String apikey = "1234";                       // der API Key
const String apiUrl = "/api/v1";                    // API Url, nur ändern wenn du weißt was du machst

const int status_led = LED_BUILTIN;
const int error_led = LED_BUILTIN;
const int dir_pin = D2;
const int step_pin = D1;
const int en_pin = D3;
const int sensor1_pin = D10;                        // Referenzsensor im Rollomodus
const int sensor2_pin = D9;                         // Endsensor im Vorhangmodus

const uint reference_speed = 2500;                  // Motorreferenzgeschwindigkeit
const uint movement_accel = 750;                    // Motorbeschleunigung

                                                    // Die nachfolgenden Werte werden nur beim Ersten start geschriebn
                                                    // und können danach nur noch per API geändert werden
const uint sensor1_A_Wert = 2000;                   // Auslösewert für Sensor 1
const uint sensor2_A_Wert = 2000;                   // Auslösewert für Sensor 2
const uint64_t end_pos = 350000;                    // Endposition
const uint movement_speed = 5000;                   // Motorgeschwindigkeit

//**************************************************************
// SETTINGS END HERE!!! DONT CHANGE ANYTHING BELOW THIS POINT!!!
//**************************************************************

#define debug_mode

volatile uint64_t cur_pos = 0;
const String Build_Version = "v1.0";
bool disableLED = true;

#ifdef BLIND_MODE
  #ifdef CURTAIN_MODE
    #error only define one Operation Mode!
  #endif
#endif

#ifdef CURTAIN_MODE
  #error Not yet implemented!
#endif

#ifdef password_OTA_clear
  #ifdef password_OTA_hash
    #error only define one password mode hashed or clear!
  #endif
#endif

#include <AccelStepper.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Preferences.h>

WebServer server(80);
AccelStepper stepper(AccelStepper::DRIVER, step_pin, dir_pin);
Preferences settings;

void setup(){
  // Motor Funktion auf Core 1 pinnen
  //xTaskCreatePinnedToCore(motor_move,     // Task function to be called
  //                        "motor_move",   // Name of the task Any Name
  //                        10000,          // available heap memory of the task
  //                        NULL,           // possibly parameter of the task
  //                        2,              // task priority
  //                        NULL,           // used task handle
  //                        1);             // used Core

  // Pin modes
  pinMode(status_led, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(step_pin, OUTPUT);
  pinMode(en_pin, OUTPUT);
  pinMode(sensor1_pin, INPUT);
  pinMode(sensor2_pin, INPUT);

  // Status LED ausschalten
  digitalWrite(status_led, HIGH);
  // Error LED ausschalten
  digitalWrite(error_led, HIGH);

  // Motor Treiber ausschalten
  digitalWrite(en_pin, HIGH);

  Serial.begin(115200);

  settings.begin("osbac", false);

  // Werte beim Ersten Start schreiben
  if(!settings.isKey("sensor1_A_Wert")) settings.putUInt("sensor1_A_Wert", sensor1_A_Wert);
  if(!settings.isKey("sensor2_A_Wert")) settings.putUInt("sensor2_A_Wert", sensor2_A_Wert);
  if(!settings.isKey("end_pos")) settings.putULong64("end_pos", end_pos);
  if(!settings.isKey("movement_speed")) settings.putUInt("movement_speed", movement_speed);

  // Homing
  #ifdef debug_mode
    blind_rev_dev();
  #else
    if(!blind_rev()){
      error_mode("Wegüberschreitung beim Referenz lauf");
      ESP.restart();
    }
  #endif

  connectToWifi();

  #ifdef enable_OTA_mode
    startOTA();
  #endif

  // identify handler
  //server.on(apiUrl + "/identify", identify);

  // api/v1 handle
  server.on(apiUrl, apiV1Handle);

  // main web handler
  server.on("/", webHandler);

  // handler for 404
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  

  // Status LED einschalten
  digitalWrite(status_led, LOW);
}
