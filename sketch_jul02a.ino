// based on an orginal sketch by Arduino forum member "danigom"
// http://forum.arduino.cc/index.php?action=profile;u=188950

#include <avr/pgmspace.h>
#include <LedControl.h>
#include "font.h"

const int numDevices = 4;	// number of MAX7219s used
const long scrollDelay = 50;	// adjust scrolling speed

const int motorPin = 12;

unsigned long bufferLong[16] = { 0 };

LedControl lc = LedControl(5, 7, 6, numDevices);

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

void setup()
{
	for (int x = 0; x < numDevices; x++) {
		lc.shutdown(x, false);	//The MAX72XX is in power-saving mode on startup
		lc.setIntensity(x, 10);	// Set the brightness to default value
		lc.clearDisplay(x);	// and clear the display
	}
	pinMode(motorPin, OUTPUT);

	//Serial.begin(9600);
}

void loop()
{
  vib();
	showMessage(false);
	// scrollFont();
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
void showMessage(bool still)
{
	int fontsize = sizeof(font5x7) / 8;
	for (int counter = 0; counter < fontsize; counter++) {
		loadBufferLong(counter, still);
	}
  if (still) {
    printBufferLong();
    delay(4294967295);
  }
}

// Load character into scroll buffer
void loadBufferLong(int offset, bool still)
{
  int a;
  unsigned long c, x;
  // Center align code
  int skip = 0, skip_offset;
 for (a = 0; a < 8; a++) {
    if (pgm_read_byte_near(font5x7 + (offset * 8) + a) == 0) {
      skip++;
    } else {
      skip_offset = a;
      break;
    }
  }
 for (a = 7; a >= 0; a--) {
    if (pgm_read_byte_near(font5x7 + (offset * 8) + a) == 0)
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
    c = pgm_read_byte_near(font5x7 + (offset * 8) + a - skip + skip_offset); // Index into character table to get row data
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
	//byte count = pgm_read_byte_near(font5x7 +((ascii - 0x20) * 9) + 8);     // Index into character table for kerning data
	int count = trim_matrix(font5x7 + (offset * 8));
	//byte count = 6;
	for (a = 0; a < count; a++) {
		rotateBufferLong();
    if (!still) {
		  printBufferLong();
		  delay(scrollDelay);
    }
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
