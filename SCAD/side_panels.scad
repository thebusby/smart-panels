// -*- mode: c -*-
/* All distances are in mm. */

// Set output quality
$fn=50;


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START RACK SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

// Frame dimensions;
// 300mm width, with 25mm framing on each side, mounting bolts at 12.5mm in.
// 50mm for each 1u, with bolts spaced centered every 12.5mm
rack_width=240.0;
rack_inner_width=220.0;
rack_inner_screw_width=205.0;
rack_height_per_u=50.0;
rack_frame_width=25.0;
rack_screw_x=10;
rack_screw_y_offset=10;
rack_screw_y=10.0;
rack_screw_radius=3.55; // M5?




// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                       END RACK SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START PART SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

// Screw hole size (M4 size)
screw_hole_radius = 3;

// distance between edge and screw hole
screw_offset = 10;

// 20x4 LCD specs
lcd_screw_x = 93;
lcd_screw_y = 55;
lcd_screw_radius = 1.28; // M2.6?
lcd_screw_xstart = 2.5;
lcd_screw_ystart = 1.6;
lcd_screen_x = 97; // Spec said 98mm, my measurements say 97
lcd_screen_y = 40; // Spec said 42mm, my measurements say 40
lcd_screen_xstart = 0;
lcd_screen_ystart = 9;

// Rotary encoder specs HW-040
//re_board_x=18.5
//re_board_y=26
// Positions for TOP LEFT corner of PCB board
// This means some Y values should be negative in practice!!!
re_screw_x=0.6 + 1.5; // center screw hole
re_screw_y=-5;
re_screw2_x=0.6 + 1.5; // center screw hole
re_screw2_y=-19;
re_screw_radius=1.5; // M3, = 3mm diameter / 2
re_x=10;
re_y=10;
re_radius=3.5;

// Rotary encoder specs KY-040
//re2_board_x=18.0
//re2_board_y=26.0
// Positions for TOP LEFT corner of PCB board
// This means some Y values should be negative in practice!!!
//re2_screw_x=1.0 + 1.5; // center screw hole
re2_screw_x=0.0 + 1.5; // center screw hole

re2_screw_y=-4;
//re2_screw2_x=1.0 + 1.5; // center screw hole
re2_screw2_x=0.0 + 1.5; // center screw hole

re2_screw2_y=-21;
re2_screw_radius=1.5; // M3, = 3mm diameter / 2
re2_x=10;
re2_y=10;
re2_radius=3.5;

// Sodial Red Button specs
red_button_radius=8; // 7.5mm by spec, 7.8mm by measure with screw threads

// Locking toggle (8E2011)
// 1/4 inch diameter?
small_toggle_radius=(6.36 / 2);

// Seven Segment - 2 Digit
sstd_x=15.0;
sstd_y=13.5;

// Seven Segment - 4 Digit
ssfd_x=42.0;
ssfd_y=24.0;
ssfd_screw_x = 38.0;
ssfd_screw_y = 20.0;
ssfd_screw_radius = 1.2; // M2.4?
ssfd_screw_xstart = 1.0;
ssfd_screw_ystart = 2.0;
ssfd_screen_x = 30.2; // Spec said 98mm, my measurements say 97
ssfd_screen_y = 14.4; // Spec said 42mm, my measurements say 40
ssfd_screen_xstart = 6.4;
ssfd_screen_ystart = 5.2;
// ssfd_x=30.0; // 47mm for frame
// ssfd_y=14.0; // 19mm for frame

// Seven Segment - 8 Digit
// TODO: This needs to include mounting screws!!!
ssed_frame_x=82.4; // 82.4,15.2
ssed_frame_y=15.2;
// ssed_frame_offset=9.0;
ssed_screw_xstart=2.5;
ssed_screw_ystart=2.5;
ssed_screw_x=76.0;
ssed_screw_y=9.0;
ssed_screw_radius=1.8; // M3.6 -> M4 ?
ssed_screen_xstart=12.2;
ssed_screen_ystart=0.4;
ssed_screen_x=61.6;
ssed_screen_y=14.2;


// Arduino Nano Mount Spec
//
arnano_mount_screw_radius=1.5;
arnano_mount_xstart=2.0;
arnano_mount_ystart=2.0;
arnano_mount_x=40.0;
arnano_mount_y=0.0;



module render_rack_screw_hole(position){
    translate(position) {
        circle(rack_screw_radius);
    }
}

