#include "Panel.h"



/*
 * Setup IO Expander
 */
MyPCF8575 *PCF = new MyPCF8575(new PCF8575(0x20));

/*
 * Define Panel Specific Values here
*/
InputComponent *inputs[] =
{new ButtonComponent("BUT1", new DirectIOMethod(3, iomt_input_pullup))
//,new ToggleComponent("TOG1", new DirectIOMethod(3, iomt_input_pullup))
,NULL
};

OutputComponent *outputs[] =
{new LedComponent("ILED", new DirectIOMethod(LED_BUILTIN, iomt_output))
,new RGBLedComponent("RGBLED", new DirectIOMethod(4, iomt_output), new DirectIOMethod(5, iomt_output), new DirectIOMethod(6, iomt_output) )
,NULL
};

Panel *panel = new Panel("TEST_PANEL", inputs, outputs);



void setup() {
    Serial.begin(115200);

    // Initialize panel
    if(!panel->setup()) {
        // Panel init failed, do something!
    }

    // Initialize IO expander
    //if(!PCF->setup()) {
        // IO Expander init failed, do something!
    //}

    // Serial.println("SETUP: SUCCESS");
    // Serial.flush();
}

void loop() {
    panel->loop();
}
