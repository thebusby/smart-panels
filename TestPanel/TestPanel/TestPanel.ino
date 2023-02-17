#include "Panel.h"


InputComponent *inputs[] =
{new ButtonComponent("BUT1", new DirectIOMethod(2, iomt_input_pullup))
,new ToggleComponent("TOG1", new DirectIOMethod(3, iomt_input_pullup))
,NULL
};

OutputComponent *outputs[] =
{new LedComponent("LED1", new DirectIOMethod(LED_BUILTIN, iomt_output))
,NULL
};

Panel *panel = new Panel("TEST_PANEL", inputs, outputs);


void parse_command(char* input, char** cmd, char** id, char** value) {
    int i;

    // Always start with command
    *cmd = input;

    // Terminate command
    for(i=0; input[i]; i++) {
        if(input[i] == '\t') {
            input[i] = '\0';
            i++;
            break;
        }
    }

    // Grad the id if it exists
    if(input[i])
        *id = &(input[i]);

    for(; input[i]; i++) {
        if(input[i] == '\t') {
            input[i] = '\0';
            i++;
            break;
        }
    }

    // Grad the value if it exists
    if(input[i])
        *value = &(input[i]);
        
    // value should already be NULL terminated
}

// cmds
// IDENT
// LIST
// SET ID ON/OFF/TOG
// POLL ID

void setup() {
    Serial.begin(115200);
}

void loop() {
    char buf[64];
    char* cmd;
    char* id;
    char* val;
    InputComponent *input = NULL;

    // Check Inputs
    for(input=inputs[0]; input; input++) {
        if(input->poll()) {
            input->getMessage(buf);
            Serial.println(buf);
            Serial.flush();
        }
    }

    // Check Serial
    if(Serial.available() > 0) {
        String str = Serial.readStringUntil('\n');
    }   


    delay(10);
}