module render_rack_screw_holes(position){
    translate(position) {
        circle(rack_screw_radius);
    }
    translate([(position[0]+rack_inner_screw_width), position[1]]) {
        circle(rack_screw_radius);
    }
}

module render_sstd(position){
   translate(position) {
           square([sstd_x, sstd_y]);
   }
}

module render_ssfd(position){
    translate(position) {
       
       // Four screw holes
       translate([ssfd_screw_xstart, ssfd_screw_ystart]){
           circle(ssfd_screw_radius);
       }
       translate([(ssfd_screw_xstart + ssfd_screw_x), ssfd_screw_ystart]){
           circle(ssfd_screw_radius);
       }
       translate([(ssfd_screw_xstart + ssfd_screw_x), (ssfd_screw_ystart + ssfd_screw_y)]){
           circle(ssfd_screw_radius);
       }
       translate([ssfd_screw_xstart, (ssfd_screw_ystart + ssfd_screw_y)]){
           circle(ssfd_screw_radius);
       }
       
       // Render screen aread
       translate([ssfd_screen_xstart, ssfd_screen_ystart]){
           square([ssfd_screen_x, ssfd_screen_y]);
       }
   }
}

// TODO: This needs to include mounting screws!!!
module render_ssed(position){
   translate(position) {
       
       // Four screw holes
       translate([ssed_screw_xstart, ssed_screw_ystart]){
           circle(ssed_screw_radius);
       }
       translate([(ssed_screw_xstart + ssed_screw_x), ssed_screw_ystart]){
           circle(ssed_screw_radius);
       }
       translate([(ssed_screw_xstart + ssed_screw_x), (ssed_screw_ystart + ssed_screw_y)]){
           circle(ssed_screw_radius);
       }
       translate([ssed_screw_xstart, (ssed_screw_ystart + ssed_screw_y)]){
           circle(ssed_screw_radius);
       }
       
       // Render screen aread
       translate([ssed_screen_xstart, ssed_screen_ystart]){
           square([ssed_screen_x, ssed_screen_y]);
       }
   }
}

module render_circle(position, diameter) {
    translate(position) {
        circle( (diameter / 2) );
    }
}

module render_screw_hole(position){
    translate(position) {
        circle(screw_hole_radius);
    }
}

module render_lcd(position){
   translate(position) {
       // Four screw holes
       translate([lcd_screw_xstart, lcd_screw_ystart]){
           circle(lcd_screw_radius);
       }
       translate([(lcd_screw_xstart + lcd_screw_x), lcd_screw_ystart]){
           circle(lcd_screw_radius);
       }
       translate([(lcd_screw_xstart + lcd_screw_x), (lcd_screw_ystart + lcd_screw_y)]){
           circle(lcd_screw_radius);
       }
       translate([lcd_screw_xstart, (lcd_screw_ystart + lcd_screw_y)]){
           circle(lcd_screw_radius);
       }
       
       // Spot for display
       translate([lcd_screen_xstart,lcd_screen_ystart]){
           square([lcd_screen_x, lcd_screen_y]);
       }
   }
}

// Put position at center of RE dial - Model HW-040
module render_re(position){
    new_position = [(position[0] - re_x), 
                    (position[1] + re_y)];
    translate(new_position) {
        translate([re_screw_x, re_screw_y]) {
            circle(re_screw_radius);
        }
        translate([re_screw2_x, re_screw2_y]) {
            circle(re_screw_radius);
        }
        translate([re_x,-re_y]) {
            circle(re_radius);
        }
    }
}

// Put position at center of RE dial - Model KY-040
module render_re2(position){
    new_position = [(position[0] - re_x), 
                    (position[1] + re_y)];
    translate(new_position) {
        translate([re2_screw_x, re2_screw_y]) {
            circle(re2_screw_radius);
        }
        translate([re2_screw2_x, re2_screw2_y]) {
            circle(re2_screw_radius);
        }
        translate([re2_x,-re2_y]) {
            circle(re2_radius);
        }
    }
}

module render_red_button(position){
    translate(position) {
        circle(red_button_radius);
    }
}

module render_small_toggle (position){
    translate(position) {
        circle(small_toggle_radius);
    }
}

// Provide LED diameter as well
module render_led (position, diameter){
    translate(position) {
        circle( (diameter / 2) );
    }
}

