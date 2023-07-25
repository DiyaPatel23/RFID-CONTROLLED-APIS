#include <HTTPClient.h>

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
WiFiMulti wifiMulti;
int lightPin = 5;
int lightVal;
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 21
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte nuidPICC[4] = {0xC2, 0x60, 0xE0, 0x1B};


char ssid[] = "SmS_jiofi";
char pass[] = "sms123458956";

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}



void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiMulti.addAP("SmS_jiofi", "sms123458956");



  Serial.begin(9600);
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW);

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);


}

void loop() {
  // wait for WiFi connection

  if ( ! rfid.PICC_IsNewCardPresent())
    return;


  if ( ! rfid.PICC_ReadCardSerial())
    return;
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));


  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

 
 if (rfid.uid.uidByte[0] == nuidPICC[0] ||
      rfid.uid.uidByte[1] == nuidPICC[1] ||
      rfid.uid.uidByte[2] == nuidPICC[2] ||
      rfid.uid.uidByte[3] == nuidPICC[3] )
     { if((wifiMulti.run() == WL_CONNECTED)) 



  {
    Serial.println(F("ID RECOGNISED"));

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    http.begin("http://blr1.blynk.cloud/external/api/update?token=Va2qYYSVmbHqAUQX4GGoRkiv78CB4Jya&v0=1"); //HTTP

    Serial.print("[HTTP] GET...\n");
    Serial.println("http://blr1.blynk.cloud/external/api/update?token=Va2qYYSVmbHqAUQX4GGoRkiv78CB4Jya&v0=1");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
      }

      Serial.println(F("The NUID tag is:"));
      Serial.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      Serial.println();

    }
  } }

  else {
    Serial.println(F("DO NOT MATCH."));
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://blr1.blynk.cloud/external/api/update?token=TxZ2DEkltF94K8mEeoXusBLJf321V_j7&d2=1"); //HTTPS
    http.begin("http://blr1.blynk.cloud/external/api/update?token=Va2qYYSVmbHqAUQX4GGoRkiv78CB4Jya&v0=0"); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  

  http.end();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
}
