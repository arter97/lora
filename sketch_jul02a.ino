// based on an orginal sketch by Arduino forum member "danigom"
// http://forum.arduino.cc/index.php?action=profile;u=188950

//#define SERIAL_BUFFER_SIZE 2048

#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <avr/pgmspace.h>
#include <LedControl.h>
#include "font.h"
#include <LoRaShield.h>
#include <Time.h>

int BUTTON = 13;
int state = 0;
String s, m, m2, m3, t, t2, t3;

//#define SERIAL_BUFFER_SIZE 2048

LoRaShield LoRa(10, 11);

const int numDevices = 4;	// number of MAX7219s used
const long scrollDelay = 30;	// adjust scrolling speed

const int motorPin = 12;

unsigned long bufferLong[16] = { 0 };

LedControl lc = LedControl(5, 7, 6, numDevices);

#define PN532_IRQ   (9) // (2)  <- changed to D9 for IRQ pin, refer to schematic
#define PN532_RESET (8) // (3)  <- changed to D8 for nReset pin, refer to schematic
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void nfcReader(void);

void vib()
{
	digitalWrite(motorPin, HIGH);
	delay(250);
	digitalWrite(motorPin, LOW);
	delay(100);
	digitalWrite(motorPin, HIGH);
	delay(250);
	digitalWrite(motorPin, LOW);
	//delay(200);
}

void setLPM(bool val) {
  for (int x = 0; x < numDevices; x++) {
    lc.shutdown(x, val); //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(x, 10); // Set the brightness to default value
    lc.clearDisplay(x); // and clear the display
  }
}

void setBrightness(int val) {
  for (int x = 0; x < numDevices; x++) {
    lc.shutdown(x, false); //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(x, val); // Set the brightness to default value
    lc.clearDisplay(x); // and clear the display
  }
}