module render_rot_switch(position){    
    translate([position[0], (position[1] + 12.5)]){
        square([3, 1.3] ,center=true);
    }
    render_circle(position, 8.9); 
}

module render_arduino_nano_mount(pos){
    translate(pos){
        translate([arnano_mount_xstart, arnano_mount_ystart]){
            circle(arnano_mount_screw_radius);
        }
        translate([(arnano_mount_xstart+arnano_mount_x), (arnano_mount_ystart+arnano_mount_y)]){
            circle(arnano_mount_screw_radius);
        }
    }
}

module render_arduino_nano_mount_vertical(pos){
    translate(pos){
        translate([arnano_mount_xstart, arnano_mount_ystart]){
            circle(arnano_mount_screw_radius);
        }
        translate([(arnano_mount_xstart+arnano_mount_y), (arnano_mount_ystart+arnano_mount_x)]){
            circle(arnano_mount_screw_radius);
        }
    }
}

// / / / / / / / / / / / / / / / /
//   Manufactoring Tests
// / / / / / / / / / / / / / / / /

module test_rot_switch(pos){
    dims=[15,24];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_rot_switch([7,7]);
        }
    }
}

module test_led(pos){
    dims=[20,10];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_led([4,5], 2);
            render_led([9,5], 3);
            render_led([15,5], 5);
        }
    }
}

module test_small_toggle(pos){
    dims=[10,10];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_small_toggle([5,5]);
        }
    }
}

module test_red_button(pos){
    dims=[20,20];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_red_button([10,10]);
        }
    }
}

module test_re(pos){
    dims=[18,26];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_re([12,15]);
        }
    }
}

module test_re2(pos){
    dims=[19,26];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_re2([11,14]);
        }
    }
}

module test_lcd(pos){
    dims=[102,65];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_lcd([2,2]);
        }
    }
}

module test_sstd(pos){
    dims=[20,20];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_sstd([2,2]);
        }
    }
}

module test_ssfd(pos){
    dims=[ssfd_x+6,ssfd_y+6];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_ssfd([3,3]);
        }
    }
}

module test_ssed(pos){
    dims=[82.4,19.2];
    
    translate(pos) {
        difference() {
            square(dims,center=false);            
            render_ssed([0,2]);
        }
    }
}

module test_rack(pos){
    dims=[14,14];
    
    translate(pos) {
        difference() {
            square(dims,center=false);
            render_rack_screw_hole([7,7]);
        }
    }
    
}

module test_arduino_nano_mount(pos){
    dims=[46,6];
    
    translate(pos) {
        difference() {
            square(dims,center=false);
            render_arduino_nano_mount([1,1]);
        }
    }   
}



// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                       END PART SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 


// Pin Count
// 
// LEFT [6]
// Temp Disp 2-P
// Humid Disp 1-P
// CO2 Disp 1-P
// CO2 Sensor 1-P
// Temp/Hum Sensor 1-P

// RIGHT [13]
// Wanted Temp Disp 2-P
// Rotary Encoder 2-P
// AC On/Off 1-P
// Lights On/Off 1-P
// Rotary Switch 5-P
// IR Light 1-P
// IR Recv  1-P

module render_panel_env(pos, console_u){
  dims=[rack_width, (rack_height_per_u * console_u)];
    
