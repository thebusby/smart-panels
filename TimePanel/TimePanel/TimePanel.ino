#include <Arduino.h>
#include <Wire.h>
#include <DS3231M.h>
#include <TM1637.h> // For SSFD
#include <LedControl.h> // For SSED
#include <Adafruit_MCP23017.h>
#include "my_internal_state.h"


// Enables Output to Serial Port per protocol
#define USE_SERIAL true

// Enables logging DEBUG to serial console
// #define DEBUG_SERIAL true


//
// Define PINOUT
//
#define PIN_RE_ROT1 2
#define PIN_RE_ROT2 3
#define PIN_SSD_CLK 4
#define PIN_SSFD_NOW 5
#define PIN_SSED_DIN 6
#define PIN_SSED_CS 7
#define PIN_RB_TOP 8
#define PIN_RB_MID 9
#define PIN_RB_BOT 10
#define PIN_SSED2_CS 11
#define PIN_SSED2_CLK 12
#define PIN_SSED2_DIN 13
#define PIN_BUZZER 14 // A0
#define PIN_SSED3_DIN 15 // A1
#define PIN_SSED3_CS 16 // A2
#define PIN_SSED3_CLK 17 // A3
#define PIN_SDA 18 // A4
#define PIN_SCL 19 // A5
#define PIN_MCP_LED1_R 0 // PA0
#define PIN_MCP_LED1_B 1 // PA1
#define PIN_MCP_LED1_G 2
#define PIN_MCP_LED2_R 3
#define PIN_MCP_LED2_G 5
#define PIN_MCP_LED2_B 4 
#define PIN_MCP_LED3_G 6 
#define PIN_MCP_LED3_R 7 // PA7
#define PIN_MCP_SW_BOT_RED 10 // PB2
#define PIN_MCP_SW_MID_RED 11
#define PIN_MCP_SW_TOP_RED 12
#define PIN_MCP_SW_BOT_BLK 13
#define PIN_MCP_SW_MID_BLK 14
#define PIN_MCP_SW_TOP_BLK 15 // PB7

// LED pin map
const int led_map[3][2] = { {PIN_MCP_LED1_R, PIN_MCP_LED1_G}, {PIN_MCP_LED2_R, PIN_MCP_LED2_G},{PIN_MCP_LED3_R, PIN_MCP_LED3_G} };
const int sw_map[3][2] = { {PIN_MCP_SW_TOP_BLK, PIN_MCP_SW_TOP_RED}, {PIN_MCP_SW_MID_BLK, PIN_MCP_SW_MID_RED}, {PIN_MCP_SW_BOT_BLK, PIN_MCP_SW_BOT_RED} };


// 
// Arduino Nano Pin Interrupt Notes
// 
// PortB group is PCINT0, 10-14 (D8-D13)
// PortC group is PCINT1, 28-23 (A0-A5)
// PortD group is PCINT2, 13-14 (D0-D7)

// D2 - Rotary Encoder 1 (INT)
// D3 - Rotary Encoder 2 (INT)
// D4 - SSD_CLK
// D5 - SSFD_NOW
// D6 - SSED_DIN
// D7 - SSED_CS
// D8 - RB Top (INT)
// D9 - RB Mid (INT)
// D10 - RB Bot (INT)
// D11 - 
// D12 - 
// A0 - Buzzer
// A1 -
// A2 -
// A4 - SDA (RTC, MP)
// A5 - SCL (RTC, MP)
// A6 - 
// A7 - 
// MCP PA0 - RGB LED - R
// MCP PA1 - RGB LED - G
// MCP PA2 - RGB LED - B
// MCP PA3 - RGB LED - R
// MCP PA4 - RGB LED - G
// MCP PA5 - RGB LED - B
// MCP PA6 - RGB LED - R
// MCP PA7 - RGB LED - G
// MCP PB2 - TOG
// MCP PB3 - TOG
// MCP PB4 - TOG
// MCP PB5 - TOG
// MCP PB6 - TOG
// MCP PB7 - TOG

//
// Rotary Encoder
// 
#include <RotaryEncoder.h>
RotaryEncoder *encoder = nullptr;
int re_oldpos = 0; // Remember absolute position of RE

void re_check_position() {
  encoder->tick();
}


// 
// Display Params
// 
#define SSFD_LED_BRIGHTNESS 2
#define SSED_LED_BRIGHTNESS 1
#define SSED_DISPLAY_COUNT  3


//
// Initialize Libraries
//
LedControl ssed[] = { 
  LedControl(PIN_SSED_DIN, PIN_SSED2_CLK, PIN_SSED_CS),
  LedControl(PIN_SSED2_DIN, PIN_SSED2_CLK, PIN_SSED2_CS),
  LedControl(PIN_SSED3_DIN, PIN_SSED3_CLK, PIN_SSED3_CS)
};


