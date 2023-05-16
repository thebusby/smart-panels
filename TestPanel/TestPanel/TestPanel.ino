#include "Panel.h"



/*
 * Setup IO Expander
 */
MyPCF8575 *PCF = new MyPCF8575(new PCF8575(0x20));

/*
 * Define Panel Specific Values here
*/

/*
IOMethod* switch_io[] = 
{new PCF8575IOMethod(PCF, 0, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 1, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 2, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 3, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 4, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 5, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 6, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 7, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 8, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 9, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 10, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 11, iomt_input_pullup)
,NULL};

InputComponent *inputs[] =
{new ButtonComponent("KEY_L1", new DirectIOMethod(2, iomt_input_pullup))
,new ButtonComponent("KEY_L2", new DirectIOMethod(3, iomt_input_pullup))
,new ButtonComponent("KEY_L3", new DirectIOMethod(4, iomt_input_pullup))
,new ButtonComponent("KEY_L4", new DirectIOMethod(5, iomt_input_pullup))
,new ButtonComponent("KEY_L5", new DirectIOMethod(6, iomt_input_pullup))

,new ButtonComponent("KEY_T1", new DirectIOMethod(7, iomt_input_pullup))
,new ButtonComponent("KEY_T2", new DirectIOMethod(8, iomt_input_pullup))
,new ButtonComponent("KEY_T3", new DirectIOMethod(9, iomt_input_pullup))
,new ButtonComponent("KEY_T4", new DirectIOMethod(10, iomt_input_pullup))
,new ButtonComponent("KEY_T5", new DirectIOMethod(11, iomt_input_pullup))

,new ToggleComponent("TOGGLE", new DirectIOMethod(12, iomt_input_pullup))

,new SwitchComponent("SWITCH", switch_io)
,NULL
};

OutputComponent *outputs[] =
{new RGBLedComponent("RGBLED", new DirectIOMethod(A6, iomt_output), new DirectIOMethod(A7, iomt_output), new DirectIOMethod(A3, iomt_output) )
,NULL
};

Panel *panel = new Panel("BUTTON_PANEL", inputs, outputs);
*/

InputComponent *inputs[] =
{
// new ButtonComponent("BUTTON", new DirectIOMethod(3, iomt_input_pullup))
new ButtonComponent("BUTTON", new PCF8575IOMethod(PCF, 8, iomt_input_pullup))
,new ButtonComponent("RE_BUT", new DirectIOMethod(9, iomt_input_pullup))
,new EncoderComponent("DIAL", new DirectIOMethod(7, iomt_input), new DirectIOMethod(8, iomt_input))
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
