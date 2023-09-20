// -*- mode: c -*-
/* All distances are in mm. */

// Set output quality
$fn= 64;

// -- Set Library preferences --

// Sets whether CherryMX keys should use notched holes
use_notched_holes = true;

// Imports
include <src/rack.scad> // Define common rack dimensions!
use <src/components.scad>
use <src/panels.scad>
use <src/utils.scad>


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      DEFINE
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

keys_plate_height=100.0;
status_plate_height=100.0;


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START PANELS
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

module render_keys(pos){
    render_panel(pos, keys_plate_height) {
        row_y_init=35.0;
        row_y_inc=45.0;
        key_x_init=80.0;
        key_x_inc=21.0;
        
        for(j=[0:1]) {
            for(i=[0:4]) {
                render_cherrymx([(key_x_init+(i*key_x_inc)),
                (row_y_init+(j*row_y_inc))], 
                use_notched_holes);
                }
            render_re_ky040([200.0, (row_y_init+(j*row_y_inc))]);
        }
        
        // Top row toggle
        render_small_toggle([45.0, (row_y_init+(1*row_y_inc))]);
        
        // Bottom row Rotary switch
        render_bw_rot_switch([45.0, (row_y_init+(0*row_y_inc))]);
        
        // State LED
        render_led([164.0, 57.0], 5.0);
        
    }
}

module render_giant_lcd(pos){
    glcd_screw_radius=1.6;
    
    translate(pos){
        
        // Screw holes
        render_hole([2.9, 3.1], glcd_screw_radius);
        render_hole([90.8,3.1], glcd_screw_radius);
        render_hole([90.8, 68.1], glcd_screw_radius);
        render_hole([2.9, 68.1], glcd_screw_radius);
        
        // Display
        translate([7.5, 9.5]){
            square([78.5, 51.7]);
        }
    }
}

module render_status_plate(pos){
    acrylic_width=3.05;
    slat_length=60.0;
    slat_start_y=20.0;
    slat_width_outer=180.0;
    slat_indent=10.0;
    slat_bracket_size=20.0;
    slat_depth=18.0;
    slat_width_inner=(slat_width_outer - (acrylic_width * 2));
    wire_hole_radius=22.0;
    support_height=180.0;
    plate_notch_length=83.5;
    
    // Render Slats
    render_panel(pos, status_plate_height) {
        translate([(20.0 + slat_indent), slat_start_y]){
            square([acrylic_width, slat_length]);
        }
        translate([(240.0 - (20.0 + acrylic_width + slat_indent)), slat_start_y]){
            square([acrylic_width, slat_length]);
        }
    
        // Opening for wires
        translate([120.0, 55.0]) {
            circle(wire_hole_radius);
        }    
    }
    
    // Triangle supports 1
    translate([(pos[0]), (pos[1] + status_plate_height + 10.0)]){
        difference(){
            // square([200.0, (slat_length + 20.0 + 10.0)]);
            polygon(points=[
            [0,0],
            [(slat_depth + support_height), 0],
            [slat_depth, (slat_length + (slat_bracket_size*2))],
            [0,(slat_length + (slat_bracket_size*2))] 
            ], paths = undef, convexity = 10);
            
            // Mount Notches
            square([slat_depth, slat_bracket_size]);
            translate([0,(slat_length+slat_bracket_size)]){
                square([slat_depth, slat_bracket_size]);
            }
            
            // Lock Notches
            translate([(slat_depth - ((acrylic_width*2) + 0.5)),slat_bracket_size]){
                square([acrylic_width, (slat_length / 2)]);
            }
            
            // Plate Notches
            translate([(slat_depth - 5), (slat_length + (slat_bracket_size*1.5) + 1.0)]){
                rotate(-30){
                    square([plate_notch_length,acrylic_width]);
                }
            }
            
            // Knock off top, too sharp
            polygon([
            [(slat_depth + support_height), 0],
            [((slat_depth + support_height) - 40.0), 0],
            [((slat_depth + support_height) - 40.0), 23.0],            
            ]);           
        }
    }
    
    // Triangle supports 2
    translate([(pos[0] + 230), (pos[1] + status_plate_height + 110.0)]){
        rotate(180)
        difference(){
            // square([200.0, (slat_length + 20.0 + 10.0)]);
            polygon(points=[
            [0,0],
            [(slat_depth + support_height), 0],
            [slat_depth, (slat_length + (slat_bracket_size*2))],
            [0,(slat_length + (slat_bracket_size*2))] 
            ], paths = undef, convexity = 10);
            
            // Mount Notches
            square([slat_depth, slat_bracket_size]);
            translate([0,(slat_length+slat_bracket_size)]){
                square([slat_depth, slat_bracket_size]);
            }
            
            // Lock Notches
            translate([(slat_depth - ((acrylic_width*2) + 0.5)),slat_bracket_size]){
                square([acrylic_width, (slat_length / 2)]);
            }
            
            // Plate Notches
            translate([(slat_depth - 5), (slat_length + (slat_bracket_size*1.5) + 1.0)]){
                rotate(-30){
                    square([plate_notch_length,acrylic_width]);
                }
            }
            
            // Knock off top, too sharp
            polygon([
            [(slat_depth + support_height), 0],
            [((slat_depth + support_height) - 40.0), 0],
            [((slat_depth + support_height) - 40.0), 23.0],            
            ]);           
        }
    }
    
    // Lock Plates
    translate([(pos[0] + 245.0), 0]){
        difference(){
            square([20, slat_length]);
            translate([(10 - acrylic_width),0]){
            square([acrylic_width, (slat_length / 2)]);
            }
        }
    }
    translate([(pos[0] + 245.0), (slat_length + 10)]){
        difference(){
            square([20, slat_length]);
            translate([(10 - acrylic_width),0]){
                square([acrylic_width, (slat_length / 2)]);
            }
        }
    }
    
    // Top Plate
    plate_height=160.0;
    plate_width=200.0;
    
    translate([(pos[0] + 20.0), 220.0]){
        difference(){
            square([plate_width, plate_height]);
            
            // Mount Slats
            translate([10,(plate_height - plate_notch_length)]){
                square([acrylic_width, plate_notch_length]);
            }
            translate([(200.0 - (acrylic_width + slat_indent))
            ,(plate_height - plate_notch_length)]){
                square([acrylic_width, plate_notch_length]);
            }
            
            // Components
            render_giant_lcd([87.0, 85.0]);
 
            render_cherrymx([70.0, 100.0], true);
            render_cherrymx([70.0, 120.0], true);
            render_cherrymx([70.0, 140.0], true);
                
            render_re_ky040([40.0, 140.0]);
            render_small_toggle([40.0, 100.0]);
        
            
            // Bottom Components
            bottom_y=35.0;
            bottom_x_inc=25.0;
            for(i=[0:4]) {
                render_led([(35.0 + (bottom_x_inc * i)), (bottom_y + 17.0)], 3.0);
                render_small_toggle([(35.0 + (bottom_x_inc * i)), bottom_y]);
            }
            render_led([175.0, bottom_y], 5.0);
                  
        }
    }
}


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      EXECUTE
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 


// B&W Rot Switch TEST
if(false){
    difference(){
        square([12,20], center=false);
        render_bw_rot_switch([6,6]);
    }    
}

// Giant LCD TEST
if(false){
    difference(){
        square([100,88], center=false);
        render_giant_lcd([0,0]);
    }
}

// Render Keys
if(false)
    render_keys([0,0]);


// Render Status Board
if(true) {
    render_status_plate([0,0]);
}
