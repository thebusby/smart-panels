#define PCF8575_SUPPORT
#include "Panel.h"



/*
 * Setup IO Expander
 */
MyPCF8575 *PCF = new MyPCF8575(new PCF8575(0x20));

/*
 * Define Panel Specific Values here
*/
IOMethod* switch_io[] = 
{new PCF8575IOMethod(PCF, 0, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 1, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 2, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 3, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 4, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 5, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 6, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 7, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 15, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 14, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 13, iomt_input_pullup)
,new PCF8575IOMethod(PCF, 12, iomt_input_pullup)
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

,new ToggleComponent("TOGGLE", new DirectIOMethod(12, iomt_input_pullup)) // Doesn't work yet.

,new EncoderComponent("DIAL_T", new PCF8575IOMethod(PCF, 8, iomt_input_pullup), new PCF8575IOMethod(PCF, 9, iomt_input_pullup))
//,new ButtonComponent("DIAL_T_BUT", new DirectIOMethod(12, iomt_input_pullup)) // Doesn't work yet.

,new EncoderComponent("DIAL_L", new DirectIOMethod(A0, iomt_input), new DirectIOMethod(A1, iomt_input))
//,new ButtonComponent("DIAL_L_BUT", new DirectIOMethod(A2, iomt_input_pullup))

,new SwitchComponent("SWITCH", switch_io)
,NULL
};

OutputComponent *outputs[] =
{new RGBLedComponent("RGBLED", new DirectIOMethod(A2, iomt_output), new DirectIOMethod(13, iomt_output), new DirectIOMethod(A3, iomt_output) )
,NULL
};

Panel *panel = new Panel("BUTTON_PANEL", inputs, outputs);



void setup() {
    Serial.begin(115200);

    // Serial.println("DEBUG Setup() started");
    // Serial.flush();

    // Initialize panel
    if(!panel->setup()) {
      Serial.println("ERR\tPanel init failed, do something!");
      Serial.flush();
    }

    // Initialize IO expander
    if(!PCF->setup()) {
      Serial.println("ERR\tIO Expander init failed, do something!");
      Serial.flush();
    }

    // Serial.println("INIT");
    // Serial.flush();
}

void loop() {
    panel->loop();
}