// LedControl ssed[0]=LedControl(PIN_SSED_DIN, PIN_SSD_CLK, PIN_SSED_CS, 1);
// LedControl ssed[1]=LedControl(PIN_SSED2_DIN, PIN_SSED2_CLK, PIN_SSED2_CS, 1);
// LedControl ssed[2]=LedControl(PIN_SSED3_DIN, PIN_SSED3_CLK, PIN_SSED3_CS, 1);
TM1637 ssfd_now(PIN_SSD_CLK, PIN_SSFD_NOW);
DS3231M_Class rtc;
Adafruit_MCP23017 mcp;


//
// Basic Functions
//
void serial_output(String tag, String msg) {
#ifdef USE_SERIAL
  Serial.println("O\t" + tag + "\t" + msg); 
#endif
}

void debug(String s) {
#ifdef DEBUG_SERIAL
  Serial.println("DEBUG\t" + s);
#endif
}


//
// Display Functions
//
void ssed_display(LedControl lc, int lc_id,  unsigned long num) {
  unsigned long disp_num = num;
  lc.clearDisplay(lc_id);

   for(int i=0; i<8; i++) {
     lc.setDigit(lc_id, i, (byte)(disp_num % 10), false);
     disp_num = disp_num / 10;
   }
}

void ssfd_display(TM1637 tm, unsigned int num) {
  int i=0;

  for(i=0; i<4; i++){
    tm.display((3-i), (num % 10));
    num = num / 10;
  }
}


// Enable or disable buzzer w/ HIGH or LOW
// 
void buzzer(bool state) {
  if(state)
    digitalWrite(PIN_BUZZER, HIGH);
  else
    digitalWrite(PIN_BUZZER, LOW);
}


#define LED_COLOR_R 0 
#define LED_COLOR_G 1

#define LED_STATE_OFF 0
#define LED_STATE_ON 1
#define LED_STATE_BLINK 3
#define LED_STATE_BLINK_ON 3
#define LED_STATE_BLINK_OFF 2

void set_led(MyInternalState *is, int led_id, int led_color, int led_state) {

  debug("sed_led\tID" + String(led_id) + "-" + String(led_color) + "\t" + String(led_state));

  // Set initial LED state, *IF* it hasn't already been set
  if( bitRead(led_state, 0) ) { // want it on
    
    if(! bitRead(is->leds[led_id][led_color], 0)) { // but it's currently off
      mcp.digitalWrite(led_map[led_id][led_color], HIGH);
      debug("set_led\tON");
    }
  }else{ // want it off
    if(bitRead(is->leds[led_id][led_color], 0)) { // but it's currently on
      mcp.digitalWrite(led_map[led_id][led_color], LOW);
      debug("set_led\tOFF");
    }
  }

  // Record state
  is->leds[led_id][led_color] = led_state;
}

void led_off(MyInternalState *is, int led_id) {
  for(int i=0; i < 2; i++) {
    set_led(is, led_id, i, LED_STATE_OFF);
  }  
}

#define TOGGLE_LED 0
#define TOGGLE_BUZZER 1

// Return toggle switch state for alarm
bool get_sw_state(MyInternalState *is, int alarm_id, int sw_id) {
  return is->toggle_state[sw_map[alarm_id][sw_id]];
}


