#include <Arduino.h>
// #include <Adafruit_Sensor.h>


// Enables Output to Serial Port per protocol
#define USE_SERIAL true

// Enables logging DEBUG to serial console
// #define DEBUG_SERIAL true

// Define loop timings
#define LOOP_SENSOR_COUNT 100
#define LOOP_DELAY_MS 100
#define LOOP_AC_DELAY 20 // ~2 second delay, on A/C input


// 
// DEFINE PIN OUT
// 
#define  PIN_RE_ROT1 2
#define  PIN_RE_ROT2 3
#define  PIN_SSFD_CLK 4
#define  PIN_SSFD_COO 5
#define  PIN_SSFD_HUM 6
#define  PIN_SSFD_TMP 7
#define  PIN_SSFD_AC  8
#define  PIN_SENSOR_DHT 9
#define  PIN_SENSOR_COO 10
#define  PIN_SWC_TOP 11
#define  PIN_SWC_BTM 12


//
// Temp and Humidity Sensor
// 
#include <DHT.h>
#include <DHT_U.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
#define DHTTYPE DHT11

DHT dht(PIN_SENSOR_DHT, DHTTYPE);


//
// 4-Digit Display Pins
//
#include <TM1637.h>
#define SSFD_BRIGHTNESS 1 // set brightness; 0-7
TM1637 ssfd_tmp(PIN_SSFD_CLK, PIN_SSFD_TMP);
TM1637 ssfd_hum(PIN_SSFD_CLK, PIN_SSFD_HUM);
TM1637 ssfd_coo(PIN_SSFD_CLK, PIN_SSFD_COO);
TM1637 ssfd_ac(PIN_SSFD_CLK, PIN_SSFD_AC);


//
// Rotary Encoder
// 
#include <RotaryEncoder.h>
RotaryEncoder *encoder = nullptr;
unsigned int wanted_temp = 24; // Remember desired temp to display
int re_oldpos = 0; // Remember absolute position of RE

void re_check_position() {
  encoder->tick();
}


//
// MH-Z19 infrared CO2 sensor
//
unsigned int readCO2PWM() {
  unsigned long th, tl, ppm_pwm = 0;
  do {
    th = pulseIn(PIN_SENSOR_COO, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    // ppm_pwm = 5000 * (th - 2) / (th + tl - 4); // Assumes max of 5000 with PWM mode
    ppm_pwm = 2000 * (th - 2) / (th + tl - 4); // Assumes max of 2000 with PWM mode
  } while (th == 0);

  return ppm_pwm;
}

// Define system state
struct display_state {

  // Displays
  unsigned int tmp;
  unsigned int hum;
  int ppm;
  unsigned int ac = 24; // Need a default value as we'll +1 or -1 depending on RE changes

  // Controls
  int re_pos = 0;
  int top_sw = 0;
  int bottom_sw = 0;
};
typedef struct display_state DisplayState;


void setup() {
  // put your setup code here, to run once:

  ssfd_tmp.init();
  ssfd_hum.init();
  ssfd_coo.init();
  ssfd_ac.init();

  // set brightness; 0-7
  ssfd_tmp.set(SSFD_BRIGHTNESS);
  ssfd_hum.set(SSFD_BRIGHTNESS);
  ssfd_coo.set(SSFD_BRIGHTNESS);
  ssfd_ac.set(SSFD_BRIGHTNESS);

  // Setup temp+humidity sensor
  dht.begin();

  // Setup CO2 sensor
  pinMode(PIN_SENSOR_COO, INPUT);

  // Setup two switches on front
  pinMode(PIN_SWC_TOP, INPUT);
  pinMode(PIN_SWC_BTM, INPUT);

  // Setup Rotary Encoder
  encoder = new RotaryEncoder(PIN_RE_ROT1, PIN_RE_ROT2, RotaryEncoder::LatchMode::TWO03);
  attachInterrupt(digitalPinToInterrupt(PIN_RE_ROT1), re_check_position, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_RE_ROT2), re_check_position, CHANGE);

#ifdef USE_SERIAL
  Serial.begin(115200);
  Serial.println("INIT\tENV");
#endif
}

void display_digit(TM1637 tm, unsigned int num) {
  int i=0;

  for(i=0; i<4; i++){
    tm.display((3-i), (num % 10));
    num = num / 10;
  }
}

