#ifndef Panel_h
#define Panel_h
#include "Arduino.h" 


/*
 * Input Types 
 */
enum IOMethodType { 
    iomt_input, 
    iomt_input_pullup, 
    iomt_output, 
    iomt_both 
};

class IOMethod {
    public:
        // virtual void init();
        IOMethod() {};
        virtual void set(bool) = 0; 
        virtual bool poll() = 0;
};

class DirectIOMethod: public IOMethod {
    public:
        DirectIOMethod(int pin, IOMethodType type) : IOMethod() {
            this->pin = pin;
            this->type = type;

            if (this->type == iomt_input) {
                pinMode(this->pin, INPUT);
            }
            if (this->type == iomt_input_pullup) {
                pinMode(this->pin, INPUT_PULLUP);
            }
            if (this->type == iomt_output) {
                pinMode(this->pin, OUTPUT);
            }
        }

        bool poll() {
            bool new_state = false;
            
            if(digitalRead(this->pin) == HIGH) 
                new_state = true;

            if(this->type == iomt_input_pullup)
                return !new_state; // Reverse state
            else
                return new_state;
        }

        void set(bool state) {
            if(state) {
                digitalWrite(this->pin, HIGH);
            }else{
                digitalWrite(this->pin, LOW);
            }
        }

    protected:
        int pin;
        IOMethodType type;
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
