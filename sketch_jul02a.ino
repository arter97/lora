#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <LoRaShield.h>
#include <Thread.h>

#define PN532_IRQ   (9) // (2)  <- changed to D9 for IRQ pin, refer to schematic
#define PN532_RESET (8) // (3)  <- changed to D8 for nReset pin, refer to schematic
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
LoRaShield LoRa(10, 11);

Thread nfcThread = Thread();

void setup(void) {
    nfcThread.onRun(nfcReader);
    nfcThread.setInterval(5);
  
    /* This PN532 I2C example is modified by DotoriKing from original Adafruit PN532 example */
    Serial.begin(115200);
    LoRa.begin(38400);
 
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
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 50);
 
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
        Serial.print("\r\nTimed out waiting for a card");
    }
}

void loop(void) {
  if (nfcThread.shouldRun()) {
    nfcThread.run();
  }

  Serial.print("1");
}

