#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <time.h>
#include "config.h"

#ifndef STASSID
#define STASSID CONFIG_SSID
#define STAPSK  CONFIG_PASSWD
#define DHTPIN D5
#define DHTTYPE DHT11
#endif

//Wifi
const char* ssid = STASSID;
const char* password = STAPSK;

//Time
int timezone = 3;
int dst = 0;

//SQL
WiFiClient client;                 // Use this for WiFi instead of EthernetClient
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;
char INSERT_SQL[] = "INSERT INTO `esp`.`log` (`date`, `temperature`, `humidity`) VALUES (FROM_UNIXTIME(%d), '%f', '%f');";
char query[128];

//DHP
DHT dht(DHTPIN, DHTTYPE);

//Relays
const int thermostat = 2;
const int relay1 = 0;
const int relay2 = 4;
const int relay3 = 5;

//Buttons
const int button1 = 15;
const int button2 = 13;

//Leds
const int led1 = 12;
const int led2 = 16;

//Variables
int ledState = LOW;
int loadLed = HIGH;
int buttonState;             
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void getWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  /*IPAddress ip(192,168,0,111);   
  IPAddress gateway(192,168,0,1);   
  IPAddress subnet(255,255,255,0);   
  WiFi.config(ip, gateway, subnet);*/
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    loadLed = !loadLed;
    digitalWrite(led1, loadLed);
    digitalWrite(led2, !loadLed);
    delay(500);
  }
}

void getSql() {
  //IPAddress server_addr(xx,xx,xx,xx);       // IP of the MySQL *server* here
  char user[] = CONFIG_SQL_USER;              // MySQL user login username
  char password[] = CONFIG_SQL_PASSWD;

  Serial.print("Connecting to SQL...  ");
  if (conn.connect(server_addr, 3306, user, password)){
    Serial.println("OK.");
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
  }else{
    Serial.println("FAILED.");
    while(true){
      loadLed = !loadLed;
      digitalWrite(led1, loadLed);
      digitalWrite(led2, loadLed);
      delay(1000);
    }
  }
  cursor = new MySQL_Cursor(&conn);
}

void getTime(){
  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
}

void setRelay(){
  pinMode(thermostat, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(thermostat, 1);
  digitalWrite(relay1, 1);
  digitalWrite(relay2, 1);
  digitalWrite(relay3, 1);
  
}
  
void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  getWifi();
  getTime();
  getSql();
  setRelay();

  dht.begin();
  digitalWrite(led1, ledState);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
}

void loop() {
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  //Serial.println(now);

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");

  if (conn.connected()){
    sprintf(query, INSERT_SQL, now, t, h);
    //cursor->execute(query);
  }

  int reading = digitalRead(button1);
  
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  digitalWrite(led1, ledState);
  lastButtonState = reading;

  if (digitalRead(button2) == HIGH) {
    // turn LED on:
    digitalWrite(led2, HIGH);
  } else {
    // turn LED off:
    digitalWrite(led2, LOW);
  }
  
  //delay(1000);
  int waitMin = 5;
  //delay(waitMin*60*1000);
}
