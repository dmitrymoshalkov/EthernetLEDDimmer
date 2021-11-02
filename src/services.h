#ifndef SERVICES_H
#define SERVICES_H

//#define MQTT_MAX_PACKET_SIZE 256
#include <PubSubClient.h>
//#include <SimpleTimer.h>


EthernetClient ethClient;
PubSubClient mqttAPI(ethClient);
//SimpleTimer MQTTtimer;
//SimpleTimer SendToMQTT;

//String MQTTserver;



void checkMQTT();

/*
String httpCodeStr(int code) {
  switch(code) {
    case -1:  return "CONNECTION REFUSED";
    case -2:  return "SEND HEADER FAILED";
    case -3:  return "SEND PAYLOAD FAILED";
    case -4:  return "NOT CONNECTED";
    case -5:  return "CONNECTION LOST";
    case -6:  return "NO STREAM";
    case -7:  return "NO HTTP SERVER";
    case -8:  return "TOO LESS RAM";
    case -9:  return "ENCODING";
    case -10: return "STREAM WRITE";
    case -11: return "READ TIMEOUT";
     default: return  http.codeTranslate(code);
  }
}
*/


String DisplayAddress(IPAddress address)
{
 return String(address[0]) + "." +
        String(address[1]) + "." +
        String(address[2]) + "." +
        String(address[3]);
}


String mqttCodeStr(int code) {
  switch (code) {
    case -4: return "CONNECTION TIMEOUT";
    case -3: return "CONNECTION LOST";
    case -2: return "CONNECT FAILED";
    case -1: return "MQTT DISCONNECTED";
    case  0: return "CONNECTED";
    case  1: return "CONNECT BAD PROTOCOL";
    case  2: return "CONNECT BAD CLIENT ID";
    case  3: return "CONNECT UNAVAILABLE";
    case  4: return "CONNECT BAD CREDENTIALS";
    case  5: return "CONNECT UNAUTHORIZED";
    default: return String(code);
  }
}


void sendDataToMQTT();


void resetBoard()
{
  while (true) {
    delay(1);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {

  String msg, strtopic, topicname1;

  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  //for (unsigned int i = 0; i < sizeof(topic); i++) strtopic += (char)topic[i];

  #ifdef NDEBUG

    Serial.println(topic);
  #endif

strtopic = strcat(topic,"");

String commandTopic = MQTTtopic + "/cmd";
String brightTopic =  MQTTtopic + "/bright";
String resetTopic =  MQTTtopic + "/reset";


//topicname1 = "home/" + String(conf.param("mdns_hostname")) + "/airquality/reboot";

  if (strtopic == commandTopic)
  {
      if( msg == "ON" || msg == "1")
      {

values[0]=map(0,0,100,0,255);
values[1]=map(0,0,100,0,255);
values[2]=map(0,0,100,0,255);


isOn = true;
isOnTable = true;

for (int i = 0; i < NUM_CHANNELS-1; i++)
{
  target_values[i] = map(lastencoderValue,0,100,0,255);
}



      } else if ( msg == "OFF" || msg == "0")
      {
          isOn = false;
          isOnTable = false;
      }

  } else if (strtopic == brightTopic)
  {

              uint8_t currVal=msg.toInt();

              #ifdef NDEBUG
                Serial.print("Brightness: ");
                Serial.println(currVal);
              #endif

              if ( currVal >= 0 && currVal <=100)
              {

                for (int i = 0; i < NUM_CHANNELS-1; i++)
                {
                  target_values[i] = map(currVal,0,100,0,255);
                }
                knob.write(currVal);
                lastencoderValue = currVal;
              }
  } else if (strtopic == resetTopic)
  {
             resetBoard();
  }

sendDataToMQTT();

  #ifdef NDEBUG
    String prstr = "Msg: " + strtopic + " " + msg;
    Serial.println(prstr.c_str());
  #endif
}

void setupMQTT() {
  if (MQTTserver.length()) {
    /* Указываем сервер и порт для подключения. Сервер можно указать через WEB интерфейс, порт измените на свой. */
    //mqttServer = conf.param("mqtt_server");
    mqttAPI.setServer(MQTTserver.c_str(), 1883);

    /* Устанавливаем обработчик */
   mqttAPI.setCallback(callback);

  }
  /* end if */
}

void checkMQTT()
{
  yield();
  if (!mqttAPI.connected())
  {
      if( mqttAPI.connect(MQTTClientID.c_str(), MQTTuser.c_str(), MQTTpwd.c_str()) )
      {


          // String subTopic = MQTTtopic + "/cmd";
           // mqttAPI.subscribe(subTopic.c_str());
           // subTopic = MQTTtopic + "/bright";
           // mqttAPI.subscribe(subTopic.c_str());
           // subTopic = MQTTtopic + "/reset";
           // mqttAPI.subscribe(subTopic.c_str());

      }
      #ifdef NDEBUG
        Serial.println("Conn to MQTT. res: " + mqttCodeStr(mqttAPI.state()) );
      #endif
  }



}


bool mqttPublish(String topic, String data) {
yield();


  if (MQTTtopic.length()) topic = MQTTtopic + "/" + topic;
  return mqttAPI.publish(topic.c_str(), data.c_str(), true);
}
bool mqttPublish(String topic, float data) { return mqttPublish(topic, String(data)); }
bool mqttPublish(String topic, int32_t data) { return mqttPublish(topic, String(data)); }
bool mqttPublish(String topic, uint32_t data) { return mqttPublish(topic, String(data)); }


void sendDataToMQTT() {
  if (Ethernet.localIP() and MQTTserver.length() and mqttAPI.connected()) {
    #ifdef NDEBUG
      Serial.println(F("send to MQTT"));
    #endif

      String state = "OFF";

      if (isOn)
      {
        state = "ON";
      }

      mqttPublish("state",   state );
      mqttPublish("state/brightness",   lastencoderValue );
      mqttPublish("state/ip",   DisplayAddress(Ethernet.localIP()) );



      String sUptime = String(Day) + "d " + String(Hour) + ":" + String(Minute) + ":" + String(Second);

      mqttPublish("state/uptime",   sUptime );

      #ifdef NDEBUG
        Serial.println( "answer: " +  mqttCodeStr(mqttAPI.state()));
      #endif
  }
  
}



#endif
