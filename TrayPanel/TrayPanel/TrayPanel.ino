#include "Panel.h"

#include <EEPROM.h> // Store 3 memory positions in EEPROM
#include <AccelStepper.h>
#include <MultiStepper.h>


// Define a stepper and the pins it will use
//
AccelStepper stepper_left(AccelStepper::DRIVER, A0, A1); // LEFT
AccelStepper stepper_right(AccelStepper::DRIVER, A2, A3); // RIGHT
MultiStepper steppers;

// Define stepper positions for each memory button
// 
int32_t button_1_pos;
int32_t button_2_pos;
int32_t button_3_pos;

// Define an emergency exit
//
bool emergency_halt = false;

// Define maximum number of steps per second
#define STEPPER_MAX_SPEED 3300

// Define memory address of Button positions in EEPROM
#define BUTTON_1_EEPROM_MEM_ADDR 0
#define BUTTON_2_EEPROM_MEM_ADDR 4 // sizeof(int32_t) * 1
#define BUTTON_3_EEPROM_MEM_ADDR 8 // sizeof(int32_t) * 2

// Define how long a memory button should be held down to store
// the position in memory, versus activate the movement
#define BUTTON_LONG_PRESS_MS 3000 // 3 seconds


/*
 * Define Panel Specific Values here
 */
ToggleComponent* bigred_button  = new ToggleComponent("BIG_RED",  new DirectIOMethod(8, iomt_input_pullup));
ButtonComponent* forward_button = new ButtonComponent("FORWARD",  new DirectIOMethod(7, iomt_input_pullup));
ButtonComponent* back_button    = new ButtonComponent("BACK",     new DirectIOMethod(6, iomt_input_pullup));
PotComponent*    speed_pot      = new PotComponent("SPEED",       new DirectIOMethod(A6, iomt_input));
ButtonComponent* button_0       = new ButtonComponent("BUTTON_0", new DirectIOMethod(9, iomt_input_pullup));
ButtonComponent* button_1       = new ButtonComponent("BUTTON_1", new DirectIOMethod(12, iomt_input_pullup));
ButtonComponent* button_2       = new ButtonComponent("BUTTON_2", new DirectIOMethod(10, iomt_input_pullup));
ButtonComponent* button_3       = new ButtonComponent("BUTTON_3", new DirectIOMethod(11, iomt_input_pullup));


InputComponent* inputs[] =
{
forward_button,
back_button,
bigred_button,
button_0,
button_1,
button_2,
button_3,
new ToggleComponent("TOG_PC", new DirectIOMethod(A4, iomt_input_pullup)),
speed_pot,
NULL
};


OutputComponent* outputs[] = 
{
NULL
};

Panel *panel = new Panel("TRAY_PANEL", inputs, outputs);


// Interrupt function to activate when tray hits the terminator switches
//
void terminator() {

  // Enable emergency halt
  emergency_halt = true;
  
  // Reset position
  stepper_left.setCurrentPosition(0);
  stepper_right.setCurrentPosition(0);

  // Stop motors
  stepper_left.move(0);
  stepper_right.move(0);
}


// Microcontroller setup code 
//
void setup() {  
  Serial.begin(115200);

  // Initialize panel
  if(!panel->setup()) {
    Serial.println("ERR Panel init failed, do something!");
    Serial.flush();
  }
 
  // Register terminator switches!
  attachInterrupt(digitalPinToInterrupt(2), terminator, CHANGE); // Left side
  // attachInterrupt(digitalPinToInterrupt(3), terminator, CHANGE);
  emergency_halt = false;

  // Init steppers, and add them to group
  stepper_left.setMaxSpeed(STEPPER_MAX_SPEED);
  stepper_right.setMaxSpeed(STEPPER_MAX_SPEED);
  steppers.addStepper(stepper_left);
  steppers.addStepper(stepper_right);

  // Pull memory button values from EEPROM
  EEPROM.get(BUTTON_1_EEPROM_MEM_ADDR, button_1_pos);
  EEPROM.get(BUTTON_2_EEPROM_MEM_ADDR, button_2_pos);
  EEPROM.get(BUTTON_3_EEPROM_MEM_ADDR, button_3_pos);
}


