//
// Filthy hack because otherwise function's that use "MyInternalState" as a parameter will error
// https://forum.arduino.cc/t/struct-not-defined-in-this-scope/66566
//

// 
// Maintain state
// 
struct my_internal_state {

  // Displays
  unsigned long ssed[3];
  unsigned int  ssfd_now;

  // Alternate Display state
  unsigned int ssfd_tmp; // Used for setting alarms

  // Rotary encoder
  int re_pos = 0;

  // Switches
  bool rbs[3] = {1, 1, 1}; // RB's default to 1
  bool toggle_state[16]; // Only 10-15 is used as that's PB2-PB7 on the MCP

  // LED's
  // leds[x][y] where x is LED number, 
  // and y is color (red=0, green=1)
  int leds[3][2] = { {0, 0}, {0, 0}, {0, 0}};

  // alternate UI timer
  unsigned long tmp_timeout; // loop_count when tmp value should be tossed

  // Alarms
  unsigned int alarms[3] = {0, 0, 0};
};
typedef struct my_internal_state MyInternalState;

#define MCP_TOGGLE_MIN 10
#define MCP_TOGGLE_MAX 15



