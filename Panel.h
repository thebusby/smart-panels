#ifndef Panel_h
#define Panel_h
#include "Arduino.h"
#include "version.h"


#define SERIAL_BUFFER_SIZE 64  // 64 - "\r\n"



//
// Utility definitions
//
char* pop_token(char*, char**);



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

  if (HIGHBIT_MASK(t) == HIGHBIT_MASK(GLOBAL_TC)) {
    return GLOBAL_TC > t;

  } else {
    unsigned long diff = t - GLOBAL_TC;

    if (diff > (1 << 30))
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
  IOMethod(){};

  // For called during setup phase
  virtual bool setup() = 0;

  // Read/Write wire status
  virtual bool read() = 0;
  virtual void write(bool) = 0;
};


/*
 * Microcontroller's Onboard IO
 */
class DirectIOMethod : public IOMethod {
public:
  DirectIOMethod(int pin, IOMethodType type)
    : IOMethod() {
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

    if (digitalRead(_pin) == HIGH)
      new_state = true;

    if (_type == iomt_input_pullup)
      return !new_state;  // Reverse state
    else
      return new_state;
  }

  void write(bool state) {
    if (state) {
      digitalWrite(_pin, HIGH);
    } else {
      digitalWrite(_pin, LOW);
    }
  }

private:
  uint8_t _pin;
  IOMethodType _type;
};


#ifdef PCF8575_SUPPORT
#include "PCF8575.h"  // For PCF8575 IO expander

/*
 * PCF8575 Expansion IO
 */
class MyPCF8575 {
public:
  MyPCF8575(PCF8575* module) {
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
    _module->begin(_dataOut);

    return true;
  }

  bool read(uint8_t pin) {

    // Refresh cache if necessary
    if (_tc != GLOBAL_TC) {
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

    if (newOut != _dataOut) {
      _dataOut = newOut;
      _module->write16(_dataOut);
    }
  }

private:
  PCF8575* _module;
  uint32_t _tc;
  uint16_t _dataIn;
  uint16_t _dataOut;
};


class PCF8575IOMethod : public IOMethod {
public:
  PCF8575IOMethod(MyPCF8575* module, int pin, IOMethodType type)
    : IOMethod() {
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

    if (_module->read(_pin))
      new_state = true;

    if (_type == iomt_input_pullup)
      return !new_state;  // Reverse state
    else
      return new_state;
  }

