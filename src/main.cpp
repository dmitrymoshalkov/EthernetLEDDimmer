/*****************************************************************************************************************/
// Подключение:
// G - стол
// R - первый канал подсветки раб зоны
// B - второй канал подсветки рабочей зоны
/*****************************************************************************************************************/
//#define NDEBUG

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
//#include <EthernetUdp.h>
#include <avr/wdt.h>

//#define MQTT_MAX_PACKET_SIZE 256

//#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient

#include <Encoder.h>
#include <Bounce2.h>
//#include "EEPROMAnything.h"

uint8_t mqttUnaviable = 0;


//#include <StandardCplusplus.h>

#define LED_BLUE   A1
#define LED_GREEN  A2
#define LED_RED    A3
#define ETHERNET_RESET_PIN  4

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
byte ip[] = { 10, 11, 13, 24 };
byte gateway[] = { 10, 11, 13, 1 };
byte subnet[] = { 255, 255, 255, 0 };


String MQTTserver = "192.168.100.124";
String MQTTuser = "openhabian";
String MQTTpwd = "damp19ax";
String MQTTtopic = "home/kitchen/ledstrip";
String MQTTClientID = "kitchenLEDStrip";

// tracks if the strip should be on of off
boolean isOn = false;
boolean isOnTable = false;
byte bCurrentBrightness = 100;
long lastencoderValue=0;

#define NUM_CHANNELS 3 // how many channels, RGBW=4 RGB=3...

byte target_values[3] = {100,100,50};
byte values[3] = {100,100,50};

#define KNOB_ENC_PIN_1 14    // Rotary encoder input pin 1 (A0)
#define KNOB_ENC_PIN_2 7    // Rotary encoder input pin 2
#define KNOB_BUTTON_PIN 8   // Rotary encoder button pin

#define KNOBUPDATE_TIME 500

Encoder knob(KNOB_ENC_PIN_2, KNOB_ENC_PIN_1);


#include "services.h"
#include "lightcontrol.h"

#define MQTTUPDATEINTERVAL 120000 //3760000 //940000
#define MQTTCHECKINTERVAL 15000 //840000 //940000



long lastMQTTUpdate = 0;
long lastMQTTCheckUpdate = 0;

bool bDHCPError = false;



void setup() {

//Ethernet.init(10);

  #ifdef NDEBUG
   Serial.begin(9600);
  #endif

pinMode(LED_BLUE,OUTPUT);
pinMode(LED_GREEN,OUTPUT);
pinMode(LED_RED,OUTPUT);

digitalWrite(LED_BLUE,LOW);
digitalWrite(LED_GREEN,LOW);
digitalWrite(LED_RED,LOW);

delay(1000);


     //if (Ethernet.linkStatus() == Unknown) {
       //Serial.println("Link status unknown. Link status detection is only available with W5200 and W5500.");
     //}
     //else if (Ethernet.linkStatus() == LinkON) {
       //Serial.println("Link status: On");
     //}
     //else if (Ethernet.linkStatus() == LinkOFF) {
    //   Serial.println("Link status: Off");
     //}

//uint8_t retryCount = 0;

//while ( ( Ethernet.linkStatus() != LinkON ) && retryCount < 21 )
//{
//  delay(1000);
//  retryCount++;
//}


setColorState(2);
delay(1000);
setColorState(1);
delay(1000);
setColorState(0);
delay(1000);
setColorState(3);




  //  Ethernet.begin(mac, ip);
//  IPAddress ip(192,168,1,200);




// Reset the W5500 module
  pinMode(ETHERNET_RESET_PIN, OUTPUT);
  digitalWrite(ETHERNET_RESET_PIN, HIGH);
  digitalWrite(ETHERNET_RESET_PIN, LOW);
  delay(800); //100
  digitalWrite(ETHERNET_RESET_PIN, HIGH);
  delay(800); //100

if ( Ethernet.begin(mac) == 0)
{

  //Enable watchdog timer
   wdt_enable(WDTO_8S);

  setColorState(1);
  delay(1000);
  setColorState(0);
  delay(1000);
  setColorState(3);

  resetBoard();


}

   // print your local IP address:
   //Serial.print("IP: ");
   //Serial.println(Ethernet.localIP());

   initLights();

   setupMQTT();

   setColorState(1);

     //Enable watchdog timer
     	wdt_enable(WDTO_8S);

}

void loop() {

  switch (Ethernet.maintain()) {
      case 1:
        //renewed fail
        //Serial.println("Error: renewed fail");
             setColorState(2);
             delay(1000);
             resetBoard();
        break;

      case 2:
        //renewed success
        //Serial.println("Renewed success");
        //print your local IP address:
        //Serial.print("My IP address: ");
        //Serial.println(Ethernet.localIP());
        break;

      case 3:
        //rebind fail
        //Serial.println("Error: rebind fail");
             setColorState(2);
             delay(1000);
             resetBoard();
        break;

      case 4:
        //rebind success
        //Serial.println("Rebind success");
        //print your local IP address:
        //Serial.print("My IP address: ");
        //Serial.println(Ethernet.localIP());
        break;

      default:
        //nothing happened
        break;
    }


    debouncernocontroller.update();
    byte switchState = debouncernocontroller.read();



    // and set the new light colors
    if (millis() > lastupdate + INTERVAL) {
      updateLights();
      lastupdate = millis();
    }

  checkButtonClick();

  checkRotaryEncoder();

  updateBrightness();

    /* Обработчик MQTT для режима "подписчика" */
    mqttAPI.loop();


    unsigned long currentCheckMQTTMillis = millis();

    if( (currentCheckMQTTMillis - lastMQTTCheckUpdate ) > MQTTCHECKINTERVAL )
    {
       checkMQTT();

       if (mqttAPI.connected())
       {
         setColorState(0);
         mqttUnaviable =0;

       } else if (!bDHCPError)
       {
           setColorState(3);
           mqttUnaviable++;

           if (mqttUnaviable > 21)
           {
              resetBoard();
           }
       }

         lastMQTTCheckUpdate = currentCheckMQTTMillis;
    }


    unsigned long currentMQTTMillis = millis();

       if( (currentMQTTMillis - lastMQTTUpdate ) > MQTTUPDATEINTERVAL )
       {
            sendDataToMQTT();


         lastMQTTUpdate = currentMQTTMillis;
       }

    //MQTTtimer.run();

    //reset watchdog timer
    wdt_reset();

}
