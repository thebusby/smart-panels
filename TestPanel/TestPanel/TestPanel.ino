#include "Panel.h"

#define SERIAL_BUFFER_SIZE 62 // 64 - "\r\n"


/*
 * Define Panel Specific Values here
*/
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


/*
 * Handle general panel logic
*/
typedef struct cmd {
  char *cmd_name;
  char* (*cmd_func)(char*);
} cmd_t;

/* Protocol handlers */
char* com_prot_ident   (char*);
char* com_prot_ping    (char*);
char* com_prot_set     (char*);
char* com_prot_get     (char*);

/* List of commands */
cmd_t command[] = {
  { "IDENT",    com_prot_ident },
  { "PING",     com_prot_ping },
  { "SET",      com_prot_set },
  { "GET",      com_prot_get },
  { 0 }
};

/* Tokenize space delimited text */
char* pop_token(char* input, char** next) {
    int i;
    char* token = input;

    for(i=0; input[i]; i++) {
        if( (input[i] == ' ')
            || (input[i] == '\t')
            || (input[i] == '\r')
            ) { // Handle all whitespace, \r ESPECIALLY!
            input[i] = '\0';
            i++; // Increment into next field
            *next = &(input[i]);

            return token;
        }
    }

    // Hit last field
    if(i>0) {
        *next = NULL;
        return token;
    }

    return NULL;
}



void setup() {
    Serial.begin(115200);
}

void loop() {
    char buf[SERIAL_BUFFER_SIZE];
    InputComponent *input = NULL;

    // Check Serial
    if(Serial.available() > 0) {
        int i;
        char* cmd;
        char* args;
        String str = Serial.readStringUntil('\n');

        str.toCharArray(buf, (SERIAL_BUFFER_SIZE - 1));
        buf[SERIAL_BUFFER_SIZE] = '\0'; // NULL terminate, just in case

        cmd = pop_token(buf, &args);
        if(cmd) {
            for(i = 0; command[i].cmd_name != 0; i++) {
                if(strcasecmp(cmd, command[i].cmd_name) == 0)
                    break;
            }
        }

        if(command[i].cmd_name == 0) {
            Serial.println("ERR Command not found");
            Serial.flush();
        } else {
            char* output = (command[i].cmd_func)(args);

            if(output) {
                Serial.println(output);
                Serial.flush();
            }
        }
    }

    // Check Inputs
    for(input=inputs[0]; input; input++) {
        if(input->poll()) {
            input->getMessage(buf);
            Serial.println(buf);
            Serial.flush();
        }
    }


    delay(10);
}

char* com_prot_ident   (char* args) {
    return panel->id;
}

char* com_prot_ping    (char* args) {
    return "PONG";
}

char* com_prot_set     (char* args) {
    return "TODO";
}

char* com_prot_get     (char* args) {
    return "ACK";
}
