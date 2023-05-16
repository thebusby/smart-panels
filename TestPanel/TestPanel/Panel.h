#ifndef Panel_h
#define Panel_h
#include "Arduino.h" 
#include "PCF8575.h"

#define SERIAL_BUFFER_SIZE 64 // 64 - "\r\n"



// 
// Define and handle global "Tick Counter"
// 

// Define new type, which is milliseconds counter
// pulled from Arduino millis()
typedef uint32_t tick;

tick GLOBAL_TC = 0;

void tc_update() {
    GLOBAL_TC = millis();
}

tick get_tc_alert(unsigned long millis) {
    return GLOBAL_TC + millis;
}

#define HIGHBIT_MASK(x) ((x & (1 << 31)) > 0)
bool is_tc_alert(tick t) {

    if( HIGHBIT_MASK(t) == HIGHBIT_MASK(GLOBAL_TC) ) {
        return GLOBAL_TC > t;
        
    }else{
        unsigned long diff = t - GLOBAL_TC;

        if(diff > (1 << 30))
            return !HIGHBIT_MASK(t);
        else
            return HIGHBIT_MASK(t);
    }
}


/*
 * Input Types 
 */
enum IOMethodType { 
    iomt_input, 
    iomt_input_pullup, 
    iomt_output 
};

class IOMethod {
    public:
        // Constructor 
        IOMethod() {};

        // For called during setup phase
        virtual bool setup() = 0;

        // Read/Write wire status
        virtual bool read() = 0; 
        virtual void write(bool) = 0; 
};


/*
 * Microcontroller's Onboard IO
 */
class DirectIOMethod: public IOMethod {
    public:
        DirectIOMethod(int pin, IOMethodType type) : IOMethod() {
            this->_pin = pin;
            this->_type = type;
        }

        bool setup() {
            if (_type == iomt_input) {
                pinMode(_pin, INPUT);
            }
            if (_type == iomt_input_pullup) {
                pinMode(_pin, INPUT_PULLUP);
            }
            if (_type == iomt_output) {
                pinMode(_pin, OUTPUT);
            }

            return true;
        }

        bool read() {
            bool new_state = false;
            
            if(digitalRead(_pin) == HIGH) 
                new_state = true;

            if(_type == iomt_input_pullup)
                return !new_state; // Reverse state
            else
                return new_state;
        }

        void write(bool state) {
            if(state) {
                digitalWrite(_pin, HIGH);
            }else{
                digitalWrite(_pin, LOW);
            }
        }

    private:
        uint8_t _pin;
        IOMethodType _type;
};


/*
 * PCF8575 Expansion IO
 */
class MyPCF8575 {
    public:
        MyPCF8575(PCF8575 *module) {
            this->_module = module;
            this->_tc = 0;
            this->_dataIn = 0;
            this->_dataOut = 0xFFFF;
        }

        // Components call this to set default state
        bool setup_pin(uint8_t pin, bool state) {
            if (state)
                _dataOut |= (1 << pin);
            else
                _dataOut &= ~(1 << pin);

            return true;
        }

        // Panel calls this after all compononents
        bool setup() {
            _module->write16(_dataOut);

            return true;
        }

        bool read(uint8_t pin) {
            
            // Refresh cache if necessary
            if( _tc != GLOBAL_TC ) {
                _dataIn = _module->read16();
                _tc = GLOBAL_TC;
            }

            return (_dataIn & (1 << pin)) > 0;
        }

        void write(int pin, bool state) {
            uint16_t newOut = _dataOut;

            if (state)
                newOut |= (1 << pin);
            else
                newOut &= ~(1 << pin);

            if(newOut != _dataOut) {
                _dataOut = newOut;
                _module->write16(_dataOut);
            }
        }

    private:
        PCF8575 *_module;
        uint32_t _tc;
        uint16_t _dataIn;
        uint16_t _dataOut;
};


class PCF8575IOMethod: public IOMethod {
    public:
        PCF8575IOMethod(MyPCF8575 *module, int pin, IOMethodType type) : IOMethod() {
            this->_module = module;
            this->_pin = pin;
            this->_type = type;
        }

        bool setup() {
            if (_type == iomt_input) {
                _module->setup_pin(_pin, false);
            }
            if (_type == iomt_input_pullup) {
                _module->setup_pin(_pin, true);
            }
            if (_type == iomt_output) {
                _module->setup_pin(_pin, false);
            }

            return true;
        }

        bool read() {
            bool new_state = false;
            
            if(_module->read(_pin)) 
                new_state = true;

            if(_type == iomt_input_pullup)
                return !new_state; // Reverse state
            else
                return new_state;
        }

        void write(bool state) {
            _module->write(_pin, state);
        }

    private:
        MyPCF8575 *_module;
        int _pin;
        IOMethodType _type;
};



/*
 * Components
 */

enum ComponentType { 
    button_type, 
    toggle_type, 
    switch_type,
    led_type, 
    panel_type };

char* getCTypeName(ComponentType type) {
    switch (type) {
        case button_type:
            return "BTN";
        case toggle_type:
            return "TOG";
        case switch_type:
            return "SWC";
        case led_type:
            return "LED";
        case panel_type:
            return "PNL";
        default:
            return "NIL";
    }
}