  void write(bool state) {
    _module->write(_pin, state);
  }

private:
  MyPCF8575* _module;
  int _pin;
  IOMethodType _type;
};

#endif // #ifdef PCF8575_SUPPORT


/*
 * Components
 */

enum ComponentType {
  button_type,
  toggle_type,
  switch_type,
  encoder_type,
  led_type,
  rgbled_type,
  loglcd_type,
  panel_type
};

char* getCTypeName(ComponentType type) {
  switch (type) {
    case button_type:
      return "BTN";
    case toggle_type:
      return "TOG";
    case switch_type:
      return "SWC";
    case encoder_type:
      return "ENC";
    case led_type:
      return "LED";
    case rgbled_type:
      return "RGBLED";
    case loglcd_type:
      return "LOGLCD";
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

class InputComponent : public Component {
public:
  InputComponent(char* id, ComponentType type)
    : Component(id, type){};

  virtual bool poll() = 0;
};

class OutputComponent : public Component {
public:
  OutputComponent(char* id, ComponentType type)
    : Component(id, type){};

  virtual char* set(char*) = 0;
  virtual void update() = 0;
};


//
// Component implementations
//

class LedComponent : public OutputComponent {
public:
  LedComponent(char* id, IOMethod* method)
    : OutputComponent(id, led_type) {
    this->_method = method;
    this->_state = false;
  }

  char* set(char* args) {
    char* state_str;
    char* params;
    bool new_state;
    bool state_change = false;

    state_str = pop_token(args, &params);
    if (state_str) {
      if (strcasecmp(state_str, "ONN") == 0) {
        new_state = true;
        state_change = true;
      }
      if (strcasecmp(state_str, "OFF") == 0) {
        new_state = false;
        state_change = true;
        _flash_timer = 0; /* Disable Flashing */
      }
      if (strcasecmp(state_str, "TOG") == 0) {
        toggle();
        return "ACK";
      }
      if (strcasecmp(state_str, "FLASH") == 0) {
        int fv = 0;
        char* param_value;

        param_value = pop_token(params, NULL);
        if (param_value) {
          fv = atoi(param_value);
        } else
          fv = 1000; /* Defalut to 1 second */

        if (fv) {
          _flash_interval = fv;
          _flash_timer = get_tc_alert(_flash_interval);
        } else
          return "ERR LED FLASH value not handled";

        new_state = true;
        state_change = true;
      }
    }

    if (!state_change)
      return "ERR LED SET only takes ONN or OFF";

    if (_state != new_state) {
      _state = new_state;
      _method->write(_state);
    }

    return "ACK";
  }

  void toggle() {
    _state = !_state;
    _method->write(_state);
  }

  void update() {
    if (!_flash_timer
        || !is_tc_alert(_flash_timer))
      return;

    toggle();
    _flash_timer = get_tc_alert(_flash_interval);
  }

  char* get_state() {
    char* state_string = _state ? "ONN" : "OFF";
    if (_flash_timer)
      state_string = "FLASH";

    return state_string;
  }

  void getMessage(char* buf) {
    sprintf(buf, "%s\t%s\t%s", id, getCTypeName(type), get_state());
  }

  bool setup() {
    _method->setup();
    return true;
  }

  void disable() {
    _flash_timer = 0;

    if (_state)
      toggle();
  }

private:
  IOMethod* _method;
  bool _state;
  tick _flash_timer = 0;
  uint32_t _flash_interval = 0;
};


//
// LCD20X4_SUPPORT
//
#ifdef LCD20X4_SUPPORT

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


class LCD20X4Component : public OutputComponent {
public:
  LCD20X4Component(char* id, uint8_t i2c_address)
    : OutputComponent(id, loglcd_type) {
      this->_lcd = new LiquidCrystal_I2C(i2c_address, 20, 4);
  }

  char* set(char* args) {
    char* line_num;
    char* pos;
    uint8_t pos_num;
    uint8_t lcd_line;
    char* params;
    char* output;

    line_num = pop_token(args, &params);
    if (!line_num)
      return "ERR SET wanted line num";

    // Clear the entire screen
    if(strcasecmp(line_num, "CLR") == 0) 
    {
      _lcd->clear();

      return "ACK";
    }

    // Handle backlight
    if(strcasecmp(line_num, "LIGHT") == 0) 
    {
      if(strcasecmp(params, "ONN") == 0) {
        _backlight = true;
      }else{
        if(strcasecmp(params, "OFF") == 0) {
          _backlight = false;
        }else{
          // Toggle Backlight
          _backlight = !_backlight;
        }  
      }      

      if(_backlight) {
        _lcd->backlight();
      } else {
        _lcd->noBacklight();
      }

      return "ACK";
    }

    // Treat it as a one digit line number
    switch (line_num[0]) {
      case '1':
        lcd_line = 0;
        break;
      case '2':
        lcd_line = 1;
        break;
      case '3':
        lcd_line = 2;
        break;
      case '4':
        lcd_line = 3;
        break;
      default:
        return "ERR SET invalid line num";
    }

    pos = pop_token(params, &output);
    if (!pos)
      return "ERR SET needs line pos after line num";

    // Clear the entire line
    if(strcasecmp(pos, "CLR") == 0) 
    {
      printTxt(lcd_line, 0, "                    "); // Write 20 spaces

      return "ACK";
    }

    // Determine position (by 2) in the line
    pos_num = atoi(pos);
    if (!((pos_num >= 0) && (pos_num < 20)))
      return "ERR SET line pos not between 0-19";

    printTxt(lcd_line, pos_num, output);

    return "ACK";
  }

  void update() {
    // We do nothing on update()
  }

  void getMessage(char* buf) {
    sprintf(buf, "%s\t%s", id, getCTypeName(type));
  }

  bool setup() {
    _lcd->init();
    // _lcd->backlight(); // Enable backlight by default?
    _backlight = true;  

    return true;
  }

private:
   LiquidCrystal_I2C* _lcd;
   bool _backlight;

  void printTxt(uint8_t line, uint8_t pos, char* str) {
    _lcd->setCursor(pos, line);
    _lcd->print(str);
  }
};


#endif // #ifdef LCD20X4_SUPPORT


//
// ST7920_SUPPORT
//
#ifdef ST7920_SUPPORT

#include <SPI.h>  // For HW SPI support for ST7920

// ST7920 Commands
#define LCD_CLS 0x01
#define LCD_HOME 0x02
#define LCD_ADDRINC 0x06
#define LCD_DISPLAYON 0x0C
#define LCD_DISPLAYOFF 0x08
#define LCD_CURSORON 0x0E
#define LCD_CURSORBLINK 0x0F
#define LCD_BASIC 0x30
#define LCD_EXTEND 0x34
#define LCD_GFXMODE 0x36
#define LCD_TXTMODE 0x34
#define LCD_STANDBY 0x01
#define LCD_SCROLL 0x03
#define LCD_SCROLLADDR 0x40
#define LCD_ADDR 0x80
#define LCD_LINE0 0x80
#define LCD_LINE1 0x90
#define LCD_LINE2 0x88
#define LCD_LINE3 0x98

// #define SPI_SPEED (1000000UL)
// Much more reliable at half speed!
#define SPI_SPEED (500000UL)


class ST7920Component : public OutputComponent {
public:
  ST7920Component(char* id, uint8_t cs_pin)
    : OutputComponent(id, loglcd_type) {
      this->_cs_pin = cs_pin;
      // 12, // clock
      // 11, // data
      // 10, // CS
      // 8); // reset
  }

  char* set(char* args) {
    char* line_num;
    char* pos;
    uint8_t pos_num;
    uint8_t lcd_pos;
    char* params;
    char* output;

    line_num = pop_token(args, &params);
    if (!line_num)
      return "ERR SET wanted line num";

    switch (line_num[0]) {
      case '1':
        lcd_pos = LCD_LINE0;
        break;
      case '2':
        lcd_pos = LCD_LINE1;
        break;
      case '3':
        lcd_pos = LCD_LINE2;
        break;
      case '4':
        lcd_pos = LCD_LINE3;
        break;
      default:
        return "ERR SET invalid line num";
    }

    pos = pop_token(params, &output);
    if (!pos)
      return "ERR SET needs line pos after line num";

    // Clear the entire line
    if(strcasecmp(pos, "CLR") == 0) 
    {
      printTxt(lcd_pos, "                ");

      return "ACK";
    }

    // Determine position (by 2) in the line
    pos_num = atoi(pos);
    if (!((pos_num >= 0) && (pos_num < 16)))
      return "ERR SET line pos not between 0-15";

    printTxt((lcd_pos + pos_num), output);

    return "ACK";
  }

  void update() {
    // We do nothing on update()
  }

  void getMessage(char* buf) {
    sprintf(buf, "%s\t%s", id, getCTypeName(type));
  }

  bool setup() {
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, LOW);
    SPI.begin();
    sendCmd(LCD_BASIC);
    sendCmd(LCD_BASIC);
    sendCmd(LCD_CLS);
    delay(2);
    sendCmd(LCD_ADDRINC);
    sendCmd(LCD_DISPLAYON);

    // setGfxMode(true);

    // Enable TEXT mode
    sendCmd(LCD_EXTEND);
    sendCmd(LCD_TXTMODE);

    return true;
  }

private:
  uint8_t _cs_pin;

  void sendCmd(byte b) {
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs_pin, HIGH);
    SPI.transfer(0xF8);
    SPI.transfer(b & 0xF0);
    SPI.transfer(b << 4);
    digitalWrite(_cs_pin, LOW);
    SPI.endTransaction();
  }
  // ----------------------------------------------------------------

  void sendData(byte b) {
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE3));
    digitalWrite(_cs_pin, HIGH);
    SPI.transfer(0xFA);
    SPI.transfer(b & 0xF0);
    SPI.transfer(b << 4);
    digitalWrite(_cs_pin, LOW);
    SPI.endTransaction();
  }

  void printTxt(uint8_t pos, char* str) {
    sendCmd(LCD_BASIC);
    sendCmd(pos);
    while (*str) sendData(*str++);
  }

  // For supporting non-ASCII characters
  void printTxt(uint8_t pos, uint16_t* signs) {
    sendCmd(LCD_BASIC);
    sendCmd(pos);
    while (*signs) {
      sendData(*signs >> 8);
      sendData(*signs & 0xff);
      signs++;
    }
  }
};

