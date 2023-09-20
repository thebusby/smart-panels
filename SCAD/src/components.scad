use <utils.scad>

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


// Guarded Missile switch
// Silver Toggle switch with Big Red Gurad on top of it
// From: https://www.amazon.co.jp/gp/product/B09VMTY4VZ/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&th=1
//
missile_toggle_radius=(11.6 / 2);

// VaneAims 22MM FXB38 Plastic LED Light Knob Rotary switch
// From: https://www.aliexpress.com/item/1005004513764440.html?spm=a2g0o.order_list.order_list_main.27.399b1802sZ6em2
//
fxb_radius=(21.8 / 2);

// Big Red Emergency Stop Button 
// From: https://www.amazon.co.jp/gp/product/B01BDGCH5A/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&th=1
// 
emergency_stop_radius=(22.2 / 2);

// Small Square guarded red latching button with LED
// Has "OMRON A16-1" printed on the base
// From: little stall in Akihabara **YEARS** ago
//
square_red_guard_radius=(16.0 / 2);


// Potentiometer
// From: https://www.amazon.co.jp/-/en/dp/B086619DJD?psc=1&smid=A3O114RNK5ESVR&ref_=chk_typ_imgToDp
//
pot_radius=(7.4/2);


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START PART MODULES
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

// Put position at center of RE dial - Model HW-040
module render_re_hw040(position){
    new_position = [(position[0] - re_x), 
                    (position[1] + re_y)];
    translate(new_position) {
        render_hole([re_screw_x, re_screw_y], re_screw_radius);
        render_hole([re_screw2_x, re_screw2_y], re_screw_radius);
        render_hole([re_x, -re_y], re_radius);
    }
}

// Put position at center of RE dial - Model KY-040
module render_re_ky040(position){
    new_position = [(position[0] - re_x), 
                    (position[1] + re_y)];
    translate(new_position) {
        render_hole([re2_screw_x, re2_screw_y], re2_screw_radius);
        render_hole([re2_screw2_x, re2_screw2_y], re2_screw_radius);
        render_hole([re2_x, -re2_y], re2_radius);
    }
}

module render_red_button(position){
    render_hole(position, red_button_radius);
}

module render_small_toggle (position){
    render_hole(position, small_toggle_radius);
}

module render_missile_toggle(position){
    render_hole(position, missile_toggle_radius);
}

module render_fxb(position){
    render_hole(position, fxb_radius);
}

module render_emergency_stop(position){
    render_hole(position, emergency_stop_radius);
}

module render_square_red_guard(position){
    render_hole(position, square_red_guard_radius);
}

module render_pot(position){
    render_hole(position, pot_radius);
}

// Provide LED diameter as well
module render_led (position, diameter){
    render_hole(position, (diameter / 2));
}

module render_rot_switch(position){    
    translate([position[0], (position[1] + 12.5)]){
        square([3, 1.3] ,center=true);
    }
    render_hole(position, 8.9); 
}

module render_bw_rot_switch(position){    
    render_hole([position[0], (position[1] + 9.5)], 1.55);
    render_hole(position, 4.5); 
}

module render_cherrymx(position, notches) {
  /* Cherry MX switch hole with the center at `position`. Sizes come
     from the ErgoDox design. */
  hole_size    = 13.97;
  notch_width  = 3.5001;
  notch_offset = 4.2545;
  notch_depth  = 0.8128;
  translate(position) {
    union() {
      square([hole_size, hole_size], center=true);
      if (notches == true) {
        translate([0, notch_offset]) {
          square([hole_size+2*notch_depth, notch_width], center=true);
        }
        translate([0, -notch_offset]) {
          square([hole_size+2*notch_depth, notch_width], center=true);
        }
      }
    }
  }
};



// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START FRAME SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 


rack_screw_radius=3.55; // M5?
rack_screw_offset=10.0; // 1cm in on 2020
rack_full_width=240.0;
rack_inside_width=200.0;
rack_height_per_u=50.0;
rack_overlap=1.5; // Amount top plate overlaps rack