  translate(pos) {
    difference() {
    square(dims ,center=false);

    // Render rack holes
    render_rack_screw_holes([rack_screw_x, rack_screw_y_offset]);
    render_rack_screw_holes([rack_screw_x, (rack_screw_y_offset+(3*rack_screw_y))]);
    //for(screw_y=[rack_screw_y_offset:rack_screw_y:dims[1]]) {
    //    render_rack_screw_hole([rack_screw_x, screw_y]);
    //    render_rack_screw_hole([(dims[0] - rack_screw_x), screw_y]);                
    //}

    column2_offset=55;
    led_size=3.0;
    led_offset_x=rack_frame_width+9;
    led_offset_x2=led_offset_x+column2_offset; 
    column_offset_x=(rack_frame_width+20) - ssfd_screen_xstart;
    column_offset_x2=column_offset_x+column2_offset;
    column_offset_y=8;
    column_spacing=30;
    
    // Render Displays
    render_ssfd([column_offset_x, (dims[1] - (ssfd_y + column_offset_y + (column_spacing * 0)))]); // Temp
    render_ssfd([column_offset_x, (dims[1] - (ssfd_y + column_offset_y + (column_spacing * 1)))]); // Humid
    // render_ssfd([column_offset_x2, (dims[1] - (sstd_y + column_offset_y + (column_spacing * 0)))]); // CO2
    render_ssfd([column_offset_x, (dims[1] - (ssfd_y + column_offset_y + (column_spacing * 2)))]); // CO2
    // render_ssfd() // HEAT INDEX!!!!!!!!!!!!!

    //render_ssfd([column_offset_x2, (dims[1] - (ssfd_y + column_offset_y + (column_spacing * 1)))]); // Index    
    
    // Render Arduino Nano mount brack
    render_arduino_nano_mount([105, 48]);

    // Render warning lights
    render_led([led_offset_x, (dims[1] - ((ssfd_y/2) + column_offset_y + (column_spacing * 0)))], led_size);   
    render_led([led_offset_x, (dims[1] - ((ssfd_y/2) + column_offset_y + (column_spacing * 1)))], led_size);
    render_led([led_offset_x, (dims[1] - ((ssfd_y/2) + column_offset_y + (column_spacing * 2)))], led_size);    
    
    
    // render_led([led_offset_x2, (dims[1] - ((sstd_y/2) + column_offset_y + (column_spacing * 0)))], 3.0);
    // render_led([led_offset_x2, (dims[1] - ((sstd_y/2) + column_offset_y + (column_spacing * 1)))], 3.0);
    
    
    
    // For AC section
    //
    ac_column_offset_x=125 + column_offset_x;
    ac_column_offset_y=10;
    ac_top_row_control_offset=11;
    ac_row_offset_y=60;
    ac_right_column_offset_y=90;

    render_ssfd([((ac_column_offset_x - ssfd_screen_xstart) + 0) , (dims[1] - (ssfd_y + ac_column_offset_y))]); // AC Temp Display
    render_re([(ac_column_offset_x + 60), (dims[1] - (ac_column_offset_y + ac_top_row_control_offset))]); // AC Temp Control
    render_small_toggle([(ac_column_offset_x + ac_right_column_offset_y), (dims[1] - (ac_column_offset_y + ac_top_row_control_offset))]); // AC On/Off
    render_rot_switch([(ac_column_offset_x + 35), (dims[1] - (ac_column_offset_y + ac_row_offset_y))]); // A/C Mode
    render_small_toggle([(ac_column_offset_x + ac_right_column_offset_y), (dims[1] - (ac_column_offset_y + ac_row_offset_y))]); // Light Switch
    
    // Render Status LED's
    ac_led_y_offset=36.5;
    ac_led_x_offset=-10.0;
    ac_led_gap=10.0;
    render_led([(ac_column_offset_x + ac_right_column_offset_y+ ac_led_x_offset + (ac_led_gap*0)), (dims[1] - (ac_column_offset_y + ac_led_y_offset))], 3.0);
    render_led([(ac_column_offset_x + ac_right_column_offset_y+ ac_led_x_offset + (ac_led_gap*1)), (dims[1] - (ac_column_offset_y + ac_led_y_offset))], 3.0);
    render_led([(ac_column_offset_x + ac_right_column_offset_y+ ac_led_x_offset + (ac_led_gap*2)), (dims[1] - (ac_column_offset_y + ac_led_y_offset))], 3.0);
}
}  
}

module render_panel_snd(pos, console_u){
  dims=[rack_width, (rack_height_per_u * console_u)];
    
  translate(pos) {
    difference() {
    square(dims ,center=false);
           
    // Render rack holes
    render_rack_screw_holes([rack_screw_x, rack_screw_y_offset]);
    render_rack_screw_holes([rack_screw_x, (rack_screw_y_offset+(3*rack_screw_y))]);
    //for(screw_y=[rack_screw_y_offset:rack_screw_y:dims[1]]) {
    //    render_rack_screw_hole([rack_screw_x, screw_y]);
    //    render_rack_screw_hole([(dims[0] - rack_screw_x), screw_y]);                
    //}
    
    lcd_offset_x=rack_frame_width+10;
    render_lcd([lcd_offset_x, 20]);
    
    pt_col_one_x=160;
    pt_row_y=70;
    pt_gap_size=30;
    
    
    render_re([(pt_col_one_x+(pt_gap_size*0)), pt_row_y]);
    render_re([(pt_col_one_x+(pt_gap_size*1)), pt_row_y]);
    render_red_button([(pt_col_one_x+(pt_gap_size*2)), pt_row_y]);
    render_led([(pt_col_one_x+(pt_gap_size*2)+25.0), pt_row_y], 5.0);
    
    
    // Render Arduino Nano mount brack
    render_arduino_nano_mount([210, 30]);
    
    //render_red_button([(pt_col_one_x+(pt_gap_size*3)), pt_row_y]);
    
}}}



