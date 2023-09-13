// #define ST7920_SUPPORT
// #define PCF8575_SUPPORT
#define LCD20X4_SUPPORT 
#include "Panel.h"

/*
 * Define Panel Specific Values here
*/

InputComponent *inputs[] =
{
// new ButtonComponent("BUTTON", new DirectIOMethod(3, iomt_input_pullup))
new ButtonComponent("BUTTON", new DirectIOMethod(8, iomt_input_pullup))
,new ButtonComponent("RE1_BUT", new DirectIOMethod(4, iomt_input_pullup))
,new EncoderComponent("RE1", new DirectIOMethod(2, iomt_input_pullup), new DirectIOMethod(3, iomt_input_pullup))
,new ButtonComponent("RE2_BUT", new DirectIOMethod(7, iomt_input_pullup))
,new EncoderComponent("RE2", new DirectIOMethod(5, iomt_input_pullup), new DirectIOMethod(6, iomt_input_pullup))
,NULL
};

OutputComponent *outputs[] =
{
new RGBLedComponent("RGBLED", new DirectIOMethod(12, iomt_output), new DirectIOMethod(11, iomt_output), new DirectIOMethod(10, iomt_output) )
,new LCD20X4Component("LCD", 0x27) // Where 0x27 is the I2C address of the LCD20X4
,NULL
};

Panel *panel = new Panel("MUSIC_PANEL", inputs, outputs);



void setup() {
    Serial.begin(115200);

    // Serial.println("DEBUG\tsetup() started"); Serial.flush();

    // Initialize panel
    if(!panel->setup()) {
      Serial.println("Panel init failed, do something!");
      Serial.flush();
    }
}

void loop() {
    panel->loop();
}
