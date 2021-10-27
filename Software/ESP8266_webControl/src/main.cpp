#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266LLMNR.h>

#include "I2cDataProtocol.h"
#include "Gyroscope.h"
#include "StateVector.h"
#include "DifferentialChassis.h"
#include "DisplayClass.h"
#include "EEPROM_data.h"

ESP8266WebServer server(80);
WiFiManager wifiManager;
StateVector stateVector;
DisplayClass displayOLED;
DifferentialChassis chassis;
const char deviceName[] = "RoboTommy";
const char password[] = "adminadmin";
char passwordHash[33];
char cookieHash[42];

const uint8_t I2C_ADDRESS_NANO = 4;

const uint8_t CONTROL_PERIOD = 10;
uint32_t prevControlTime = 0;
const uint16_t communicationTimeout = 1000;
uint32_t prevCommunicationTime = 0;

int16_t gyroOffsets[6] = {0, }; //xAccel, yAccel, zAccel, xGyro, yGyro, zGyro
EEPROM_data gyroOffsetsEEPROM(gyroOffsets, 6*sizeof(gyroOffsets[0]));

bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    String cookie = server.header("Cookie");
    if (cookie.indexOf(cookieHash) != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

void handleLogin() {
    String msg;
    if (server.hasHeader("Cookie")) {
        Serial.print("Found cookie: ");
        String cookie = server.header("Cookie");
        Serial.println(cookie);
    }
    if (server.hasArg("DISCONNECT")) {
        Serial.println("Disconnection");
        server.sendHeader("Location", "/login");
        server.sendHeader("Cache-Control", "no-cache");
        server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
        server.send(301);
        return;
    }
    if (server.hasArg("PASSWORD")) {
        if (server.arg("PASSWORD") == password) {
        server.sendHeader("Location", "/");
        server.sendHeader("Cache-Control", "no-cache");
        server.sendHeader("Set-Cookie", cookieHash);
        server.send(301);
        Serial.println("Log in Successful");
        return;
        }
        msg = "Wrong password! try again.";
        Serial.println("Log in Failed");
    }
    File f = SPIFFS.open("/login.html", "r");
    server.streamFile(f, "text/html");
    f.close();
}

void handleRoot() {
    String header;
    if (!is_authenticated()) {
        server.sendHeader("Location", "/login");
        server.sendHeader("Cache-Control", "no-cache");
        server.send(301);
        return;
    }

    File f = SPIFFS.open("/webApp.html", "r");
    server.streamFile(f, "text/html");
    f.close();
}

void handleNotFound() {
    File f = SPIFFS.open("/notFound.html", "r");
    server.streamFile(f, "text/html");
    f.close();
}
void handleJSData() {
    stateVector.joystickX = server.arg(0).toInt();
    stateVector.joystickY = server.arg(1).toInt();

    server.send(200, "text/plain", ""); //return an HTTP 200 (OK)
    prevCommunicationTime = millis();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    exSonarDist.setVarAddress(&(stateVector.sonarDistance));
    exEngineLeftSpeed.setVarAddress(&(stateVector.engineLeftSpeed));
    exEngineRightSpeed.setVarAddress(&(stateVector.engineRightSpeed));
    I2cExchangeData::beginMaster(I2C_ADDRESS_NANO);

    displayOLED.init();
    displayOLED.writeWelcome();
    delay(1000);
    char textOut[80];
    sprintf(textOut, "Connect to %s WiFi to pass credentials of local WiFi", deviceName);
    displayOLED.writeString(textOut);

    //WiFi.mode(WIFI_AP);
    WiFi.hostname(deviceName);
    wifiManager.autoConnect(deviceName);
    LLMNR.begin(deviceName);
    //IPAddress Ip(192, 168, 1, 1);
    //IPAddress NMask(255, 255, 255, 0);
    /*WiFi.softAPConfig(Ip, Ip, NMask);
    if(!WiFi.softAP(deviceName, password)) {
        Serial.println("WiFi.softAP failed.(Password too short?)");
        return;
    }*/

    sprintf(textOut, "Connect to %s WiFi and http://%s or IP %s", WiFi.SSID().c_str(), deviceName, WiFi.localIP().toString().c_str());
    displayOLED.writeString(textOut);

    Serial.print("WiFi connected, IP address:");
    Serial.println(WiFi.localIP());

    MD5Builder md5;
    md5.begin();
    md5.add(password);
    md5.calculate();
    md5.getChars(passwordHash);
    sprintf(cookieHash, "psdHash=%s", passwordHash);

    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS Mount failed");
    } 
    else {
        Serial.println("SPIFFS Mount succesfull");
    }
    server.on("/", handleRoot);
    server.serveStatic("/style.css", SPIFFS, "/style.css");
    server.on("/login", handleLogin);
    server.on("/jsData.html", handleJSData);
    server.onNotFound(handleNotFound);

    //here the list of headers to be recorded
    const char * headerkeys[] = {"Cookie"} ;
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    //ask server to track these headers
    server.collectHeaders(headerkeys, headerkeyssize);
    server.begin();

    gyroOffsetsEEPROM.read();
    initiateGyroscope(gyroOffsets);
    chassis.setDirSpeed(0, 0);
}

void loop()
{
    server.handleClient();
    if(millis() > prevControlTime + CONTROL_PERIOD) {
        prevControlTime = millis();

        //Get Inputs
        I2cExchangeData::exchange();
        updateGyroData(stateVector.ypr);

        //Automatic Control
        if(millis() > prevCommunicationTime + communicationTimeout) {
            stateVector.joystickX = 0;
            stateVector.joystickY = 0;
        }
        chassis.setDirSpeed(stateVector.direction, stateVector.speed);

        //Set Outputs
        chassis.updateOutput(stateVector.ypr[0]);
        //stateVector.engineLeftSpeed = chassis.getLeftSpeed();
        //stateVector.engineRightSpeed = chassis.getRightSpeed();
        stateVector.engineLeftSpeed = (stateVector.joystickY + ((stateVector.joystickY >= 0) ? 1 : -1) * stateVector.joystickX) * 2.55;
        stateVector.engineRightSpeed = (stateVector.joystickY - ((stateVector.joystickY >= 0) ? 1 : -1) * stateVector.joystickX) * 2.55;

        stateVector.engineLeftSpeed = constrain(stateVector.engineLeftSpeed, -255, 255);
        stateVector.engineRightSpeed = constrain(stateVector.engineRightSpeed, -255, 255);

        //Serial.printf("%d %d\n", stateVector.engineLeftSpeed, stateVector.engineRightSpeed);
        //Serial.printf("%f, %f, %f\n", stateVector.ypr[0], stateVector.ypr[1], stateVector.ypr[2]);
    }
}