//
// USER FUNCTIONS HERE
// 
void display_alarms(MyInternalState *is) {
  for(int display_id=0; display_id<SSED_DISPLAY_COUNT; display_id++) {
    unsigned long display_digits = 0;
    unsigned int alarm = is->alarms[display_id];
    unsigned int time_digits = is->ssfd_now;

    // Ignore any old alarms
    if(alarm) {
      int hour_diff = (alarm / 100) - (time_digits / 100);
      int min_diff = (alarm % 100) - (time_digits % 100);

      // Handle case of upcoming alarm where mins go negative
      if((hour_diff > 0) && (min_diff < 0)) {
        hour_diff--;
        min_diff=60+min_diff;       
      }

      // Perform Custom Alarm logic here
      //
      if(!hour_diff && (min_diff < 15)) {
        if(min_diff < 6) {
          if(get_sw_state(is, display_id, TOGGLE_LED)) {
            set_led(is, display_id, LED_COLOR_R, LED_STATE_OFF);
            if(is->leds[display_id][LED_COLOR_G] == LED_STATE_OFF) // Only enable LED if it's off currently, otherwise we could be blinking
              set_led(is, display_id, LED_COLOR_G, LED_STATE_ON);
          }
        }else{
          if(get_sw_state(is, display_id, TOGGLE_LED)) {
            set_led(is, display_id, LED_COLOR_R, LED_STATE_ON);
          }
        }
      } 


      // Case we're in alarm mode
      if(!hour_diff && (min_diff < 2) && (min_diff > -30)) {
        bool want_led = get_sw_state(is, display_id, TOGGLE_LED);
        bool want_buz = get_sw_state(is, display_id, TOGGLE_BUZZER);
        
        if(want_led) // Make LED blink
          if(is->leds[display_id][LED_COLOR_G] == LED_STATE_ON) // Should already be enabled, but we don't want to disable BLINK_OFF
            set_led(is, display_id, LED_COLOR_G, LED_STATE_BLINK_ON);
        if(want_buz) // Set off buzzer
          buzzer(true);

        debug("ALARM\t" + String(min_diff) + "\t" + String(want_led) + "\t" +String(want_buz));

        // For display reasons, just set minutes difference to zero
        min_diff = 0;
      }


      // Construct 8 digit value for display
      // Format hhmmHHMM where 'h' is hours until alarm, 'm' is minutes until alarm, "H" is hour alarm is set at, "M" is minute alarm is set at.
      display_digits=((unsigned long)hour_diff) * 1000000UL;
      display_digits+=((unsigned long)min_diff) * 10000UL;
      display_digits+=alarm;

      debug("SSED\t" + String(display_id) + "\tTIME\t" + String(hour_diff) + "\t" + String(min_diff) +  "\t" + String((unsigned long)display_digits));
      ssed_display(ssed[display_id], 0, display_digits);

    }else{ // Case alarm has expired
      ssed[display_id].clearDisplay(0);

    }
  }
}



//
// SETUP HERE
// 
void setup() {

  // Initialize Displays
  ssfd_now.init();  
  ssfd_now.set(SSFD_LED_BRIGHTNESS);
  ssfd_now.point(true);

  for(int i=0; i < SSED_DISPLAY_COUNT; i++) {
    ssed[i].shutdown(0, false);
    ssed[i].setIntensity(0, SSED_LED_BRIGHTNESS);
    ssed[i].clearDisplay(0);
  }


  // Setup pins
  pinMode(PIN_RB_TOP, INPUT_PULLUP);
  pinMode(PIN_RB_MID, INPUT_PULLUP);
  pinMode(PIN_RB_BOT, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);


  // Setup MCP
  mcp.begin();

  // Configure LED pins
  for(int i=0; i<8; i++) {
    mcp.pinMode(i, OUTPUT);
  }
  // Configure Toggle Switch pins
  for(int i=10; i<16; i++) {
    mcp.pinMode(i, INPUT);
    mcp.pullUp(i, HIGH);
  }
  // Start with LEDs off
  for(int i=0; i<8; i++)
    mcp.digitalWrite(i, LOW);
  
  // Setup Serial
#ifdef USE_SERIAL
  Serial.begin(115200);
  Serial.println("INIT\tTIME");
#endif


  // Setup Rotary Encoder
  encoder = new RotaryEncoder(PIN_RE_ROT1, PIN_RE_ROT2, RotaryEncoder::LatchMode::TWO03);
  // attachInterrupt(digitalPinToInterrupt(PIN_RE_ROT1), re_check_position, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(PIN_RE_ROT2), re_check_position, CHANGE);


  // Handle RTC init
  while(!rtc.begin()) {
    serial_output("RTC", "Init Failing");
    delay(1000);
  }


  // Set time if it hasn't already been set
  DateTime now = rtc.now();
  if(!now.hour() && !now.minute()) { // Only reset internal clock if we think it's wrong.
    DateTime now(__DATE__, __TIME__);
    rtc.adjust(now);
  }
}


//
// Define frequency of various checks/execution
//
#define LOOP_DISPLAY_INTERVAL 100000
#define LOOP_RB_INTERVAL 100
#define LOOP_SW_INTERVAL 10000
#define LOOP_LED_BLINK_INTERVAL 1000
#define TMP_TIMEOUT_DELAY 100000

