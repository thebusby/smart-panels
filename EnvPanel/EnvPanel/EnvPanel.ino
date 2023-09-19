#define MHZ19_SUPPORT
#define SSFD_SUPPORT
#define DHT_SUPPORT
#include "Panel.h"

// Number of milliseconds between poll events
#define POLL_INTERVAL 60000

/*
 * Define Panel Specific Values here
 */

DhtComponent* dht = new DhtComponent("DHT", 9, DHT11);
Mhz19Component* mhz19 = new Mhz19Component("CO2", 10);

InputComponent* inputs[] =
{
// new ToggleComponent("BLK_TOG", new DirectIOMethod(11, iomt_input)), // Doesn't seem to work at all
new ToggleComponent("RED_TOG", new DirectIOMethod(12, iomt_input)),
// new EncoderComponent("RE", new DirectIOMethod(2, iomt_input_pullup), new DirectIOMethod(3, iomt_input_pullup) ),
mhz19,
dht,
NULL
};

SsfdComponent* ssfd_co2 = new SsfdComponent("SSFD_CO2", 4, 5);
SsfdComponent* ssfd_hum = new SsfdComponent("SSFD_HUM", 4, 6);
SsfdComponent* ssfd_tmp = new SsfdComponent("SSFD_TMP", 4, 7);

OutputComponent* outputs[] = 
{
ssfd_co2,
ssfd_tmp,
ssfd_hum,
new SsfdComponent("SSFD_AC", 4, 8),
NULL
};

Panel *panel = new Panel("ENV_PANEL", inputs, outputs);

void setup() {  
    Serial.begin(115200);

    // Initialize panel
    if(!panel->setup()) {
      Serial.println("ERR Panel init failed, do something!");
      Serial.flush();
    }
}

void loop() {
  static tick timer=0;

  panel->loop();

  // Handle custom panel behavour 
  if(is_tc_alert(timer)) {
      uint8_t t = dht->get_temp();
      uint8_t h = dht->get_humidity();
      uint8_t c = mhz19->get_co2();

      ssfd_tmp->display_digits(t, 0);
      ssfd_tmp->display_digit(0, 'C');

      ssfd_hum->display_digits(h, 0);
      ssfd_hum->display_digit(0, 'H');

      ssfd_co2->display_digits(c, 1);

      timer = get_tc_alert(POLL_INTERVAL);
    }
}
