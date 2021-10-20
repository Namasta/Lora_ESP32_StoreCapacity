/*
  This is a simple example show the Heltec.LoRa sended data in OLED.

  The onboard OLED display is SSD1306 driver and I2C interface. In order to make the
  OLED correctly operation, you should output a high-low-high(1-0-1) signal by soft-
  ware to OLED's reset pin, the low-level signal at least 5ms.

  OLED pins to ESP32 GPIOs via this connecthin:
  OLED_SDA -- GPIO4
  OLED_SCL -- GPIO15
  OLED_RST -- GPIO16
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  https://heltec.org
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/

#include "heltec.h" 
#include "images.h"
#include "Arduino.h"
#include "WiFi.h"


#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6

//unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
const int ProxSensor=36;//data pin from IR Proximity Sensor
int exitCust = 0;//counter for exit customer

void logo()
{
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

void setup()
{
  //Exit proximity sensor (flying fish sensor with 3 pins) pins assigned for detection of customer exit
  //Pin 36 is connected to the output of proximity sensor
  pinMode(ProxSensor,INPUT);
  
  //Serial print
  Serial.begin(115200);
   
  //Initialise Heltec LoRA WiFi Board
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);//only  font sizes of 10,16,24
  logo();
  delay(1500);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->display();
  delay(1000);
}

void loop()
{
  //display EXIT ONLY when no customer exiting
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0, 25, String("EXIT ONLY"));
  Heltec.display->display();

  //Exit Sensor at work
  if(digitalRead(ProxSensor)==LOW)
  {
    exitCust++;    // Set exitCust count
    //bidding goodbye to exiting customer
    Serial.print("exitCust");
    Serial.println(exitCust);
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(0, 25, String("SEE YOU SOON!"));
    Heltec.display->display();
    delay(2000);

    // send packet
  LoRa.beginPacket();//initialise a send packet
  
/*
 * LoRa.setTxPower(txPower,RFOUT_pin);
 * txPower -- 0 ~ 20
 * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
 *   - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
 *   - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
*/
  //send exit customer update to entrance LoRa.
  LoRa.setTxPower(0,RF_PACONFIG_PASELECT_PABOOST);
  //LoRa.print("hello ");//LoRa.print is writing data into packet
  LoRa.print(exitCust);//write count exitCust into packet
  LoRa.endPacket();//end sending
  }
  else
  {
    delay(100); 
  }
}