void display_float(TM1637 tm, float num) {

  tm.display(3, (((int)num / 10) % 10));
  tm.display(2, ((int)num % 10));
  tm.display(1, (((int)num * 10) % 10));
  tm.display(0, (((int)num * 100) % 10));
  tm.point(0); // hopefully this enables the decimal point
}

void display_digit_no_leading_zero(TM1637 tm, unsigned int num) {
  int i=0;
  int base=1000;
  int nolead=0;

  for(i=0; i<4; i++) {
    int digit = ((num / base) % 10);
    if (digit || nolead) {
      tm.display(i, digit);
      nolead=1;
    }
    base = base / 10;
  }
}

void serial_output(String tag, String msg) {
#ifdef USE_SERIAL
  Serial.println("O\t" + tag + "\t" + msg); 
#endif
}


//
// MAIN STARTS HERE
//
void loop() {
  static DisplayState ds;
  static unsigned long loop_count=0;
  static unsigned long ac_loop_count=0;
  int i=0;

  // Only check sensors every N loops
  if(!(loop_count%LOOP_SENSOR_COUNT)) {
    unsigned int t = (unsigned int)dht.readTemperature();
    unsigned int h = (unsigned int)dht.readHumidity();
    unsigned int ppm = readCO2PWM();
    // float hic = dht.computeHeatIndex(t, h, false);

#ifdef DEBUG_SERIAL
    Serial.println("DEBUG\t" + String((unsigned long)loop_count) + " Env: " + String((int)t) + "C " + String((int)h) + "H " + String(ppm) + " CO2");
#endif

    // Check Temp
    if(t != ds.tmp) {
      ssfd_tmp.display(0,12); // Display Leading C
      display_digit_no_leading_zero(ssfd_tmp, ((int)t));
      serial_output("TMP", String(t));

      ds.tmp = t;
    }

    // Check Humidity
    if(h != ds.hum) {
      ssfd_hum.displayRaw(0,B01110110); // Display Leading H
      display_digit_no_leading_zero(ssfd_hum, ((int)h));
      serial_output("HUM", String(h));

      ds.hum = h;
    }

    // Check CO2 ppm
    if(ppm != ds.ppm) {
      display_digit(ssfd_coo, ppm);
      serial_output("COO", String(ppm));

      ds.ppm = ppm;
    }
  } // End sensor loop


  // Check rotary encoder state 
  encoder->tick();
  int re_pos = encoder->getPosition();
  if(ds.re_pos != re_pos) {
    int new_temp = ds.ac + ((int)encoder->getDirection());

    // Check desired AC temp
    if((new_temp > 17) && (new_temp < 31)) {
      // Display Value
      ssfd_ac.display(0,10); // Display Leading 'A'
      ssfd_ac.display(1,12); // Display Leading 'C'
      ssfd_ac.display(2, (new_temp/10));
      ssfd_ac.display(3, (new_temp%10));

      // Tag so we check back to see if there haven't been any changes after LOOP_AC_DELAY
      ac_loop_count = loop_count + LOOP_AC_DELAY;
      ds.ac = new_temp;
    }

    // Record last time UI was tweaked
    // last_touch_millis = millis();

    ds.re_pos = re_pos;
  }

  // Check if A/C value has stabilized
  // If so, report on value and reset ac_loop_count
  if(ac_loop_count && (loop_count >= ac_loop_count)) {
    serial_output("AC", String(ds.ac));
    ac_loop_count = 0;
  }


  // Check switch state
  int top_sw = (digitalRead(PIN_SWC_TOP) == LOW);
  if(top_sw != ds.top_sw) {
    if(top_sw)
      serial_output("TOP", "ON");
    else
      serial_output("TOP", "OFF");

    ds.top_sw = top_sw;
  }

  int bottom_sw = (digitalRead(PIN_SWC_BTM) == HIGH);
  if(bottom_sw != ds.bottom_sw) {
    if(bottom_sw)
      serial_output("BOT", "ON");
    else
      serial_output("BOT", "OFF");

      ds.bottom_sw = bottom_sw;
  }

#ifdef DEBUG_SERIAL
  // if(!(loop_count%10)) {
  if(0) {
    Serial.println("DEBUG\tTOP_SWC\t" + String((int)(digitalRead(PIN_SWC_TOP))));
    Serial.println("DEBUG\tBOT_SWC\t" + String((int)(digitalRead(PIN_SWC_BTM))));
  }
#endif

  // Add a basic delay on ever loop
  delay(LOOP_DELAY_MS);
  loop_count++;
}