// -*- mode: c -*-
/* All distances are in mm. */

// Set output quality
$fn= 64;

include <src/rack.scad> // Define common rack dimensions!
use <src/components.scad>
use <src/panels.scad>
use <src/utils.scad>


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      DEFINE
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

joystick_plate_height=120.0;
joystick_screw_radius=2.28; // 4.5mm radius?
joystick_x_offset=33.0;
joystick_y_offset=30.0;
joystick_screw_distance=60.1;
joystick_radius=(110 / 2);

throttle_plate_height=280.0;
throttle_x_offset=66.0;
throttle_y_offset=10.0;
throttle_dims=[142+2, 258+2]; // 258mm is too long by ~20mm!!!


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START PANELS
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

module render_bottom(pos){
    translate(pos) {            
        dims=[rack_inside_width, joystick_plate_height];
      
        difference() {
            color([0,0,1]) // To help debug with overlay
            square(dims ,center=false);

            // Render rack holes
            render_rect_holes([rack_screw_offset,rack_screw_offset]
                ,(rack_inside_width - 20.0)
                ,(joystick_plate_height - 20.0)
                ,rack_screw_radius);    

            // Render joystick mount holes
            render_rect_holes([joystick_x_offset, joystick_y_offset]
                ,joystick_screw_distance
                ,joystick_screw_distance
            ,joystick_screw_radius);

            // DEBUG
            echo("Screw center is x="
                ,(joystick_x_offset + (joystick_screw_distance / 2)) 
                ," y="
                ,(joystick_y_offset + (joystick_screw_distance / 2)));
        }
    }
}


module render_top(pos){
    render_panel(pos, joystick_plate_height) {            
    
        // Render space for joystick to come through
        // 20.0 is to ignore the 2cm over the 2020 frame
        render_hole([(20.0 + joystick_x_offset + (joystick_screw_distance / 2))
            ,(joystick_y_offset + (joystick_screw_distance / 2))]
            ,joystick_radius);
    
        // DEBUG
        echo("Hole center is x="
            ,(joystick_x_offset + (joystick_screw_distance / 2))
            , " y="
            , (joystick_y_offset + (joystick_screw_distance / 2)));

        // Render additional controls
        x_base=160.0;
        render_small_toggle([x_base, 99.0]);
        render_small_toggle([x_base+40.0, 99.0]);    
        render_red_button([x_base+20.0, 62.0]);
        render_re_ky040([x_base+20.0, 25.0]);        
    }
}

module render_throttle(pos){
    render_panel(pos, 280){
        
        // Add slot for throttle
        translate([throttle_x_offset, throttle_y_offset]) {
            square(throttle_dims ,center=false);
        }
        
        // Add some controls to the left
        x_base=40.0;
        render_led([x_base, 15], 5.0);
        render_small_toggle([x_base, 40.0]);
        render_small_toggle([x_base, 140.0]);
        render_small_toggle([x_base, 240.0]);
        
    }
}


// To compare top vs bottom for alignment
if(true)
if(false){ 
    // render_bottom([20.0, (joystick_plate_height + 2.0)]);
    render_bottom([20.0, 0, -30.0]);
    render_top([0,0]);
}else{
    
    // FOR PRODUCTION PRINT
    if(true) {
        // Cut with black 3mm acrylic
        // 120x200mm
        render_top([0,0]);
    
    }else{
        // Cut with transparent 5mm acrylic
        // 120x240mm
        render_bottom([0, 0]);
    }
}


// Render throttle mount
if(false)
    render_throttle([0,0]);