module render_panel_time(pos, console_u){
  dims=[rack_width, (rack_height_per_u * console_u)];
    
  translate(pos) {
    difference() {
    square(dims ,center=false);
           
    // Render rack holes
    render_rack_screw_holes([rack_screw_x, rack_screw_y_offset]);
    render_rack_screw_holes([rack_screw_x, (rack_screw_y_offset+(7*rack_screw_y))]);
    //for(screw_y=[rack_screw_y_offset:rack_screw_y:dims[1]]) {
    //    render_rack_screw_hole([rack_screw_x, screw_y]);
    //    render_rack_screw_hole([(dims[0] - rack_screw_x), screw_y]);                
    //}
    
    // Render main section
    main_col_offset_x=rack_frame_width+10;
    main_row_offset_y=dims[1]-30;
    
    led_base=2.0;
    led_gap=8.0;
    led_y_offset=12.5;
    led_x=22.5;
    // render_led([(main_col_offset_x+led_x), (main_row_offset_y-(led_base+(led_gap*0)))], 5.0);
    // render_led([(main_col_offset_x+led_x), (main_row_offset_y-(led_base+(led_gap*1)))], 5.0);
    // render_led([(main_col_offset_x+led_x), (main_row_offset_y-(led_base+(led_gap*2)))], 5.0);
    
    render_led([(main_col_offset_x+led_x+(led_gap*0)), (main_row_offset_y+led_y_offset)], 5.0);
    render_led([(main_col_offset_x+led_x+(led_gap*1)), (main_row_offset_y+led_y_offset)], 5.0);
    render_led([(main_col_offset_x+led_x+(led_gap*2)), (main_row_offset_y+led_y_offset)], 5.0);
    
    
    render_ssfd([(main_col_offset_x+10.0), (main_row_offset_y-22.0)]);
    // render_lcd([(main_col_offset_x+115.0), (main_row_offset_y-40.0)]);
    render_re([(main_col_offset_x+85.0), (main_row_offset_y-9.5)]);
    
    // Render Arduino Nano mount brack
    // render_arduino_nano_mount_vertical([206.0, (main_row_offset_y-125.0)]);
    render_arduino_nano_mount([(main_col_offset_x+115.0+10.0), (main_row_offset_y-20.0)]);
    
    // Render Alarm holes
    tarm_section_x_start=(main_col_offset_x+115.+90.0);
    tarm_section_y_start=(main_row_offset_y-20.0);
    tarm_hole_size=1.0;
    tarm_gap=1.5;
    tarm_grid_size=3;
    
    for(tarm_x=[tarm_section_x_start:tarm_gap:(tarm_section_x_start+(tarm_gap*tarm_grid_size))]) {
        for(tarm_y=[tarm_section_y_start:tarm_gap:(tarm_section_y_start+(tarm_gap*tarm_grid_size))]) {
            render_led([tarm_x, tarm_y], tarm_hole_size);
        }
    }
    // render_led([(tarm_section_x_start+(tarm_gap*0)),(tarm_section_y_start+(tarm_gap*0))], tarm_hole_size);
    
    
    // Render sub-section
    sub_col_offset_x=rack_frame_width+15;
    sub_col_y_start=30;
    sub_col_y_gap=40;
    for(i=[0,1,2]) {
        row_y=sub_col_y_start+(sub_col_y_gap*i);
        //render_small_toggle([(sub_col_offset_x+(sc_x_gap*0)), row_y]);
        sc_x_gap=15.0;
        sc_fudge_gap=13.0;
        render_ssed([(sub_col_offset_x+(sc_x_gap*0)), (row_y-7.5)]);
        //render_re([(sub_col_offset_x+ssed_frame_x+sc_fudge_gap+(sc_x_gap*1)+10), row_y]);
        render_red_button([(sub_col_offset_x+ssed_frame_x+sc_fudge_gap+(sc_x_gap*1)+10), row_y]);
        render_small_toggle([(sub_col_offset_x+ssed_frame_x+sc_fudge_gap+(sc_x_gap*2)+30), row_y]);
        render_small_toggle([(sub_col_offset_x+ssed_frame_x+sc_fudge_gap+(sc_x_gap*3)+40), row_y]);
        //render_red_button([(sub_col_offset_x+ssed_frame_x+sc_fudge_gap+(sc_x_gap*4)+45), row_y]);
    }
    
    
}}}

