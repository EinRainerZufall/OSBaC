//*********
// SETTINGS
//*********

// choose one mode
#define BLIND_MODE
//#define CURTAIN_MODE


//#define* mDNS_mode "ESP32"                           // if you want a mDNS

//#define* update_interval_time 30                     // update interval in days, if you do not want to check for updates comment it out

// OTA update settings
//#define enable_OTA_mode                             // enable OTA update mode
#define OTA_port 3232                               // Port defaults to 3232
#define OTA_hostname "myESP32"                      // Hostname defaults to esp3232-[MAC]

// It is recommended to set a password when you use the OTA update function, you can set the pw in clear or hashed form
//#define password_OTA_clear "admin"                  // No authentication by default
//#define password_OTA_hash "21232f297a57a5a743894a0e4a801fc3"// Password can be set with it's md5 value as well (admin)

const char* ssid = "FRITZ!Box 6690 LL";
const char* password = "38358216546848753105";
#define debug_mode

const int status_led_off_time = 60;                 // time after the status LED switches off in sec

const String apikey = "1234";                       // choose your ouwn API key
const String apiUrl = "/api/v1";                    // do not change this unlike you know what your doing

const int status_led = D0;
const int dir_pin = D2;
const int step_pin = D1;
const int en_pin = D3;
const int sensor1_pin = D10;                        // sensor for reference run in blind mode
const int sensor2_pin = D9;                         // also A10 and A9 possible

const long end_pos = 350000;                        // only used in debug mode
const int movement_speed = 5000;                    // motor speed
const int reference_speed = 2500;                   // motor speed for the reference run
const int movement_accel = 750;                     // motor acceleration

//**************************************************************
// SETTINGS END HERE!!! DONT CHANGE ANYTHING BELOW THIS POINT!!!
//**************************************************************

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

WebServer server(80);
AccelStepper stepper(AccelStepper::DRIVER, step_pin, dir_pin); 

void setup(){
  // set pin modes
  pinMode(status_led, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(step_pin, OUTPUT);
  pinMode(en_pin, OUTPUT);
  pinMode(sensor1_pin, INPUT_PULLDOWN);
  pinMode(sensor2_pin, INPUT_PULLDOWN);

  // switch status LED off
  digitalWrite(status_led, LOW);
  // diable motor controller
  digitalWrite(en_pin, HIGH);

  Serial.begin(115200);

  // homing routine
  #ifdef debug_mode
    blind_rev_dev();
  #else
    if(!blind_rev()){
      ESP.restart();
      //ToDo: what else?
    }
  #endif

  connectToWifi();

  #ifdef enable_OTA_mode
    startOTA();
  #endif

  // identify handler
  //server.on(apiUrl + "/identify", identify);

  // positinstate handler <- ToDo: not work, to much to handle the wifi and motor control, solution?
  //server.on(apiUrl + "/positionstate", []() {
  //  server.send(200, "text/plain", positionState);
  //});

  // main web handler
  server.on("/", webHandler);

  // handler for 404
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  

  // switch status LED off
  digitalWrite(status_led, HIGH);
}
