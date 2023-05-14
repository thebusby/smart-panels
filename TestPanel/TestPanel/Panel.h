#ifndef Panel_h
#define Panel_h
#include "Arduino.h" 
#include "PCF8575.h"


// Usage notes;
// 1. Each IOMethod used should have IOMethod.setup() called during setup()
// 2. update_tc() should be called at start of each loop()



// 
// Define and handle global "Tick Counter"
// 
static uint32_t GLOBAL_TC = 0;

void update_tc() {
    GLOBAL_TC = millis();
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
            if (this->_type == iomt_input) {
                pinMode(this->_pin, INPUT);
            }
            if (this->_type == iomt_input_pullup) {
                pinMode(this->_pin, INPUT_PULLUP);
            }
            if (this->_type == iomt_output) {
                pinMode(this->_pin, OUTPUT);
            }

            return true;
        }

        bool read() {
            bool new_state = false;
            
            if(digitalRead(this->_pin) == HIGH) 
                new_state = true;

            if(this->_type == iomt_input_pullup)
                return !new_state; // Reverse state
            else
                return new_state;
        }

        void write(bool state) {
            if(state) {
                digitalWrite(this->_pin, HIGH);
            }else{
                digitalWrite(this->_pin, LOW);
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
        MyPCF8575(PCF8575 module) {
            this->_module = module;
            this->_tc = 0;
            this->_dataIn = 0;
            this->_dataOut = 0xFFFF;
        }

        // Components call this to set default state
        bool setup_pin(uint_8_t pin, bool state) {
            if (state)
                _dataOut |= (1 << pin);
            else
                _dataOut &= ~(1 << pin);

            return true;
        }

        // Panel calls this after all compononents
        bool setup(uint16_t mask) {
            _module.write16(_dataOut);
        }

        bool read(uint8_t pin) {
            
            // Refresh cache if necessary
            if( _tc != GLOBAL_TC ) {
                _dataIn = _module.read16();
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
                _module.write16(_dataOut);
            }
        }

    private:
        PCF8575 _module;
        uint32_t _tc;
        uint16_t _dataIn;
        uint16_t _dataOut;
};


class PCF8575IOMethod: public IOMethod {
    public:
        PCF8575IOMethod(MyPCF8575 module, int pin, IOMethodType type) : IOMethod() {
            this->_module = module;
            this->_pin = pin;
            this->_type = type;
        }

        void setup() {
            if (_type == iomt_input) {
                _module.setup_pin(_pin, false);
            }
            if (_type == iomt_input_pullup) {
                _module.setup_pin(_pin, true);
            }
            if (_type == iomt_output) {
                _module.setup_pin(_pin, false);
            }
        }

        bool read() {
            bool new_state = false;
            
            if(digitalRead(_pin) == HIGH) 
                new_state = true;

            if(this->type == iomt_input_pullup)
                return !new_state; // Reverse state
            else
                return new_state;
        }

        void write(bool state) {
            if(state) {
                digitalWrite(this->pin, HIGH);
            }else{
                digitalWrite(this->pin, LOW);
            }
        }

    private:
        MyPCF8575 _module;
        int _pin;
        IOMethodType _type;
};



/*
 * Components
 */

enum ComponentType { button_type, toggle_type, led_type, panel_type };

char* getCTypeName(ComponentType type) {
    switch (type) {
        case button_type:
            return "BTN";
        case toggle_type:
            return "TOG";
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
            this->method = method;
            this->state = false;
        }

        void set(bool state) {
            this->state = state;
            this->method->set(this->state);
        }

        void toggle() {
            this->set(!this->state);
        }

        void getMessage(char* buf) {
            char* state_string = this->state ? "ONN" : "OFF";
            sprintf(buf, "%s\t%s\t%s", this->id, getCTypeName(this->type), state_string);
        }

    protected:
        IOMethod *method;
        bool state;
};


class ToggleComponent: public InputComponent {
    public:
        ToggleComponent(char* id, IOMethod *method) : InputComponent(id, toggle_type) {
            this->method = method;
        }

        bool poll(){
            bool new_state = this->method->poll();
            if(this->state != new_state) {
                this->state = new_state;
                return true;
            }
            return false;
        }

        void getMessage(char* buf) {
            char* state_string = this->state ? "ONN" : "OFF";
            sprintf(buf, "%s\t%s\t%s", this->id, getCTypeName(this->type), state_string);
        }

    protected:
        IOMethod *method;
        bool state;
};


class ButtonComponent: public InputComponent {
    public:
        ButtonComponent(char* id, IOMethod *method) : InputComponent(id, button_type) {
            this->method = method;
        }

        bool poll(){
            bool new_state = this->method->poll();
            if(this->state != new_state) {
                this->state = new_state;
                return true;
            }
            return false;
        }

        void getMessage(char* buf) {
            char* state_string = this->state ? "ONN" : "OFF";
            sprintf(buf, "%s\t%s\t%s", this->id, getCTypeName(this->type), state_string);
        }

    protected:
        IOMethod *method;
        bool state;
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
};


#endif