// Main loop
//
void loop() {
  static int16_t speed=0; // Store motor speed through every iteration
 
  // Check panel basics on every loop
  panel->loop();

  // Handle speed changes
  if(1) {
    int16_t new_speed = STEPPER_MAX_SPEED * speed_pot->getValue();
 
    if(new_speed != speed) {
      speed = new_speed;
      stepper_left.setMaxSpeed(speed);
      stepper_right.setMaxSpeed(speed);
    }
  }

  // Handle Forward button
  if(forward_button->getValue()) {
    int32_t dist = 1000000;
    int32_t positions[2] = {dist, (dist * -1)};
    
    steppers.moveTo(positions);

    // Do this while the button is held down
    while(!forward_button->poll() && !emergency_halt) {
      steppers.run();
    }
    emergency_halt = false;
  }

  // Handle Back button
  if(back_button->getValue()) {
    int32_t dist = 1000000;
    int32_t positions[2] = {(dist * -1), dist};
    
    steppers.moveTo(positions);

    // Do this while the button is held down
    while(!back_button->poll() && !emergency_halt) {
      steppers.run();
    }
    emergency_halt = false;
  }

  // Emergency Button
  // Activates when it turns OFF
  if(!bigred_button->getValue()) {
    int32_t dist = 1000000;
    int32_t positions[2] = {(dist * -1), dist};
    
    // Set MAX speed for motors
    stepper_left.setMaxSpeed(STEPPER_MAX_SPEED);
    stepper_right.setMaxSpeed(STEPPER_MAX_SPEED);
  
    steppers.moveTo(positions);

    // Do this while the button is off
    while(!bigred_button->poll() && !emergency_halt) {
      steppers.run();
      // Keep doing run() until the button is unlocked
      // Alternatively, terminator switch and interrupt may stop us as well.
    }
    emergency_halt = false;
  }

  // Handle Button 0 (Open)
  if(button_0->getValue()) {
    int32_t positions[2] = {50, 50}; // Leave a little room off the terminator switch if possible
    steppers.moveTo(positions);

    while(steppers.run() && !emergency_halt) {
        // Keep doing run() until run() returns false meaning we're in position
        // Alternatively, terminator switch and interrupt may stop us as well.
    }
    emergency_halt = false;
  }

  // Handle Button 1 (Memory)
  if(button_1->getValue()) {
    uint32_t start = millis();
    uint32_t dur = 0;

    while(!button_1->poll()) {
      // Doing nothing waiting to see how long button is held down
    }
    dur = millis() - start;

    if( dur >= BUTTON_LONG_PRESS_MS) {
      // Long press, let's record the current position in memory
      button_1_pos = stepper_left.currentPosition();
      EEPROM.put(BUTTON_1_EEPROM_MEM_ADDR, button_1_pos);

    }else{
      // Short press, let's make the motors move there
      int32_t positions[2] = {button_1_pos, (button_1_pos * -1)};
    
      steppers.moveTo(positions);

      while(steppers.run() && !emergency_halt) {
        // Keep doing run() until run() returns false meaning we're in position
        // Alternatively, terminator switch and interrupt may stop us as well.
      }
      emergency_halt = false;
    }
  }

  // Handle Button 2 (Memory)
  if(button_2->getValue()) {
    uint32_t start = millis();
    uint32_t dur = 0;

    while(!button_2->poll()) {
      // Doing nothing waiting to see how long button is held down
    }
    dur = millis() - start;

    if( dur >= BUTTON_LONG_PRESS_MS) {
      // Long press, let's record the current position in memory
      button_2_pos = stepper_left.currentPosition();
      EEPROM.put(BUTTON_2_EEPROM_MEM_ADDR, button_2_pos);

    }else{
      // Short press, let's make the motors move there
      int32_t positions[2] = {button_2_pos, (button_2_pos * -1)};
    
      steppers.moveTo(positions);

      while(steppers.run() && !emergency_halt) {
        // Keep doing run() until run() returns false meaning we're in position
        // Alternatively, terminator switch and interrupt may stop us as well.
      }
      emergency_halt = false;
    }
  }

  // Handle Button 3 (Memory)
  if(button_3->getValue()) {
    uint32_t start = millis();
    uint32_t dur = 0;

    while(!button_3->poll()) {
      // Doing nothing waiting to see how long button is held down
    }
    dur = millis() - start;

    if( dur >= BUTTON_LONG_PRESS_MS) {
      // Long press, let's record the current position in memory
      button_3_pos = stepper_left.currentPosition();
      EEPROM.put(BUTTON_3_EEPROM_MEM_ADDR, button_3_pos);

    }else{
      // Short press, let's make the motors move there
      int32_t positions[2] = {button_3_pos, (button_3_pos * -1)};
    
      steppers.moveTo(positions);

      while(steppers.run() && !emergency_halt) {
        // Keep doing run() until run() returns false meaning we're in position
        // Alternatively, terminator switch and interrupt may stop us as well.
      }
      emergency_halt = false;
    }
  }

  // end loop()
}