//
// MAIN STARTS HERE
//
void loop() {
  static MyInternalState is;
  static unsigned long loop_count = 0;
  bool force_display = false;

  // Light debugging
  // static int debug_light = 0;
  // if(!(loop_count%10000)) {
  //   mcp.digitalWrite((debug_light%8), LOW);
  //   debug_light++;
  //   mcp.digitalWrite((debug_light%8), HIGH);
  // }

  // Handle Rotary Encoder
  // 
  encoder->tick();
  int re_pos = encoder->getPosition();
  if(is.re_pos != re_pos) {
    int re_dir = ((int)encoder->getDirection()) * -1; 

    // If we're enabling tmp state grab the current time to start from
    if(!is.tmp_timeout){
      is.ssfd_tmp = (is.ssfd_now / 10) * 10;
    }
    is.tmp_timeout = loop_count + TMP_TIMEOUT_DELAY;

    // Handle the fact we have 60 minutes is an hour, and not 100
    if(re_dir > 0) { // Increment
      if( (is.ssfd_tmp % 100) == 55 ) {
        is.ssfd_tmp += 45; 
      }
      if(is.ssfd_tmp >= 2395) {
        is.ssfd_tmp = 0; 
      }
    }else{ // Decrement
      if(is.ssfd_tmp <= 5) {
        is.ssfd_tmp = 2400;
      } 
      if( (is.ssfd_tmp % 100) == 0) {
        is.ssfd_tmp -= 40;  
      }
    }
    is.ssfd_tmp += re_dir * 5; // Increment in units of 10 minutes 

    force_display=true;
    is.re_pos = re_pos;
  } // end rotary loop


  // Handle buttons
  // 
  if( !(loop_count%LOOP_RB_INTERVAL)) {
    for(int i=0; i<3; i++) {
      int rb = digitalRead((i+8)); // To cover pins 8-10

      if(rb != is.rbs[i]){
        if(rb) { // Button release
          serial_output("RB", "RELEASE\t" + String(i));
          serial_output("ALARM", "SET\t" + String(i) + "\t" + String(is.ssfd_tmp));

          if(is.ssfd_tmp) { // SETTING the alarm
            is.alarms[i] = is.ssfd_tmp; // Set the alarm
            is.ssfd_tmp = 0; // Disable alternate display state
            is.tmp_timeout = 0; // disable UI timer

          }else{ // REMOVING the alarm
            is.alarms[i] = 0;
          }

          force_display = true;
          buzzer(false); // Always disable buzzer
          led_off(&is, i); // Disable LED
        }else{ // Button press
          serial_output("RB", "PRESS\t" + String(i));
        }
        is.rbs[i] = rb;
      }
    }
  }

  // Handle Switches
  // 
  if( !(loop_count%LOOP_SW_INTERVAL)) {
    // Check MCU for switch state

    for(int i=MCP_TOGGLE_MIN; i<=MCP_TOGGLE_MAX; i++) {
      int tog = mcp.digitalRead(i);

      if(tog!=is.toggle_state[i]){
        if(tog) {
          serial_output("TOG", "ON\t" + String(i));
        }else{
          serial_output("TOG", "OFF\t" + String(i));

          // Turn off any LED's currently on
          if( (i >12) && (i<16)) { // Ignore buzzer toggle switches though
            int id = 15 - i; // So 15 (Top) is 0, and 13 (Bot) is 2
            led_off(&is, id);
          }
        }

        // Update state
        is.toggle_state[i] = tog;
      }
    }
  } // END LOOP_SW_INTERVAL


  // Make LEDs blink
  if( !(loop_count%LOOP_LED_BLINK_INTERVAL)) {
    for(int led_id=0; led_id<3; led_id++) {
      for(int led_color=0; led_color<2; led_color++) {

        // Toggle On/Off
        if(is.leds[led_id][led_color] == LED_STATE_BLINK_ON) {
          debug("BLINK\tOFF\t" + String(led_id) + "-" + String(led_color));
          set_led(&is, led_id, led_color, LED_STATE_BLINK_OFF);
        }else{
          if(is.leds[led_id][led_color] == LED_STATE_BLINK_OFF) {
            debug("BLINK\tON\t" + String(led_id) + "-" + String(led_color));
            set_led(&is, led_id, led_color, LED_STATE_BLINK_ON);
          }
        }
      }
    }
  }


  // Handle UI time outs
  if(is.tmp_timeout && (loop_count >= is.tmp_timeout)) {
    is.tmp_timeout = 0;
    is.ssfd_tmp = 0;
    force_display=true;
  }


  if( !(loop_count%LOOP_DISPLAY_INTERVAL) || force_display) {
    DateTime now = rtc.now();
    unsigned int time_digit = 0;
    unsigned long debug_digit = 0;

    // Keep time
    is.ssfd_now = (now.hour() * 100) + now.minute();
    
    // Choose time to display on SSFD
    if(is.tmp_timeout) {
      time_digit = is.ssfd_tmp;
    }else{
      time_digit = is.ssfd_now; 
    }

    // debug("SSFD\t" + String(time_digit));
    ssfd_display(ssfd_now, time_digit);
    display_alarms(&is);

    force_display = false;
  } // end display loop


  loop_count++;
}