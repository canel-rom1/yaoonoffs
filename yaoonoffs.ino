/*
Project: YAOOnOffS - Yet An Other on/off Switch (Sonoff)
File: yaoonoffs.ino
Version: 0.1
Create by: Rom1 <rom1@canel.ch>
		   CANEL - https://www.canel.ch
Date: 28/02/18
License: GNU GENERAL PUBLIC LICENSE v3
Language: Arduino (C/C++)
Description: Un autre firmware pour les cartes ESP8266 pour piloter un relais. Compatible Sonoff
*/

//#define DEBUG
#define HTTP
//#define USE_HTTPS         /* Utilise le protocole HTTPS
//#define MQTT              /* Activer le serveur MQTT. Attention, il ne passe par une couche TLS */
#define SWITCH

#include <ESP8266WiFi.h>

#ifndef USE_HTTPS
  #include <ESP8266WebServer.h>
#else
  #include <ESP8266WebServerSecure.h>
  #include "cert-ssl.h"
#endif//USE_HTTPS

#ifdef MQTT
  #include <PubSubClient.h>
#endif//MQTT

#ifdef DEBUG
  #include <GDBStub.h>
#endif//DEBUG

/* Fichier de configuration avec les variables à personnaliser */
#include "config.h"


/* Définir les librairies */
WiFiClient espClient;

#ifdef HTTP
#ifndef USE_HTTPS
ESP8266WebServer serverHTTP(80);
#else
ESP8266WebServerSecure serverHTTP(443);
#endif//USE_HTTPS
#endif//HTTP

#ifdef MQTT
PubSubClient clientMQTT(serverMQTT, portMQTT, espClient);
#endif//MQTT

/* Variables globales */
int state_relay1 = 0;
int lastDebounceTime = 5;
int debounceDelay = 500;

/*************/
/* Functions */
/*************/

/* Headers */
void handleSubmit(void);
String homePage(void);
void offRelay(void);
void onRelay(void);
void rcvMQTT(char *topic, byte *_payload, unsigned int len);
void setRelay(int stat);
void swapRelay(void);

#ifdef HTTP
String homePage(void)
{
  String page;

  page  = "<!DOCTYPE HTML>\r\n";
  page += "<html lang=fr-CH><head>";
  page +=   "<meta charset='utf-8' />";
  page +=   "<title>" + String(hostString) + "</title>";
  page += "</header><body>";
  page +=   "<h1>" + String(hostString) + "</h1>";
  page +=     "State: " + String(state_relay1);
  page +=     "<form action='/' method='POST'><table>";
  page +=       "<tr><input type='radio' name='state' value='1'> ON</tr>";
  page +=       "<tr><input type='radio' name='state' value='0' > OFF</tr>";
  page +=     "</table><input type='submit' value='OK'>";
  page += "</body></html>";

  return page;
}

void handleSubmit(void)
{
  if(serverHTTP.arg("state").toInt())
    onRelay();
  else
    offRelay();
  serverHTTP.send(200, "text/html", homePage());
}

void handleRoot(void)
{
  if(serverHTTP.hasArg("state"))
    handleSubmit();
  else
    serverHTTP.send(200, "text/html", homePage());
}
#endif//HTTP

#ifdef MQTT
void rcvMQTT(char *topic, byte *_payload, unsigned int len)
{
  String payload;
  for(int i=0 ; i<len ; i++)
    payload += (char)_payload[i];

  Serial.println(payload);
  if(payload == "on")
  {
    onRelay();
  }
}
#endif//MQTT


/* Enclanche le relais */
void onRelay(void)
{
  digitalWrite(relay1, HIGH);
  Serial.println("Relay On");
#ifdef MQTT
  clientMQTT.publish("yaoonoffs/relay/stat", "1");
#endif//MQTT
  state_relay1 = 1;
}

/* Déclanche le relais */
void offRelay(void)
{
  digitalWrite(relay1, LOW);
  Serial.println("Relay Off");
#ifdef MQTT
  clientMQTT.publish("yaoonoffs/relay/stat", "0");
#endif//MQTT
  state_relay1 = 0;
}

/* Met la valeur donnée au relais */
void setRelay(int stat)
{
  if(stat = 1)
    onRelay();
  else
    offRelay();
}

/* Inverse l'état du relais */
void swapRelay(void)
{
  if(state_relay1)
    offRelay();
  else
    onRelay();
}

#ifdef SWITCH
void buttonISR(void)
{
  if( (millis()-lastDebounceTime) > debounceDelay )
  {
    swapRelay();
    lastDebounceTime = millis();
  }
}
#endif//SWITCH

void setup(void)
{
//  delay(5000);
  pinMode(relay1, OUTPUT);
#ifdef SWITCH
  pinMode(button1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1), buttonISR, RISING);
#endif//SWITCH
  

  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to... ");
  Serial.print(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");

#ifdef USE_HTTPS
  serverHTTP.setServerKeyAndCert_P(rsakey, sizeof(rsakey), x509, sizeof(x509));
#endif//USE_HTTPS
#ifdef HTTP
  serverHTTP.on("/", handleRoot);

  serverHTTP.begin();
  Serial.print("Server HTTP started at ");
  Serial.println(WiFi.localIP());
#endif//HTTP
#ifdef MQTT
  while(!clientMQTT.connected())
  {
    Serial.print("Connecting to MQTT server...");
    if(clientMQTT.connect("arduinoClient", userMQTT, passwdMQTT))
    
      Serial.println("OK");
      clientMQTT.setCallback(rcvMQTT);
      clientMQTT.subscribe("yaoonoffs/relay/cmd");
    }
    else
    {
      Serial.print("Error:");
      Serial.println(clientMQTT.state());
      delay(1000);
    } 
  }
#endif//MQTT
}

void loop(void)
{
#ifdef HTTP
  serverHTTP.handleClient();
#endif//HTTP

#ifdef MQTT
  if(clientMQTT.connected())
  {
    clientMQTT.loop();
  }
  else
    Serial.println("mqtt not connected");
#endif//MQTT
}


// vim: ft=arduino tw=100 et ts=2 sw=2
