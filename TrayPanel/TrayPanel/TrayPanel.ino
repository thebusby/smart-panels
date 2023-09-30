#include "Panel.h"

#include <AccelStepper.h>
#include <MultiStepper.h>

// Define a stepper and the pins it will use
//
AccelStepper stepper_left(AccelStepper::DRIVER, A0, A1); // LEFT
AccelStepper stepper_right(AccelStepper::DRIVER, A2, A3); // RIGHT
MultiStepper steppers;

// Define maximum number of steps per second
#define STEPPER_MAX_SPEED 1000

// Number of milliseconds between poll events
#define POLL_INTERVAL 60000

/*
 * Define Panel Specific Values here
 */

InputComponent* inputs[] =
{
new ButtonComponent("FORWARD", new DirectIOMethod(6, iomt_input)),
new ButtonComponent("BACK", new DirectIOMethod(7, iomt_input)),
new ToggleComponent("BIG_RED", new DirectIOMethod(8, iomt_input)),
new ButtonComponent("BUTTON_0", new DirectIOMethod(9, iomt_input)),
new ButtonComponent("BUTTON_1", new DirectIOMethod(10, iomt_input)),
new ButtonComponent("BUTTON_2", new DirectIOMethod(11, iomt_input)),
new ButtonComponent("BUTTON_3", new DirectIOMethod(12, iomt_input)),
new ToggleComponent("TOG_PC", new DirectIOMethod(A4, iomt_input)),
new PotComponent("SPEED", new DirectIOMethod(A6, iomt_input)),
NULL
};


OutputComponent* outputs[] = 
{
NULL
};

Panel *panel = new Panel("TRAY_PANEL", inputs, outputs);

void setup() {  
    Serial.begin(115200);

    // Initialize panel
    if(!panel->setup()) {
      Serial.println("ERR Panel init failed, do something!");
      Serial.flush();
    }
 
  // Change these to suit your stepper if you want
  stepper_left.setMaxSpeed(STEPPER_MAX_SPEED);
  stepper_right.setMaxSpeed(STEPPER_MAX_SPEED);
  
  steppers.addStepper(stepper_left);
  steppers.addStepper(stepper_right);
}

void loop() {
  static tick timer=0;

  panel->loop();


  //   long dist = 3000;
  //   long positions[2] = {dist, (dist * -1)};

  //   steppers.moveTo(positions);
  //   steppers.runSpeedToPosition();
  //   delay(2000);

  //   positions[0] = (dist * -1);
  //   positions[1] = dist;
    
  //   steppers.moveTo(positions);
  //   steppers.runSpeedToPosition();
  //   delay(2000);

  // Handle custom panel behavour 
  // if(is_tc_alert(timer)) {
  //     uint16_t t = dht->get_temp();
  //     uint16_t h = dht->get_humidity();
  //     uint16_t c = mhz19->get_co2();

  //     ssfd_tmp->display_digit(0, 12); // Add C to display
  //     ssfd_tmp->display_digits(t, 0);

      // ssfd_hum->display_digit(0, 'H'); // This doesn't work yet...
  //     ssfd_hum->display_digits(h, 0);

  //     ssfd_co2->display_digits(c, 1);

  //     timer = get_tc_alert(POLL_INTERVAL);
  //   }
}