joystick_plate_height=120.0;
joystick_screw_radius=2.28; // 4.5mm radius?
joystick_x_offset=33.0;
joystick_y_offset=30.0;
joystick_screw_distance=60.1;
joystick_radius=(110 / 2);

// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START FRAME MODULES
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

module render_panel_tabs(y_u){
    // Left
    translate([10, 5]){
        square([10, 40] ,center=false);
    }
    translate([10, 25]){
        circle(10.0);
    }

    translate([10, (y_u - 45)]){
        square([10, 40] ,center=false);
    }
    translate([10, (y_u - 25)]){
        circle(10);
    }
 
    // Right
    translate([220, 5]){
        square([10, 40] ,center=false);
    }
    translate([230, 25]){
        circle(10);
    }
    translate([220, (y_u - 45)]){
        square([10, 40] ,center=false);
    }
    translate([230, (y_u - 25)]){
        circle(10);
    }
}

module render_panel_tabs_cuts(y_u){
    x_offset=10.0 - rack_overlap; // So we have a TINY bit of overlap with the frame

    // Top
    render_rect_bolt_holes([x_offset, (y_u - 45.0)]
        ,220.0 + (rack_overlap * 2)
        ,40.0
        ,10.0);    

    // Bottom
    render_rect_bolt_holes([x_offset, 5.0]
        ,220.0 + (rack_overlap * 2)
        ,40.0
        ,10.0);    

    // Screw holes
    render_rect_bolt_holes([10.0, 25.0]
        ,220.0
        ,(y_u - 50.0)
        ,rack_screw_radius);    

    
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


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START PANELS
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

module render_bottom(pos){
  translate(pos) {            
    dims=[rack_inside_width, joystick_plate_height];
      
    difference() {
    square(dims ,center=false);

    // Render rack holes
    render_rect_bolt_holes([rack_screw_offset,rack_screw_offset]
        ,(rack_inside_width - 20.0)
        ,(joystick_plate_height - 20.0)
        ,rack_screw_radius);    

    // Render joystick mount holes
    render_rect_bolt_holes([joystick_x_offset, joystick_y_offset]
        ,joystick_screw_distance
        ,joystick_screw_distance
        ,joystick_screw_radius);
   echo("Screw center is x="
        ,(joystick_x_offset + (joystick_screw_distance / 2)) 
        ," y="
        ,(joystick_y_offset + (joystick_screw_distance / 2)));    

}}}


module render_top(pos){
  translate(pos) {            
    dims=[(rack_inside_width + (rack_overlap * 2))
      ,joystick_plate_height];
      
    difference() {
    union(){
        translate([20.0 - rack_overlap, 0]){
            square(dims ,center=false);
        }
        render_panel_tabs(joystick_plate_height);
    }

    // Render rack holes
    // render_rect_bolt_holes([rack_screw_offset,rack_screw_offset]
    //     ,(rack_full_width - 20.0)
    //     ,(joystick_plate_height - 20.0)
    //     ,rack_screw_radius);
    
    render_panel_tabs_cuts(joystick_plate_height);

    // Render space for joystick to come through
    // 20.0 is to ignore the 2cm over the 2020 frame
    render_hole([(20.0 + joystick_x_offset + (joystick_screw_distance / 2))
        ,(joystick_y_offset + (joystick_screw_distance / 2))]
        ,joystick_radius);
    echo("Hole center is x="
       ,(joystick_x_offset + (joystick_screw_distance / 2))
       , " y="
       , (joystick_y_offset + (joystick_screw_distance / 2)));

    // Render additional controls
    x_base=160.0;
    render_small_toggle([x_base, 99.0]);
    render_small_toggle([x_base+40.0, 99.0]);    
    render_red_button([x_base+20.0, 62.0]);
    render_re2([x_base+20.0, 25.0]);        
}}}


// To compare top vs bottom for alignment
if(false){ 
    render_bottom([20.0, (joystick_plate_height + 2.0)]);
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
