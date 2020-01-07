#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DHT.h>
#include "config.h"

#ifndef STASSID
#define STASSID CONFIG_SSID
#define STAPSK  CONFIG_PASSWD
#define DHTPIN D5
#define DHTTYPE DHT11
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

const int thermostat = 2;
const int relay1 = 0;
const int relay2 = 4;
const int relay3 = 5;

void handleRoot() {
  server.send(200, "text/plain", "Thermostat 2000");
  server.send(200, "text/plain", STASSID);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  pinMode(thermostat, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  digitalWrite(thermostat, 1);
  digitalWrite(relay1, 1);
  digitalWrite(relay2, 1);
  digitalWrite(relay3, 1);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  IPAddress ip(192,168,0,111);   
  IPAddress gateway(192,168,0,1);   
  IPAddress subnet(255,255,255,0);   
  WiFi.config(ip, gateway, subnet);
  WiFi.hostname("Thermostat");
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {;
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/temp", []() {
    float t = dht.readTemperature();
    server.send(200, "text/plain", String(t));
  });

  server.on("/humi", []() {
    float h = dht.readHumidity();
    server.send(200, "text/plain", String(h));
  });

  server.on("/thermostat/on", []() {
    digitalWrite(thermostat, 0);
    server.send(200, "text/plain", "Relay ON");
  });

   server.on("/thermostat/off", []() {
    digitalWrite(thermostat, 1);
    server.send(200, "text/plain", "Relay OFF");
  });

  server.on("/relay1/on", []() {
    digitalWrite(relay1, 0);
    server.send(200, "text/plain", "Relay ON");
  });
  server.on("/relay1/off", []() {
    digitalWrite(relay1, 1);
    server.send(200, "text/plain", "Relay OFF");
  });

  server.on("/relay2/on", []() {
    digitalWrite(relay2, 0);
    server.send(200, "text/plain", "Relay ON");
  });
  server.on("/relay2/off", []() {
    digitalWrite(relay2, 1);
    server.send(200, "text/plain", "Relay OFF");
  });

 server.on("/relay3/on", []() {
    digitalWrite(relay3, 0);
    server.send(200, "text/plain", "Relay ON");
  });
  server.on("/relay3/off", []() {
    digitalWrite(relay3, 1);
    server.send(200, "text/plain", "Relay OFF");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
