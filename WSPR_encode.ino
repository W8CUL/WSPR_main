//
// Simple JT65/JT9/JT4/FT8/WSPR/FSQ beacon for Arduino, with the Etherkit
// Si5351A Breakout Board, by Jason Milldrum NT7S.
//
// Transmit an abritrary message of up to 13 valid characters
// (a Type 6 message) in JT65, JT9, JT4, a type 0.0 or type 0.5 FT8 message,
// a FSQ message, or a standard Type 1 message in WSPR.
//
// Connect a momentary push button to pin 12 to use as the
// transmit trigger. Get fancy by adding your own code to trigger
// off of the time from a GPS or your PC via virtual serial.
//
// Original code based on Feld Hell beacon for Arduino by Mark
// Vandewettering K6HX, adapted for the Si5351A by Robert
// Liesenfeld AK6L <ak6l@ak6l.org>.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>

#include "Wire.h"

// Mode defines
#define WSPR_TONE_SPACING 146  // ~1.46 Hz
#define FT8_TONE_SPACING 625   // ~6.25 Hz

#define WSPR_DELAY 683     // Delay value for WSPR
#define FT8_DELAY 159      // Delay value for FT8


//#define WSPR_DEFAULT_FREQ 14097200UL
#define WSPR_DEFAULT_FREQ 14095000UL

// freq -> WSPR detected
//6X1 -> ~2080
//590 -> 2040
//000 -> Within the band of 1500 hz

#define FT8_DEFAULT_FREQ 14075000UL
#define DEFAULT_MODE MODE_WSPR

// Hardware defines
#define BUTTON 12
#define LED_PIN 13

// Enumerations
enum mode { 
            MODE_WSPR,
            MODE_FT8 };

// Class instantiation
Si5351 si5351;
JTEncode jtencode;

// Global variables
unsigned long freq;
char message[] = "KE8TJE FM09";
char loc[]="xxxx";
uint8_t dbm = 27;

// 27 - 500 mW
// 23 - 200 mW

uint8_t tx_buffer[255];
enum mode cur_mode = DEFAULT_MODE;
uint8_t symbol_count;
uint16_t tone_delay, tone_spacing;

// Loop through the string, transmitting one character at a time.
void encode() {
  uint8_t i;
  
  // Reset the tone to the base frequency and turn on the output
  si5351.output_enable(SI5351_CLK0, 1);
  digitalWrite(LED_PIN, HIGH);
  
  for (i = 0; i < symbol_count; i++) {
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0);
    delay(tone_delay);
  }
  // Turn off the output
  si5351.output_enable(SI5351_CLK0, 0);
  digitalWrite(LED_PIN, LOW);
}



void set_tx_buffer(uint8_t pwr) {
  // Clear out the transmit buffer
  memset(tx_buffer, 0, 255);
  //jtencode.wspr_encode(call, loc, dbm, tx_buffer);

#ifdef FT8
  jtencode.ft8_encode(message, tx_buffer);
#endif

#ifdef WSPR
  jtencode.wspr_encode(call, loc_public, pwr, tx_buffer);
#endif
}


void setup_WSPR() {
  // Initialize the Si5351
  // Change the 2nd parameter in init if using a ref osc other
  // than 25 MHz
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);

  // Use the Arduino's on-board LED as a keying indicator.
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Use a button connected to pin 12 as a transmit trigger
  pinMode(BUTTON, INPUT_PULLUP);
  // Set the mode to use
  //cur_mode = MODE_JT65;

#ifdef FT8
  cur_mode = MODE_FT8;
  freq = FT8_DEFAULT_FREQ;
  symbol_count = FT8_SYMBOL_COUNT;  // From the library defines
  tone_spacing = FT8_TONE_SPACING;
  tone_delay = FT8_DELAY;
#endif

#ifdef WSPR
  cur_mode = MODE_WSPR;
  freq = WSPR_DEFAULT_FREQ;
  symbol_count = WSPR_SYMBOL_COUNT;  // From the library defines
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
#endif

  // Set the proper frequency, tone spacing, symbol count, and
  // tone delay depending on mode

  // Set CLK0 output
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);  // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 0);                  // Disable the clock initially
  //set_tx_buffer();
}