#endif // #ifdef ST7920_SUPPORT

class RGBLedComponent : public OutputComponent {
public:
  RGBLedComponent(char* id, IOMethod* red_method, IOMethod* green_method, IOMethod* blue_method)
    : OutputComponent(id, rgbled_type) {
    this->_red = new LedComponent("RED", red_method);
    this->_green = new LedComponent("GREEN", green_method);
    this->_blue = new LedComponent("BLUE", blue_method);
  }

  char* set(char* args) {
    uint8_t i;
    LedComponent* leds[] = { _red, _green, _blue, 0 };
    char* color_name;
    char* params;

    color_name = pop_token(args, &params);

    if (!color_name)
      return "ERR SET wanted color name or OFF";

    if (strcasecmp(color_name, "OFF") == 0) {
      _active->disable();
      _active = NULL;

      return "ACK";
    }

    /* Find the color */
    for (i = 0; leds[i]; i++)
      if (strcasecmp(color_name, leds[i]->id) == 0) {
        if (_active && _active != leds[i])
          _active->disable();

        _active = leds[i];
        return _active->set(params);
      }

    return "ERR failed to find color indicated";
  }

  void update() {
    if (_active)
      _active->update();
  }

  void getMessage(char* buf) {

    // Early exit if nothing going on
    if (!_active) {
      sprintf(buf, "%s\t%s\t%s", id, getCTypeName(type), "OFF");
      return;
    }

    sprintf(buf, "%s\t%s\t%s\t%s", id, getCTypeName(type), _active->id, _active->get_state());
  }

