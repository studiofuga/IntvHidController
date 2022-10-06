/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>

/* This sketch demonstrates USB HID keyboard.
 * - PIN A0-A5 is used to send digit '0' to '5' respectively
 *   (On the RP2040, pins D0-D5 used)
 * - LED and/or Neopixels will be used as Capslock indicator
 */

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD()
};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

//------------- Input Pins -------------//
// Array of pins and its keycode.
// Notes: these pins can be replaced by PIN_BUTTONn if defined in setup()
uint8_t pins[] = { D5, D4, D3, D2, D9, D8, D7, D6 };

/*

 Pin: 
 1  0x80
 2  0x40
 3  0x20
 4  0x10
 6  0x08
 7  0x04
 8  0x02
 9  0x01
 
 
Direction:
D1 2      = 0x40
D2 2,9    = 0x41
D3 2,3,9  = 0x61
D4 2,3    = 0x60
D5 3      = 0x20
D6 3,9    = 0x21
D7 3,4,9  = 0x31
D8 3,4    = 0x30
D9 4      = 0x10
D10 4,9   = 0x11
D11 1,4,9 = 0x91
D12 1,4   = 0x90
D13 1     = 0x80
D14 1,9   = 0x81
D15 1,2,9 = 0xc1
D16 1,2   = 0xc0

K1 4,6    = 0x18
K2 4,7    = 0x14
K3 4,8    = 0x12
K4 3,6    = 0x28
K5 3,7    = 0x24
K6 3,8    = 0x22
K7 2,6    = 0x48
K8 2,7    = 0x44
K9 2,8    = 0x42
CL 1,6    = 0x88
K0 1,7    = 0x84
EN 1,8    = 0x82

S1 6,8    = 0x0a
S2 7,8    = 0x06
S3 6,7    = 0x0c

*/

// number of pins
uint8_t pincount = sizeof(pins)/sizeof(pins[0]);

// For keycode definition check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
uint8_t hidcode[] = { 
  HID_KEY_A, HID_KEY_B, HID_KEY_C, HID_KEY_D, HID_KEY_E, HID_KEY_F,HID_KEY_G,HID_KEY_H,
  HID_KEY_I, HID_KEY_J, HID_KEY_K, HID_KEY_L, HID_KEY_M, HID_KEY_N,HID_KEY_O,HID_KEY_P,

  HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, 
  HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, 
  HID_KEY_R, HID_KEY_0, HID_KEY_S,

  HID_KEY_X, HID_KEY_Y, HID_KEY_Z 
};

uint8_t keymap[] = {
  0x40, 0x41, 0x61, 0x60, 0x20, 0x21, 0x31, 0x30, 
  0x10, 0x11, 0x91, 0x90, 0x80, 0x81, 0xc1, 0xc0,
  
  0x18, 0x14, 0x12, 0x28, 0x24, 
  0x22, 0x48, 0x44, 0x42, 
  0x88, 0x84, 0x82,
  
  0x0a, 0x06, 0x0c, 0x00
};

#if defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS) || defined(ARDUINO_NRF52840_CIRCUITPLAY) || defined(ARDUINO_FUNHOUSE_ESP32S2)
  bool activeState = true;
#else
  bool activeState = false;
#endif

//------------- Neopixel -------------//
// #define PIN_NEOPIXEL  8
#ifdef PIN_NEOPIXEL

// How many NeoPixels are attached to the Arduino?
// use on-board defined NEOPIXEL_NUM if existed
#ifndef NEOPIXEL_NUM
  #define NEOPIXEL_NUM  10
#endif

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUM, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

#endif


// the setup function runs once when you press reset or power the board
void setup()
{
#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
  TinyUSB_Device_Init(0);
#endif

  // Notes: following commented-out functions has no affect on ESP32
  // usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  // usb_hid.setPollInterval(2);
  // usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("Intellivision Disk Controller");

  // Set up output report (on control endpoint) for Capslock indicator
  usb_hid.setReportCallback(NULL, hid_report_callback);

  usb_hid.begin();

  // led pin
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // neopixel if existed
#ifdef PIN_NEOPIXEL
  pixels.begin();
  pixels.setBrightness(50);

  #ifdef NEOPIXEL_POWER
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, NEOPIXEL_POWER_ON);
  #endif
#endif

  // Set up pin as input
  for (uint8_t i=0; i<pincount; i++)
  {
    pinMode(pins[i], activeState ? INPUT_PULLDOWN : INPUT_PULLUP);
  }

  // wait until device mounted
  while( !TinyUSBDevice.mounted() ) delay(1);
}


void loop()
{
  // poll gpio once each 2 ms
  delay(2);

  // used to avoid send multiple consecutive zero report for keyboard
  static bool keyPressedPreviously = false;

  uint8_t keyvalue=0;
  uint8_t keycode[6] = { 0 };
  int count = 0;

  // scan normal key and send report
  for(uint8_t i=0; i < pincount; i++)
  {
    
    if ( activeState == digitalRead(pins[i]) )
    {
      keyvalue |= (1 << (7-i));
      count = 1;
      
      // if pin is active (low), add its hid code to key report
      //keycode[count++] = hidcode[i];

      // 6 is max keycode per report
      //if (count == 6) break;
    }
  }

  if ( TinyUSBDevice.suspended() && count )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if ( !usb_hid.ready() ) return;

  if ( count )
  {
    // Send report if there is key pressed
    uint8_t const report_id = 0;
    uint8_t const modifier = 0;

    int idx = 0;
    while (keymap[idx] != 0) {
      if (keymap[idx] == keyvalue) {
        break;
      }
      ++idx;
    }

    if (keymap[idx] != 0) {
      keyPressedPreviously = true;
      keycode[0] = hidcode[idx];
      usb_hid.keyboardReport(report_id, modifier, keycode);
    }
  }else
  {
    // Send All-zero report to indicate there is no keys pressed
    // Most of the time, it is, though we don't need to send zero report
    // every loop(), only a key is pressed in previous loop()
    if ( keyPressedPreviously )
    {
      keyPressedPreviously = false;
      usb_hid.keyboardRelease(0);
    }
  }
}

// Output report callback for LED indicator such as Caplocks
void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) report_id;
  (void) bufsize;

  // LED indicator is output report with only 1 byte length
  if ( report_type != HID_REPORT_TYPE_OUTPUT ) return;

  // The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
  // Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
  uint8_t ledIndicator = buffer[0];

  // turn on LED if capslock is set
  digitalWrite(LED_BUILTIN, ledIndicator & KEYBOARD_LED_CAPSLOCK);

#ifdef PIN_NEOPIXEL
  pixels.fill(ledIndicator & KEYBOARD_LED_CAPSLOCK ? 0xff0000 : 0x000000);
  pixels.show();
#endif
}
