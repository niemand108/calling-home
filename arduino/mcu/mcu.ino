const int intPin = 2; 
const int intPinTest = 3; 
const int pinWifi = 4;
unsigned long counter = 0;
volatile unsigned long lastInterrupt = 0;
volatile bool signaled_wifi = false;
unsigned long lastUpdate = 0;
unsigned long lastCall = 0;
const unsigned int MIN_TIME_INTERRUPT = 450;
const unsigned int MAX_TIME_INTERRUPT = 3000;
const unsigned int TIME_UPDATE_STATUS = 1000*60;
const unsigned int TIME_BW_CALLING= 1000*20;
const unsigned int TIME_SIGNALING_WIFI= 1000*3;
volatile unsigned long now = 0;
volatile unsigned long time_new_interrupt = 0;
volatile unsigned long now_interrupt = 0;
volatile unsigned long count_consecutive_int = 0;

void setup() {
unsigned long lastCall = 0;
  Serial.begin(115200);
  Serial.println("MCU ON");
  pinMode(intPin, INPUT);
  pinMode(intPinTest, INPUT);
  pinMode(pinWifi, OUTPUT);
  digitalWrite(pinWifi, LOW);
  attachInterrupt(digitalPinToInterrupt(intPin), callIntercepted, CHANGE);
  attachInterrupt(digitalPinToInterrupt(intPinTest), callIntercepted, CHANGE);
}

void loop() {

  now = millis();
  if(now < lastCall || now < lastInterrupt){
    lastCall = now;
    lastInterrupt = now;
    Serial.println("Resetting to avoid overflow millis()");
  }

  if (lastCall+TIME_SIGNALING_WIFI<now && signaled_wifi){
    digitalWrite(pinWifi, LOW);
    signaled_wifi = false;
    Serial.println("Wifi LOW");
  }

  //Serial.println(String(now));
  delay(500);
}

void callIntercepted() {
  now_interrupt = millis();

  cli();
 
  if (!signaled_wifi && (lastInterrupt == 0 || (now_interrupt-lastInterrupt) >=TIME_BW_CALLING)){
    unsigned int time_bw_consecutive = now_interrupt-time_new_interrupt;
    if (time_new_interrupt==0 || time_bw_consecutive > MAX_TIME_INTERRUPT){
      time_new_interrupt = now_interrupt;
      //Serial.println("INT NOT YET:time_bw_consecutive="+String(time_bw_consecutive)+", nos_int="+String(now_interrupt)+", time_new_interrupt="+String(time_new_interrupt));

    } else if (time_bw_consecutive > MIN_TIME_INTERRUPT ){//here when second/thrid consecutuve interruption happens
        //Serial.println("INFO: time_bw_consecutive="+String(time_bw_consecutive)+", nos_int="+String(now_interrupt)+", time_new_interrupt="+String(time_new_interrupt));
        lastInterrupt = now;
        lastCall = lastInterrupt;
        digitalWrite(pinWifi, HIGH);
        signaled_wifi = true;
        Serial.println("INT + Wifi HIGH: "+String(now_interrupt));
    } else {
        //Serial.println("IGNORING: time_bw_consecutive="+String(time_bw_consecutive)+",  nos_int="+String(now_interrupt)+", time_new_interrupt="+String(time_new_interrupt));
    }
  }
  sei();
}