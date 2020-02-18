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
int ledState1 = LOW;
int ledState2 = LOW;
int loadLed = HIGH;
int buttonState1;             
int lastButtonState1 = LOW;
unsigned long lastDebounceTime1 = 0;
int buttonState2;             
int lastButtonState2 = LOW;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay = 50;
unsigned long previousMillis = 0;
const long interval = 10000;

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
  digitalWrite(led1, ledState1);
  digitalWrite(led1, ledState2);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
}

void loop() {
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  //Serial.println(now);

  float t = dht.readTemperature() - 5.4;  //temp tuning
  float h = dht.readHumidity();

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");

  if(t > 25){
    digitalWrite(thermostat, HIGH);
    }
  if(t < 24){
    digitalWrite(thermostat, LOW);
    }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (conn.connected()){
      sprintf(query, INSERT_SQL, now, t, h);
      cursor->execute(query);
    }
  }
  
  //switch No.2
  int reading1 = digitalRead(button1);
  
  if (reading1 != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
  }

  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (reading1 != buttonState1) {
      buttonState1 = reading1;
      if (buttonState1 == HIGH) {
        ledState1 = !ledState1;
      }
    }
  }

  digitalWrite(led1, ledState1);
  digitalWrite(relay1, !ledState1);
  lastButtonState1 = reading1;

  //switch No.2
  int reading2 = digitalRead(button2);
  
  if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }

  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      if (buttonState2 == HIGH) {
        ledState2 = !ledState2;
      }
    }
  }

  digitalWrite(led2, ledState2);
  digitalWrite(relay2, !ledState2);
  lastButtonState2 = reading2;
 
}