class Component {
    public:
        char* id;
        ComponentType type;

        virtual void getMessage(char*) = 0;
        virtual bool setup() = 0;

        Component(char* id, ComponentType type) {
            this->id = id;
            this->type = type;
        }
};

class InputComponent: public Component {
    public:
        InputComponent(char* id, ComponentType type) : Component(id, type) {};

        // Indicates whether a state change exists
        virtual bool poll() = 0;
};

class OutputComponent: public Component {
    public:
        OutputComponent(char* id, ComponentType type) : Component(id, type) {};

        // For now, enables/disables a pin
        virtual void set(bool) = 0;
};


//
// Component implementations
//

class LedComponent: public OutputComponent {
    public:
        LedComponent(char* id, IOMethod *method) : OutputComponent(id, led_type) {
            this->_method = method;
            this->_state = false;
        }

        void set(bool state) {
            _state = state;
            _method->write(_state);
        }

        void toggle() {
            set(!_state);
        }

        void getMessage(char* buf) {
            char* state_string = _state ? "ONN" : "OFF";
            sprintf(buf, "%s\t%s\t%s", id, getCTypeName(type), state_string);
        }

        bool setup() {
            _method->setup();
            return true;
        }

    private:
        IOMethod *_method;
        bool _state;
};


class ToggleComponent: public InputComponent {
    public:
        ToggleComponent(char* id, IOMethod *method) : InputComponent(id, toggle_type) {
            this->_method = method;
        }

        bool poll(){
            bool new_state = _method->read();
            if(_state != new_state) {
                _state = new_state;
                return true;
            }
            return false;
        }

        void getMessage(char* buf) {
            char* state_string = _state ? "ONN" : "OFF";
            sprintf(buf, "%s\t%s\t%s", id, getCTypeName(type), state_string);
        }

        bool setup() {
            _method->setup();
            return true;
        }

    private:
        IOMethod *_method;
        bool _state;
};


class ButtonComponent: public InputComponent {
    public:
        ButtonComponent(char* id, IOMethod *method) : InputComponent(id, button_type) {
            this->_method = method;
        }

        bool poll(){
            bool new_state = _method->read();
            if(_state != new_state) {
                _state = new_state;
                return true;
            }
            return false;
        }

        void getMessage(char* buf) {
            char* state_string = _state ? "ONN" : "OFF";
            sprintf(buf, "%s\t%s\t%s", id, getCTypeName(type), state_string);
        }

        bool setup() {
            _method->setup();
            return true;
        }

    private:
        IOMethod *_method;
        bool _state;
};



//
// Define a panel
//
class Panel: public Component {
    public:
        InputComponent **inputs;
        OutputComponent **outputs;

        Panel(char* id, InputComponent** inputs, OutputComponent** outputs): Component(id, panel_type) {
            this->inputs = inputs;
            this->outputs = outputs;
        }

        void getMessage(char* buf) {
            sprintf(buf, "%s\t%s", this->id, getCTypeName(this->type));
        }

        bool setup() {
            int i;

            for(i=0; inputs[i]; i++)
                if(!inputs[i]->setup())
                    return false;

            for(i=0; outputs[i]; i++)
                if(!outputs[i]->setup())
                    return false;

            return true;
        }

        bool loop();
};



/*
 * Handle general panel logic
*/
typedef struct cmd {
  char *cmd_name;
  char* (*cmd_func)(Panel*, char*);
} cmd_t;

/* Protocol handlers */
char* com_prot_ident   (Panel*, char*);
char* com_prot_desc    (Panel*, char*);
char* com_prot_ping    (Panel*, char*);
char* com_prot_set     (Panel*, char*);
char* com_prot_get     (Panel*, char*);

/* List of commands */
cmd_t command[] = {
  { "IDENT",    com_prot_ident },
  { "PING",     com_prot_ping },
  { "DESC",     com_prot_desc },
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
            // if(input[i] != '\n') // This is handled by NULL termination of string
                *next = &(input[i]);
            // else
            //    *next = NULL; // Return NULL if we hit \r\n

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




bool Panel::loop() {
    char buf[SERIAL_BUFFER_SIZE];
    InputComponent *input = NULL;

    // Increment Tick counter
    tc_update();

    // Check Serial
    if(Serial.available() > 0) {
        int i;
        char* cmd;
        char* args;
        String str = Serial.readStringUntil('\n');

        str.toCharArray(buf, (SERIAL_BUFFER_SIZE - 1));
        buf[(SERIAL_BUFFER_SIZE - 1)] = '\0'; // NULL terminate, just in case

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
            char* output = (command[i].cmd_func)(this, args);

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


char* com_prot_ident   (Panel* panel, char* args) {
    return panel->id;
}

char* com_prot_ping    (Panel* panel, char* args) {
    return "PONG";
}

char* com_prot_desc    (Panel* panel, char* args) {
    return "DESC";
}

char* com_prot_set     (Panel* panel, char* args) {
    return "TODO";
}

char* com_prot_get     (Panel* panel, char* args) {
    return "ACK";
}

#endif
