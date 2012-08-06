/*
 * A 'fork' of the adalight code, with a nice case statement.
 *
 * Tested with the LPD6803 50 led string
 * should work with the WS2801 as well
 */
#include "FastSPI_LED.h"

#define NUM_LEDS 50
/*
 * pin 11 -> blue  for the LPD6803 'yellow' for the WS2801 string
 * pin 13 -> green for the LPD6803 'green' for the WS2801 string
 */


#define PIN 4

#define MODE_HDRA	0
#define MODE_HDRd	1
#define MODE_HDRa	2
#define MODE_HDRhi	3
#define MODE_HDRlo	4
#define MODE_HDRchk	5
#define MODE_DATAr	6
#define MODE_DATAg	7
#define MODE_DATAb	8

// Sometimes chipsets wire in a backwards sort of way
struct CRGB { unsigned char r; unsigned char b; unsigned char g; };
struct CRGB *leds;

unsigned long	t,
  startTime,
  lastByteTime,
  lastAckTime;

uint8_t hi, lo, chk,
  mode          = MODE_HDRA;

int channels,channel;

void setup()
{
  Serial.begin(115200);
  FastSPI_LED.setLeds(NUM_LEDS);

  //Change this to match your led strip
  FastSPI_LED.setChipset(CFastSPI_LED::SPI_LPD6803);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_TM1809);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_HL1606);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_595);
  //FastSPI_LED.setChipset(CFastSPI_LED::SPI_WS2801);

  FastSPI_LED.setPin(PIN);
  //Change datarate to match your led strip as well
  FastSPI_LED.setDataRate(3);

  FastSPI_LED.init();
  FastSPI_LED.start();
  
  leds = (struct CRGB*)FastSPI_LED.getRGBData();
  clearLeds();

  Serial.print("Ada\n");
  lastByteTime = startTime = lastAckTime = millis();
}

int readByte(){
  while(Serial.available()==0){
    ;
    //FastSPI_LED.stop();
  }
  //FastSPI_LED.start();
  return Serial.read();
}

void clearLeds(){
  memset(leds, 0, NUM_LEDS * 3);
  FastSPI_LED.show();
  delay(20);
}

void loop(){
  int16_t		c;

  c = readByte();
  t = millis();

  if((t - lastAckTime) > 1000) {
    Serial.print("Ada\n"); // Send ACK string to host
    lastAckTime = t; // Reset counter
  }
  if ((t - lastByteTime) > 5000) {
    mode = MODE_HDRA;     // After 5 seconds reset to HDRA
  }
  lastByteTime = t;
  lastAckTime = t;
  
  switch(mode){
  case MODE_HDRA:
    if (c == 0x41) {
      mode = MODE_HDRd;
    }
    break;
  case MODE_HDRd:
    if (c == 0x64) {
      mode = MODE_HDRa;
    }
    break;
  case MODE_HDRa:
    if (c == 0x61) {
      mode = MODE_HDRhi;
    }
    break;
  case MODE_HDRhi:
    hi = c;
    mode = MODE_HDRlo;
    break;
  case MODE_HDRlo:
    lo = c;
    mode = MODE_HDRchk;
    break;
  case MODE_HDRchk:
    chk = c;
    if (chk == (hi ^ lo ^ 0x55)){
      mode = MODE_DATAr;
      channels = (long)hi*256+(long)lo;
      channel = 0;
     } else {
      // wrong checksum, reset header
      mode = MODE_HDRA;
    }
    break;
  case MODE_DATAr:
    // we are in the data business ;-)
    // ignore all data for channels higher than NUM_LEDS
    if (channel<NUM_LEDS) { leds[channel].r = c; }
    mode = MODE_DATAb;
    break;
  case MODE_DATAb:
    // we are in the data business ;-)
    // ignore all data for channels higher than NUM_LEDS
    if (channel<NUM_LEDS) { leds[channel].b = c; }
    mode = MODE_DATAg;
    break;
  case MODE_DATAg:
    // we are in the data business ;-)
    // ignore all data for channels higher than NUM_LEDS
    if (channel<NUM_LEDS) { leds[channel].g = c; }
    channel++;
    if (channel>channels) {
      FastSPI_LED.show();
      mode = MODE_HDRA;
    } else {
      mode = MODE_DATAr;
    }
    break;
  default:
    // I should not be here, back to
    mode = MODE_HDRA;
  }    
}
