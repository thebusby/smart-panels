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

module render_power(pos){
    render_panel(pos, 200.0){
        
        column_a = 40.0;
        column_b = 80.0;
        column_c = 120.0;
        column_d = 160.0;
        
        row_one=65.0;
        row_two=180.0;
        
        // Top Row
        render_missile_toggle([row_one, column_a]); // PS-ON
        render_missile_toggle([row_one, column_b]); // 12V  
        render_missile_toggle([row_one, column_c]); // 12V 
        render_missile_toggle([row_one, column_d]); // ???
        
        render_fxb([125.0, 140.0]); // 12V - Motor Drivers
        // render_fxb([125.0, 80.0]); // 12V - Motor Drivers
        
        for(i=[0:4]) {
            render_led([ (92.0 + (i * 12.0)), column_a], 3);
        }
        
        // Redrum Suspend-Wake Button
        render_square_red_guard([row_two, column_d]); 
        render_small_toggle([row_two, column_a]);
        render_small_toggle([row_two, column_b]);
        render_small_toggle([row_two, column_c]);   
    }
}

module render_tray(pos){
    render_panel(pos, 100.0){
        
        
        
        translate([220.0, 0]){
        rotate([0, 0, 90]){
            
            column_a=25.0;
            column_b=50.0;
            column_c=75.0;
            
            row_one=100.0;
            row_two=135.0;
            
            // Manual Control
            render_small_toggle([30.0, 35.0]);
            
            // Normal Open
            render_cherrymx([70.0, 35.0], true);
            
            // Move speed pot
            render_pot([50.0, 65.0]);
            
            // Memory
            render_cherrymx([column_a, row_one], true);
            render_cherrymx([column_b, row_one], true);
            render_cherrymx([column_c, row_one], true);
            
            // Emergency Open
            render_emergency_stop([50.0, 165.0]);
            
            // Driver EN+ lines
            render_small_toggle([column_a, row_two]);
            render_small_toggle([column_c, row_two]);
         }
     }
    }
}



// Render final
// if(true)
    render_power([0,0]);
// else
    render_tray([0,200.1]);