  bool setup() {
    _red->setup();
    _green->setup();
    _blue->setup();

    return true;
  }

private:
  LedComponent* _active = NULL;

  LedComponent* _red;
  LedComponent* _green;
  LedComponent* _blue;
};

class EncoderComponent : public InputComponent {
public:
  EncoderComponent(char* id, IOMethod* clk, IOMethod* dt)
    : InputComponent(id, encoder_type) {
    this->_clk = clk;
    this->_dt = dt;
  }

  bool poll() {
    _dir = NULL;

    _currentStateCLK = _clk->read();

    if ((_currentStateCLK != _lastStateCLK)
        && _currentStateCLK) {

      if (_dt->read() != _currentStateCLK) {
        _counter++;
        _dir = "RIGHT";
      } else {
        _counter--;
        _dir = "LEFT";
      }
    }
    _lastStateCLK = _currentStateCLK;

    return (_dir != NULL);
  }

  void getMessage(char* buf) {
    sprintf(buf, "%s\t%s\t%s\t%d", id, getCTypeName(type), _dir, _counter);
  }

  bool setup() {
    _clk->setup();
    _dt->setup();

    return true;
  }

private:
  IOMethod* _clk;
  IOMethod* _dt;
  bool _currentStateCLK = false;
  bool _lastStateCLK = false;
  int _counter = 0;
  char* _dir = NULL;
};

class ToggleComponent : public InputComponent {
public:
  ToggleComponent(char* id, IOMethod* method)
    : InputComponent(id, toggle_type) {
    this->_method = method;
  }