void setup()
{
  setLPM(true);
	pinMode(motorPin, OUTPUT);

	Serial.begin(115200);
    //LoRa.setTimeout(500);
  LoRa.begin(38400);

//  setTime(22,22,0,0,0,0);
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

void showMessage(bool still, const unsigned char* PROGMEM txt, int size);
void clearScreen(void);
void nfcReader(void);
void showTime(void);

int powint(int factor, unsigned int exponent)
{
   int product = 1;
   while (exponent--)
      product *= factor;
   return product;
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
        t=m2.substring(0,4);
        if(t=="1000")
        {  
          int index, i;
          for(i=4; i<12; i++)
          {
            t2=m2.substring(i,i+1);
            if(t2!="0")
            {
              index = i;
            }
          }          
          Serial.print("spend : ");
          t3 = m.substring(index,12);
          Serial.print(t3);
          Serial.print("원");
          Serial.println();


          setLPM(false);
          vib();

   memset(bufferLong, 0, sizeof(bufferLong));

int to_min = t3.toInt();
for (int i = log10(to_min); i >= 0; i--) {
  loadBufferLong((to_min / pow(10, i)), false, time_txt);
  to_min = to_min % powint(10, i);
}

   loadBufferLong(13, false, time_txt);

  for (int i = 0; i < 32; i++) {
   rotateBufferLong();
   delay(scrollDelay);
   printBufferLong();
  }
          

          // Clear screen
          memset(bufferLong, 0, sizeof(bufferLong));
          setLPM(true);



        }
        else if(t=="1001")
        {
          int index, i;
          for(i=4; i<12; i++)
          {
            t2=m2.substring(i,i+1);
            if(t2!="0")
            {
              index = i;
            }
          }          
          Serial.print("waiting time : ");
          t3 = m.substring(index,12);
          Serial.print(t3);


          setLPM(false);
          vib();

   memset(bufferLong, 0, sizeof(bufferLong));
int to_min = t3.toInt();
for (int i = log10(to_min); i >= 0; i--) {
  loadBufferLong((to_min / pow(10, i)), true, time_txt);
  to_min = to_min % powint(10, i);
}

loadBufferLong(12, true, time_txt); // Min

   for (int i = 0; i < 9; i++)
   rotateBufferLong(); // Move by 9 pixel

   printBufferLong();


          
          delay(10000);

          // Clear screen
          memset(bufferLong, 0, sizeof(bufferLong));
          setLPM(true);

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

    Serial.flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int trim_matrix(const unsigned char *font)
{
	int i;
	int j;
	const unsigned char product = 1;
	int ret = 0;
	unsigned char temp;

	for (i = 0; i < 8; i++) {
		temp = pgm_read_byte_near((font + i));
		for (j = 0; j < 8; j++) {
			if ((product & temp) == 1) {
				if (ret < (8 - j)) {	// 8-j == length
					ret = 8 - j;
				}
				break;
			}
			temp = (temp >> 1);
		}
	}

	if (ret == 0)
		return 3;	// Space

	return ret + 1;
}

// Show Message
void showMessage(bool still, const unsigned char* PROGMEM txt, int size)
{
  memset(bufferLong, 0, sizeof(bufferLong));
  
	for (int counter = 0; counter < size; counter++) {
		loadBufferLong(counter, still, txt);
	}
  if (still) {
    printBufferLong();
    //delay(4294967295); // Unsigned Long MAX
  }
}

// Load character into scroll buffer
void loadBufferLong(int offset, bool still, const unsigned char* PROGMEM txt)
{
  int a;
  unsigned long c, x;
  // Center align code
  int skip = 0, skip_offset = 0;
 for (a = 0; a < 8; a++) {
    if (pgm_read_byte_near(txt + (offset * 8) + a) == 0) {
      skip++;
    } else {
      skip_offset = a;
      break;
    }
  }
 for (a = 7; a >= 0; a--) {
    if (pgm_read_byte_near(txt + (offset * 8) + a) == 0)
      skip++;
    else
      break;
  }
  skip /= 2;
  for (a = 0; a < skip; a++) {
    c = 0;
    x = bufferLong[a * 2];  // Load current scroll buffer
    x = x | c;  // OR the new character onto end of current
    bufferLong[a * 2] = x;  // Store in buffer
  }
 for (a = skip; a < 8 - skip_offset + 1; a++) { // Loop 7 times for a 5x7 font
    c = pgm_read_byte_near(txt + (offset * 8) + a - skip + skip_offset); // Index into character table to get row data
    x = bufferLong[a * 2];  // Load current scroll buffer
    x = x | c;  // OR the new character onto end of current
    bufferLong[a * 2] = x;  // Store in buffer
  }
  for (a = 8 - skip_offset + 1; a < 8; a++) {
    c = 0;
    x = bufferLong[a * 2];  // Load current scroll buffer
    x = x | c;  // OR the new character onto end of current
    bufferLong[a * 2] = x;  // Store in buffer
  }
  //byte count = pgm_read_byte_near(txt +((ascii - 0x20) * 9) + 8);     // Index into character table for kerning data
  int count = trim_matrix(txt + (offset * 8));
  //byte count = 6;
  for (a = 0; a < count; a++) {
    rotateBufferLong();
    if (!still) {
      printBufferLong();
      delay(scrollDelay);
    }
  }
}

void clearScreen(void)
{
    int a;
    unsigned long c, x;
   for (a = 0; a < 8; a++) { // Loop 7 times for a 5x7 font
      c = 0;
      x = bufferLong[a * 2];  // Load current scroll buffer
      x = x | c;  // OR the new character onto end of current
      bufferLong[a * 2] = x;  // Store in buffer
    }
    int count = 32;
    for (a = 0; a < count; a++) {
      rotateBufferLong();
      printBufferLong();
      delay(scrollDelay);
    }
}

// Rotate the buffer
void rotateBufferLong()
{
	for (int a = 0; a < 8; a++) {	// Loop 7 times for a 5x7 font
		unsigned long x = bufferLong[a * 2];	// Get low buffer entry
		byte b = bitRead(x, 31);	// Copy high order bit that gets lost in rotation
		x = x << 1;	// Rotate left one bit
		bufferLong[a * 2] = x;	// Store new low buffer
		x = bufferLong[a * 2 + 1];	// Get high buffer entry
		x = x << 1;	// Rotate left one bit
		bitWrite(x, 0, b);	// Store saved bit
		bufferLong[a * 2 + 1] = x;	// Store new high buffer
	}
}

// Display Buffer on LED matrix
void printBufferLong()
{
	for (int a = 0; a < 8; a++) {	// Loop 7 times for a 5x7 font
		unsigned long x = bufferLong[a * 2 + 1];	// Get high buffer entry
		byte y = x;	// Mask off first character
		lc.setRow(3, a, y);	// Send row to relevent MAX7219 chip
		x = bufferLong[a * 2];	// Get low buffer entry
		y = (x >> 24);	// Mask off second character
		lc.setRow(2, a, y);	// Send row to relevent MAX7219 chip
		y = (x >> 16);	// Mask off third character
		lc.setRow(1, a, y);	// Send row to relevent MAX7219 chip
		y = (x >> 8);	// Mask off forth character
		lc.setRow(0, a, y);	// Send row to relevent MAX7219 chip
	}
}
