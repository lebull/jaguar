/*
  HTTP Advanced Authentication example
  Created Mar 16, 2017 by Ahmed El-Sharnoby.
  This example code is in the public domain.
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

//Range stuffs
const int trigPin = D2;
const int echoPin = D1;
const int doorPin = D3;

const int DOOR_TOGGLE_DURATION = 500;

bool buttonLock = false;
unsigned long buttonLockLastTimestamp;
const unsigned long buttonLockDuration = 15 * 1000;

//Webstuffs
const char* ssid     = "FireStans";
const char* password = "chargeyourbattery";
const char* networkHostname = "Jaguar";
const char* www_username = "voyage";
const char* www_password = "g]'N$+4pG^<=xS,t^9vstz!'VmSf23Ps";
// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  setupOTA();
  setupWifi();
  setupWeb();
  setupGPIO();

  ArduinoOTA.begin();
}

void setupWifi(){
 WiFi.mode(WIFI_STA);
  WiFi.hostname(networkHostname);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
}

void setupOTA(){
 ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void setupWeb(){

  
  server.on("/", []() {
    if (!server.authenticate(www_username, www_password))
      //Basic Auth Method with Custom realm and Failure Response
      //return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
      //Digest Auth Method with realm="Login Required" and empty Failure Response
      //return server.requestAuthentication(DIGEST_AUTH);
      //Digest Auth Method with Custom realm and empty Failure Response
      //return server.requestAuthentication(DIGEST_AUTH, www_realm);
      //Digest Auth Method with Custom realm and Failure Response
    {
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    }
    Serial.print("hit on /");
    server.send(200, "text/plain", "Login OK");
  });

  server.on("/door/isMoving", [](){
    if (!server.authenticate(www_username, www_password)){
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    }

        if(buttonLock){
          server.send(200, "text/plain", "1");
        }else{
          server.send(200, "text/plain", "0");
        }
        
  });

  server.on("/range", []() {
     Serial.print("hit on /range\n");
    //Auth
    if (!server.authenticate(www_username, www_password)){
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    }

    unsigned long range = getRange();
    //long range = 1337;
    //Serial.printf("Distance: %ld", range);
    char message[32];
    sprintf(message, "%ld", range);
    server.send(200, "text/plain", message);
  });

   server.on("/door/toggle", []() {
     Serial.print("hit on /door/toggle\n");
    //Auth
    if (!server.authenticate(www_username, www_password)){
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    }

    bool success = toggleDoor();

    if(success){
      server.send(200, "text/plain", "1");
    }else{
      server.send(400, "text/plain", "0");
    }
  });

  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void setupGPIO(){
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(doorPin, OUTPUT);
  
  digitalWrite(trigPin, LOW);  // Added this line
  digitalWrite(doorPin, HIGH);  // Added this line
}

long getRange(){
  long duration, detectedDistance, assumedDistance;

  assumedDistance = 0;

  for(int i = 0; i < 50; i++){
    digitalWrite(trigPin, LOW); 
    delayMicroseconds(2); 
    
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); 
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    detectedDistance = duration*0.034/2;

    if(assumedDistance == 0 || 
      (detectedDistance < assumedDistance && detectedDistance != 0)){
      assumedDistance = detectedDistance;
    }
    delayMicroseconds(1);
  }
  return assumedDistance;
}

bool toggleDoor(){
  if(millis() - buttonLockLastTimestamp < buttonLockDuration)
  {
    return false;
  }
  
  digitalWrite(doorPin, LOW);
  delay(DOOR_TOGGLE_DURATION);
  digitalWrite(doorPin, HIGH);
  
  buttonLock = true;
  buttonLockLastTimestamp = millis();
  Serial.println("Toggling door");
  return true;
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  delay(10);
}