  bool poll() {
    bool new_state = _method->read();
    if (_state != new_state) {
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
  IOMethod* _method;
  bool _state;
};


class ButtonComponent : public InputComponent {
public:
  ButtonComponent(char* id, IOMethod* method)
    : InputComponent(id, button_type) {
    this->_method = method;
  }

  bool poll() {
    bool new_state = _method->read();
    if (_state != new_state) {
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
  IOMethod* _method;
  bool _state;
};


class SwitchComponent : public InputComponent {
public:
  SwitchComponent(char* id, IOMethod** methods)
    : InputComponent(id, switch_type) {
    this->_methods = methods;
  }

  bool poll() {
    uint8_t new_active = find_active();

    // Ignore case we're in the middle of rotating the switch
    if (new_active == 0xFF)
      return false;

    if (new_active != _active) {
      _active = new_active;
      return true;
    }
    return false;
  }

  void getMessage(char* buf) {
    sprintf(buf, "%s\t%s\t%d", id, getCTypeName(type), _active);
  }

  bool setup() {
    uint8_t i;
    for (i = 0; _methods[i]; i++) {
      _methods[i]->setup();
    }

    return true;
  }

private:
  uint8_t find_active() {
    uint8_t i;

    for (i = 0; _methods[i]; i++)
      if (_methods[i]->read())
        break;

    if (!_methods[i])
      return 0xFF;

    return i;
  }

  IOMethod** _methods = NULL;
  uint8_t _active = 0xFF;
};



//
// Define a panel
//
class Panel : public Component {
public:
  InputComponent** inputs;
  OutputComponent** outputs;
  char buf[SERIAL_BUFFER_SIZE];

  Panel(char* id, InputComponent** inputs, OutputComponent** outputs)
    : Component(id, panel_type) {
    this->inputs = inputs;
    this->outputs = outputs;
  }

  void getMessage(char* buf) {
    sprintf(buf, "%s\t%s", this->id, getCTypeName(this->type));
  }

  bool setup() {
    int i;

    for (i = 0; inputs[i]; i++)
      if (!inputs[i]->setup())
        return false;

    for (i = 0; outputs[i]; i++)
      if (!outputs[i]->setup())
        return false;

    return true;
  }

  bool loop();
};



/*
 * Handle general panel logic
*/
typedef struct cmd {
  char* cmd_name;
  char* (*cmd_func)(Panel*, char*);
} cmd_t;

/* Protocol handlers */
char* com_prot_ident(Panel*, char*);
char* com_prot_version(Panel*, char*);
char* com_prot_desc(Panel*, char*);
char* com_prot_ping(Panel*, char*);
char* com_prot_set(Panel*, char*);
char* com_prot_get(Panel*, char*);

/* List of commands */
cmd_t command[] = {
  { "IDENT", com_prot_ident },
  { "VERSION", com_prot_version },
  { "PING", com_prot_ping },
  { "DESC", com_prot_desc },
  { "SET", com_prot_set },
  { "GET", com_prot_get },
  { 0 }
};

/* Tokenize space delimited text */
char* pop_token(char* input, char** next) {
  int i;
  char* token = input;

  for (i = 0; input[i]; i++) {
    if ((input[i] == ' ')
        || (input[i] == '\t')
        || (input[i] == '\r')) {  // Handle all whitespace, \r ESPECIALLY!
      input[i] = '\0';

      i++;  // Increment into next field
      if (next)
        *next = &(input[i]);

      return token;
    }
  }

  // Hit last field
  if (i > 0) {
    if (next)
      *next = NULL;
    return token;
  }

  return NULL;
}




bool Panel::loop() {
  uint8_t i;

  // Increment Tick counter
  tc_update();

  // Check Inputs
  for (i = 0; inputs[i]; i++) {
    if (inputs[i]->poll()) {
      sprintf(buf, "EVENT\t");
      inputs[i]->getMessage(buf + 6);
      Serial.println(buf);
      Serial.flush();
    }
  }

  // Check Outputs for auto-state changes
  for (i = 0; outputs[i]; i++)
    outputs[i]->update();

  // Check Serial
  if (Serial.available() > 0) {
    char* cmd;
    char* args;
    String str = Serial.readStringUntil('\n');

    str.toCharArray(buf, (SERIAL_BUFFER_SIZE - 1));
    buf[(SERIAL_BUFFER_SIZE - 1)] = '\0';  // NULL terminate, just in case

    cmd = pop_token(buf, &args);
    if (cmd) {
      for (i = 0; command[i].cmd_name != 0; i++) {
        if (strcasecmp(cmd, command[i].cmd_name) == 0)
          break;
      }
    }

    if (command[i].cmd_name == 0) {
      Serial.println("ERR Command not found");
      Serial.flush();

    } else {
      char* output = (command[i].cmd_func)(this, args);

      if (output) {
        Serial.println(output);
        Serial.flush();
      }
    }
  }

  delay(10);
}


char* com_prot_ident(Panel* panel, char* args) {
  return panel->id;
}

char* com_prot_version(Panel* panel, char* args) {
  snprintf(panel->buf, (SERIAL_BUFFER_SIZE-3), "VER\t%s\t%s", BUILD_NUMBER, BUILD_DATE);
  Serial.println(panel->buf);
  Serial.flush();
  return "ACK";
}

char* com_prot_ping(Panel* panel, char* args) {
  return "PONG";
}

char* com_prot_desc(Panel* panel, char* args) {
  uint8_t i;

  // Describe inputs
  for (i = 0; panel->inputs[i]; i++) {
    panel->inputs[i]->getMessage(panel->buf);
    Serial.println(panel->buf);
    Serial.flush();
  }

  // Describe outputs
  for (i = 0; panel->outputs[i]; i++) {
    panel->outputs[i]->getMessage(panel->buf);
    Serial.println(panel->buf);
    Serial.flush();
  }

  return "ACK";
}

char* com_prot_set(Panel* panel, char* args) {
  uint8_t i;
  char* comp_name = NULL;
  char* params = NULL;

  comp_name = pop_token(args, &params);
  if (comp_name) {
    for (i = 0; panel->outputs[i]; i++)
      if (strcasecmp(comp_name, panel->outputs[i]->id) == 0)
        break;
  } else {
    return "ERR\tComponent name not found in SET command";
  }

  if (panel->outputs[i])
    return panel->outputs[i]->set(params);
  else
    return "ERR\tComponent not found in SET command";
}

char* com_prot_get(Panel* panel, char* args) {
  uint8_t i;
  char* comp_name = NULL;
  char* params = NULL;

  comp_name = pop_token(args, &params);
  if (comp_name) {
    for (i = 0; panel->inputs[i]; i++)
      if (strcasecmp(comp_name, panel->inputs[i]->id) == 0)
        break;
  } else {
    return "ERR\tComponent name not found in GET command";
  }

  if (panel->inputs[i]) {
    panel->inputs[i]->getMessage(panel->buf);
    Serial.println(panel->buf);
    Serial.flush();

  } else
    return "ERR\tComponent not found in GET command";

  return "ACK";
}

#endif
