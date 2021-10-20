
#include "heltec.h" 
#include "images.h"
#include "Arduino.h"
#include "WiFi.h"
#include "Firebase_ESP_Client.h"

#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6

#define FIREBASE_FCM_SERVER_KEY "AAAAh8ZWaNo:APA91bEo7YX222Vjv72K-WwWCMJ9r0K46FiEltaSGjWZVkipYJEzXBQqmppryTFFlp_wlZB21JCbFyb4fZheiRBFNzJg80gUhz90MlXaEZxdBsm_Gj6xW6UC2k0cPI77mA-RkCLx2gjE"
#define DEVICE_REGISTRATION_ID_TOKEN "cGWIzL1SQeeAzVNqv5urFO:APA91bGnPk0H6KdEzC6K7s8WBVucxokdnmEPUtzbnbcLEakWw69C1oNDV32AYWUtS6tpSWMJkSE2oEz9EIiKC7k1P-0s6T5Bbg2exrhOzR9Ltz6RgpXEPtowjkDF53MTDI-a3Y8sX3_L"
const char* ssid= "chyesoon";    //WiFi SSID
const char* password= "whyqueue";  //WiFi Password

String rssi = "RSSI --";//what is the purpose?
String packSize = "--";//what is the purpose?
String packet ;//what is the purpose?
const int ProxSensor=36;//data pin from IR Proximity Sensor
int netCust = 48;

//Define Firebase Data object
FirebaseData fbdo;
unsigned long lastTime = 0;
int count = 0;
void sendMessage();

void logo(){
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

void LoRaData(){
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  //Serial.print(rssi);
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, rssi);
  //Heltec.display->drawString(0 , 15 , "Received "+ packSize + " bytes");
  //Heltec.display->drawStringMaxWidth(0 , 26 , 128, packet);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0, 25, "ENTRANCE");  
  Heltec.display->display();
}

void setup() { 
  
  //Pin 36 is connected to the output of proximity sensor
  pinMode(ProxSensor,INPUT);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 23, 17);//initialise pin 17 as Tx pin. Pin 23 is Rx but need not use
   
   //Initialise Heltec LoRA WiFi Board
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->drawString(0, 10, "Wait for incoming data...");
  Heltec.display->display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();

  //Wifi codes
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    //required for legacy HTTP API
    Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);
    Firebase.reconnectWiFi(true);
    //sendMessage();

}

void loop() {
  
  //Serial.print(rssi);
  LoRaData();
  //check for incoming exit customer count from Exit Lora esp32
  int packetSize = LoRa.parsePacket();//check for incoming packet

  //minus net customer count 
  if (packetSize){
      if (LoRa.available()){
      netCust--;//minus net customer count
      Serial.print("Exit: ");// to remove later
      Serial.print(netCust);
      Serial1.println(netCust);//sending net customer count to UNO_entry via soft serial
      Serial.println();
      sendMessage();//sending net customer cout to Firebase cloud
    }
  else{
    delay(10);
   }
  }

  //Entry Detection Sensor
  if(digitalRead(ProxSensor)==LOW)      //Check the sensor output
  {
    netCust++;//add count to net customer count
    Serial.print("Entrance: ");
    Serial.print(netCust);
    Serial.println();
    Serial1.println(netCust);//update net customer count to UNO_entry via soft serial
    sendMessage();//sending net customer count to Firebase cloud
    //delay(2000);
    

    //add WELCOME to OLED
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_24);
    Heltec.display->drawString(0, 25, String("WELCOME!"));
    Heltec.display->display();
    delay(4000); 
  }
  else{
    delay(10); 
  }

}

/*    if (millis() - lastTime > 60 * 1000 || lastTime == 0)
    {
        lastTime = millis();
        sendMessage();
    }*/

void sendMessage()
{
    Serial.print("Send Firebase Cloud Messaging... ");
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(0, 0, String("Sending Firebase "));
    Heltec.display->drawString(0, 15, String("Cloud Messaging..."));
    Heltec.display->display();
    delay(100);
    //Read more details about legacy HTTP API here https://firebase.google.com/docs/cloud-messaging/http-server-ref
    FCM_Legacy_HTTP_Message msg;
    msg.targets.to = DEVICE_REGISTRATION_ID_TOKEN;
    msg.options.time_to_live = "1000";
    msg.options.priority = "high";
    msg.payloads.notification.title = "From Arduino test 1";
    msg.payloads.notification.body = "test";
//    msg.payloads.notification.icon = "myicon";
    msg.payloads.notification.click_action = "AndroidPushNew";

    FirebaseJson json;
/*    String payload;
     //all data key-values should be string
    json.add("temp", "28");
    json.add("unit", "celsius");
    json.add("timestamp", "1609815454");
    json.toString(payload);
    msg.payloads.data = payload.c_str();*/

     if (Firebase.FCM.send(&fbdo, &msg)) //send message to recipient
        Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    else
        Serial.println(fbdo.errorReason());
    count++;
}
