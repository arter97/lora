#include <LoRaShield.h>
#include <stdio.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   (9) // (2)  <- changed to D9 for IRQ pin, refer to schematic
#define PN532_RESET (8) // (3)  <- changed to D8 for nReset pin, refer to schematic
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
 
int BUTTON = 13;
int state = 0;
String s, m, m2, t, t2;
LoRaShield LoRa(10,11);
 
void nfcReader(void);

void setup() {
  Serial.begin(115200);
  LoRa.begin(38400);
  pinMode(BUTTON, INPUT);

      nfc.begin();
 
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata)
    {
        Serial.print("\r\nDidn't find PN53x board");
        while (1); // halt
    }
 
    // Got ok data, print it out!
    Serial.print("\r\nFound chip PN5");
    Serial.print((versiondata>>24) & 0xFF, HEX);
    Serial.print("\r\nFirmware ver. ");
    Serial.print((versiondata>>16) & 0xFF, DEC);
    Serial.print('.');
    Serial.print((versiondata>>8) & 0xFF, DEC);
    
    nfc.setPassiveActivationRetries(0xFF);
 
    nfc.SAMConfig();
 
    Serial.print("\r\nWaiting for an ISO14443A card");
}
 
void loop() {
  while (digitalRead(BUTTON) == LOW)
  {
    LoRa.sendMessage("7777");
    //Serial.println("Sended");
    
    while(LoRa.available())
    {
      s = LoRa.ReadLine();
      //Serial.print(s);
      m = LoRa.GetMessage();
      if(m != "")
      {
        m2=m;
        Serial.println(m);
        t=m2.substring(0,1);
        if(t=="0")
        {
          Serial.print("spend : ");
          t2 = m.substring(1,7);
          Serial.print(t2);
        }
        else
        {
          Serial.print("waiting time : ");
          t2 = m.substring(1,5);
          Serial.print(t2);
        }
      }
    }
  }

  nfcReader();
}

void nfcReader(void)
{
    boolean success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t uidLength;
    char arr[2];
    char temparr[18]={0};
    temparr[17]='\0';
    temparr[0]='0';
    temparr[1]='1';
    temparr[2]='1';
    temparr[3]='3';
    temparr[4]='4';
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 500);
 
    if (success)
    {
        Serial.print("\r\nFound a card!");
        Serial.print("\r\nUID Length: ");
        Serial.print(uidLength, DEC);
        Serial.print(" bytes");
        Serial.print("\r\nUID Value: ");
        int a=5;
        Serial.print("0x4");
 
        for (uint8_t i = 1; i < uidLength; i++)
        {
            Serial.print(" 0x");
            Serial.print(uid[i], HEX);
            itoa(uid[i],arr,16);
            temparr[a]=arr[0];
            temparr[a+1]=arr[1];
            a=a+2;
        }
        Serial.print("\n");
        Serial.print(temparr);
        Serial.print("\n");
        LoRa.SendMessage(temparr,HEX);
        Serial.print("sended in hex\n");
 
        // Wait 1 second before continuing
        delay(1000);
    }
    else
    {
        // PN532 probably timed out waiting for a card
        //Serial.print("\r\nTimed out waiting for a card");
    }
}
