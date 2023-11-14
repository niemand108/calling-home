
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK "password"
#endif


IPAddress local_IP(192, 168, 0, 177);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress remote_IP(192, 168, 0, 178);

const unsigned int remote_port = 58870;
unsigned int localPort = 8888; 

char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];  
char ReplyBuffer[50] = "CALLING\r\n\0"; 
WiFiUDP Udp;
const int intPin = 12;
const int intMutePin = 4;
const int pinWifi = 2;
unsigned long counter = 0;
volatile unsigned long lastInterrupt = 0;
volatile bool send_now = false;
volatile unsigned long lastCall = 0;
unsigned long lastUpdate = 0;
const unsigned int TIME_UPDATE_STATUS = 1000*5*60;
const unsigned int TIME_BW_CALLING= 1000*45 ;
const unsigned int TIME_SIGNALING_WIFI= 1000*2;
const unsigned int TIME_BT_PACKET = 1000*2;
const unsigned int BURST_TOTAL = 3;
volatile unsigned long now = 0;
volatile bool mute = false;
volatile unsigned long mute_time = 0;
const unsigned int TIME_MUTING = 3*1000;

unsigned int BURST_COUNT = 0;
bool wifi_restart = false;
String reply_str ="ack";

void setup() {
  Serial.begin(115200);

  Serial.println("ESP8266");

  pinMode(intPin, INPUT);//INPUT_PULLUP);
  pinMode(pinWifi, OUTPUT);
  digitalWrite(pinWifi, LOW);

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);
  attachInterrupt(digitalPinToInterrupt(intPin), callIntercepted, CHANGE);
  attachInterrupt(digitalPinToInterrupt(intMutePin), MuteIntercepted, CHANGE);

}

void loop() {
  now = millis();

  //overflow millis thing, *now* never is greater except when there is concurrency (almost impossible here)
  if(now < lastCall || now < lastUpdate || now < lastInterrupt){
    lastCall = now;
    lastUpdate = now;
    lastInterrupt = now;
    Serial.println("Resetting for avoid overflow millis()");
    reply_str = String("UPDATE, ")+String(now)+String(", warning: Resetting for avoid overflow millis()\r\n");
    send_message_update(reply_str);
  }

  if (mute){
    if(send_now){
      digitalWrite(pinWifi, LOW);
      send_now = false;
    }

    Serial.println("Muted");
    if ( (mute_time+TIME_MUTING) < now){
      mute = false;
    }
  }
  else if (send_now){
    digitalWrite(pinWifi, HIGH);
    if (lastCall+(BURST_COUNT*TIME_BT_PACKET) < now || lastInterrupt == lastCall){
      reply_str = String("CALLING, ")+String(lastCall)+String("\r\n");
      send_message_update(reply_str);
      lastCall = now;
      if (BURST_TOTAL <= BURST_COUNT){
        BURST_COUNT = 0;
        send_now = false;
        digitalWrite(pinWifi, LOW);
      } else {
        BURST_COUNT = BURST_COUNT + 1;
      }
    }
  } else {
      if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting wifi");
      WiFi.mode(WIFI_STA);
      WiFi.config(local_IP, gateway, subnet); 
      delay(10);
      WiFi.begin(STASSID, STAPSK);
      for ( int i = 0; i < 300; i++) {   
        if (WiFi.status() == WL_CONNECTED) {
          reply_str = String("WIFIOFF, ")+String(now)+String("\r\n");
          send_message_update(reply_str);
          break;}
        delay(100);
      }
    }
    
    if (WiFi.status() != WL_CONNECTED) {
      delay(5000);  
      ESP.restart();
      wifi_restart = true;
    }
  }

  if (!send_now && (lastUpdate == 0 || (lastUpdate+TIME_UPDATE_STATUS) < now || wifi_restart))
  {
    Serial.println("now:" + String(now));
    Serial.println("lastUpdate:" + String(lastUpdate));
    reply_str = String("UPDATE, ")+String(now)+String("\r\n");
    if (wifi_restart){
      wifi_restart = false;
      reply_str = reply_str + String(", wifi_restart");
    }

    send_message_update(reply_str);

    Serial.println("LAST_UPDATE NOW");

    lastUpdate = now;

  }
  delay(750);
}

void send_message_update(String reply){
    Udp.beginPacket(remote_IP, remote_port);
    reply.toCharArray(ReplyBuffer,reply_str.length() + 1);
    Udp.write(ReplyBuffer);
    Udp.endPacket();
    Serial.println("PACKET SEND");
}


//RAM_ATTR
ICACHE_RAM_ATTR void callIntercepted() {
  //cli();
  if (lastInterrupt == 0 || (now-lastInterrupt) >=TIME_BW_CALLING){
    lastCall = now;
    lastInterrupt = now;
    send_now = true;
  }
  //sei();
}

ICACHE_RAM_ATTR void MuteIntercepted() {
  //cli();
  if (!mute){
    mute_time = now;
    mute = true;
  }
  //sei();
}