cut_width=5.05;
cut_length=250;
frame_height=500;
module render_frame_side(pos){
    mount_height=frame_height;
    mount_width_top=100;
    mount_width_bottom=45;
    coords=[ 
    [0.0, 0.0], 
    [0.0, mount_height], 
    [mount_width_top, mount_height], 
    [mount_width_bottom, 0.0] 
    ];
    
    difference() {
        polygon(coords);
        
        translate([20,cut_length]) {
            square([cut_width,cut_length],center=false);
        }

        for(i=[10,35,45,55,65,75,85]) {
            translate([i, (mount_height-10)]) {
                circle(2);
            }
        }   
        
        // Absolute positions for now
        translate([65,375]) {
            circle(2);
        }
        translate([55,250]) {
            circle(2);
        }
        translate([45,125]) {
            circle(2);
        }
        translate([35,10]) {
            circle(2);
        }
        translate([10,10]) {
            circle(2);
        }
        
    }

}

module render_frame_face(pos){
    face_height=500;
    cut_in_gap=25.0;
    face_width=(rack_width+(2*cut_width)+(2*cut_in_gap)+20.0);
    
    translate(pos){
        difference(){
            square([face_width, face_height]);
            
            // Cuts
            cut_in_gap=25.0;
            translate([cut_in_gap,0.0]){
                square([cut_width,cut_length],center=false);
            }
            translate([(face_width - (cut_in_gap+cut_width) ),0]){
                square([cut_width,cut_length],center=false);
            }
            
            // Inner section
            translate([((face_width - rack_inner_width) / 2), 25]){
                square([rack_inner_width, (face_height - 50.0)],center=false);
            }
           
           
            // Render rack holes
            rack_screw_frame_x_offset=((face_width - rack_width) / 2);
            rack_screw_frame_y_offset=25.0;
            for(screw_y=[(rack_screw_y_offset+rack_screw_frame_y_offset):rack_screw_y:(face_height - 25.0)]) {
                render_rack_screw_hole([(rack_screw_x+rack_screw_frame_x_offset), screw_y]);
                render_rack_screw_hole([(rack_screw_x+rack_screw_frame_x_offset+275.0), screw_y]);
                // echo(screw_y);
            }
            
        }
    }
}

module render_frame_face_side(pos){
    face_height=500;
    cut_in_gap=25.0;
    face_width=(rack_frame_width + cut_width + cut_in_gap + 10.0);
    
    translate(pos){
        difference(){
            square([face_width, face_height]);
            
            // Cuts
            cut_in_gap=25.0;
            translate([cut_in_gap,0.0]){
                square([cut_width,cut_length],center=false);
            }
           
            // Render rack holes
            rack_screw_frame_x_offset=40;
            rack_screw_frame_y_offset=0.0;
            for(screw_y=[(rack_screw_y_offset+rack_screw_frame_y_offset):rack_screw_y:(face_height - 0.0)]) {
                render_rack_screw_hole([(rack_screw_x+rack_screw_frame_x_offset), screw_y]);
                // echo(screw_y);
            }
            
        }
    }
}

module render_one_u_spacer_horiz(pos){
  dims=[rack_width, (rack_height_per_u * 1)];
    
  translate(pos) {
    difference() {
        square(dims ,center=false);
           
        // Render rack holes
        render_rack_screw_holes([rack_screw_x, rack_screw_y_offset]);
        render_rack_screw_holes([rack_screw_x, (rack_screw_y_offset+(1*rack_screw_y))]);
        }   
    }
}

module render_one_u_spacer_with_switch(pos){
  dims=[rack_width, (rack_height_per_u * 1)];
    
  translate(pos) {
    difference() {
    square(dims ,center=false);
           
    // Render rack holes
    render_rack_screw_holes([rack_screw_x, rack_screw_y_offset]);
    render_rack_screw_holes([rack_screw_x, (rack_screw_y_offset+(1*rack_screw_y))]);
    
    translate([25,0]) {
        
        render_led([10,25], 3.0);
        render_small_toggle([25, 25]);
        render_led([135,25], 3.0);
        render_small_toggle([150, 25]);
    }
    }   
}
}

