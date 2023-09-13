#define ST7920_SUPPORT
#define PCF8575_SUPPORT
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
new ButtonComponent("BUT_UP", new PCF8575IOMethod(PCF, 2, iomt_input_pullup))
,new ButtonComponent("BUT_MD", new PCF8575IOMethod(PCF, 1, iomt_input_pullup))
,new ButtonComponent("BUT_DN", new PCF8575IOMethod(PCF, 0, iomt_input_pullup))
,new ToggleComponent("TOG_1", new PCF8575IOMethod(PCF, 3, iomt_input_pullup))
,new ToggleComponent("TOG_2", new PCF8575IOMethod(PCF, 4, iomt_input_pullup))
,new ToggleComponent("TOG_3", new PCF8575IOMethod(PCF, 5, iomt_input_pullup))
,new ToggleComponent("TOG_4", new PCF8575IOMethod(PCF, 6, iomt_input_pullup))
,new ToggleComponent("TOG_5", new PCF8575IOMethod(PCF, 7, iomt_input_pullup))
,new ButtonComponent("RE_BUT", new DirectIOMethod(4, iomt_input_pullup))
,new EncoderComponent("DIAL", new DirectIOMethod(2, iomt_input_pullup), new DirectIOMethod(3, iomt_input_pullup))

//,new ButtonComponent("RE_BUT", new PCF8575IOMethod(PCF, 13, iomt_input_pullup))
// ,new EncoderComponent("DIAL", new PCF8575IOMethod(PCF, 15, iomt_input_pullup), new PCF8575IOMethod(PCF, 14, iomt_input_pullup))
,NULL
};

OutputComponent *outputs[] =
{
  // new LedComponent("ILED", new DirectIOMethod(LED_BUILTIN, iomt_output))
new ST7920Component("LCD", 10) // Where 10 is the CS pin
// new LedComponent("TLED", new PCF8575IOMethod(PCF, 0, iomt_output))
,new RGBLedComponent("RGBLED", new DirectIOMethod(5, iomt_output), new DirectIOMethod(7, iomt_output), new DirectIOMethod(6, iomt_output) )
,NULL
};

Panel *panel = new Panel("STATUS_PANEL", inputs, outputs);



void setup() {
    Serial.begin(115200);

    // Serial.println("DEBUG\tsetup() started"); Serial.flush();

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
