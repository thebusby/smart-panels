#define ST7920_SUPPORT
#include "Panel.h"


/*
 * Setup IO Expander
 */
MyPCF8575 *PCF = new MyPCF8575(new PCF8575(0x20));

/*
 * Define Panel Specific Values here
*/

InputComponent *inputs[] =
{
// new ButtonComponent("BUTTON", new DirectIOMethod(3, iomt_input_pullup))
new ButtonComponent("BUTTON", new PCF8575IOMethod(PCF, 8, iomt_input_pullup))
,new ButtonComponent("RE_BUT", new PCF8575IOMethod(PCF, 13, iomt_input_pullup))
,new EncoderComponent("DIAL", new PCF8575IOMethod(PCF, 15, iomt_input_pullup), new PCF8575IOMethod(PCF, 14, iomt_input_pullup))
,NULL
};

OutputComponent *outputs[] =
{
  // new LedComponent("ILED", new DirectIOMethod(LED_BUILTIN, iomt_output))
new ST7920Component("LCD", 10)
,new RGBLedComponent("RGBLED", new DirectIOMethod(4, iomt_output), new DirectIOMethod(5, iomt_output), new DirectIOMethod(6, iomt_output) )
,NULL
};

Panel *panel = new Panel("TEST_PANEL", inputs, outputs);



void setup() {
    Serial.begin(115200);

    Serial.println("DEBUG\tsetup() started"); Serial.flush();

    // Initialize panel
    if(!panel->setup()) {
      Serial.println("Panel init failed, do something!");
      Serial.flush();
    }

    // Initialize IO expander
    if(!PCF->setup()) {
      Serial.println("IO Expander init failed, do something!");
      Serial.flush();
    }
}

void loop() {
    panel->loop();
}