module render_one_u_spacer(pos){
    dims=[(rack_height_per_u * 1), rack_width];
    
    translate(pos) {
        difference() {
        square(dims ,center=false);

        // Render rack holes
        render_rack_screw_hole([rack_screw_y_offset, rack_screw_x]);
        render_rack_screw_hole([(rack_screw_y_offset+(1*rack_screw_y)), rack_screw_x]);
        render_rack_screw_hole([rack_screw_y_offset, (rack_screw_x+rack_inner_screw_width)]);
        render_rack_screw_hole([(rack_screw_y_offset+(1*rack_screw_y)), (rack_screw_x+rack_inner_screw_width)]);                 
        }
    }
}

module render_panel_tabs(u_count){
    y_u=((u_count-1) * rack_height_per_u);

    // Left
    translate([10, (y_u + 5)]){
        square([10, 40] ,center=false);
    }
    translate([10,(y_u + 25)]){
        circle(10);
    }
    
    // Right
    translate([220, (y_u + 5)]){
        square([10, 40] ,center=false);
    }
    translate([230,(y_u + 25)]){
        circle(10);
    }
}

module render_panel_tabs_cuts(u_count){
    y_u=((u_count-1) * rack_height_per_u);

    // Left
    translate([10,(y_u + 5)]){
        circle(10);
    }
    translate([10,(y_u + 45)]){
        circle(10);
    }
    translate([10,(y_u + 25)]){
        circle(rack_screw_radius);
    }
    
    // Right
    translate([230,(y_u + 5)]){
        circle(10);
    }
    translate([230,(y_u + 45)]){
        circle(10);
    }
    translate([230,(y_u + 25)]){
        circle(rack_screw_radius);
    }

}

module render_blank_panel(pos, console_u, name){
  dims=[200.0, (rack_height_per_u * console_u)];
    
  translate(pos) {
    difference() {
    union(){
        translate([20,0]){
            square(dims ,center=false);
        }
        render_panel_tabs(1);
        if(console_u > 1){
            render_panel_tabs(console_u);
        }
    }
    render_panel_tabs_cuts(1);
    if(console_u > 1){
        render_panel_tabs_cuts(console_u);
    }
    if(name) {
        translate([120, ((rack_height_per_u * console_u) - 12)]){
            text(name,  
                font = "PragmataPro Mono:style=Bold",
                size = 8,
                halign = "center");
        }
    }
    // Panel Created
    // Further cuts go after this point
}}}


// Render panels
//
if(false) {
    render_panel_env([0,0], 2);
    render_panel_snd([0,100.3], 2);
    render_panel_time([0,200.6], 4);
}


// ALREADY TESTED !!!
// Render Test Set
// 10cm across at the widest
if(false) {
    test_rot_switch([0,0]);
    test_sstd([17,0]);
    test_ssfd([60,0]);
    test_red_button([38,0]);
    test_re2([85,26]);
    test_led([27,22]);
    test_small_toggle([48,22]);
    test_ssed([0,34]);
    test_lcd([110,0]);
    test_arduino_nano_mount([0,54]);
}

//
// Render Second Test Set
//
if(false) {
    // Test longer switches
    // Test oled mount
    // Test covered black switches
    // etc
}


if(true) {
    render_blank_panel([0,0], 4);
}




// Render Test 7-Segment Displays only
if(false) {
    test_ssfd([0,0]);
    // test_ssed([43,0]);
    // test_rack([127,0]);
    // test_arduino_nano_mount([43, 20]);
    //test_re2([143,0]);
}

// Render Mount
if(false){
    render_frame_side([0,0]);
    
    translate([146,500]) {
        rotate(180, [0,0,1]) {
            render_frame_side([0,0]);
        }
    }
    
    // render_frame_face([150,0]);
    render_frame_face_side([150,0]);
    render_frame_face_side([220,0]);
    
    // Spacers
    // render_one_u_spacer_with_switch([290,0]);
    // render_one_u_spacer_horiz([290, 52]);
    // render_one_u_spacer([290, 0]);
    // render_one_u_spacer([342, 0]);
}

if(false){
    render_one_u_spacer_with_switch([0,0]);
    render_one_u_spacer_horiz([0, 52]);
}

// TODO
// Wire up each via breadboard
// Make each work
