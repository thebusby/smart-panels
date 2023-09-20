// -*- mode: c -*-
/* All distances are in mm. */

// Set output quality
$fn=50;

// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

module render_frame(x, y, length, position, orientation){
    translate(position) {
        rotate(orientation){
            cube([x, y, length]);
        }
    }
}

module render_2020(length, position, orientation){
    render_frame(20, 20, length, position, orientation);
}

module render_4040(length, position, orientation){
    render_frame(40, 40, length, position, orientation);
}

module render_4080(length, position, orientation){
    render_frame(40, 80, length, position, orientation);
}


// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                       END PART SPEC
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

side_panel_width=200;
side_panel_length=1000;
side_panel_height=500;

// 6m PER BOX (4x each dimension)
module render_side_panel(position){
    translate(position){
        render_2020(side_panel_length, [0,0,0], [-90,0,0]);
        render_2020(side_panel_length, [(side_panel_width - 20),0,0], [-90,0,0]);
        render_2020(side_panel_width, [0,20,0], [-90,0,-90]);
        render_2020(side_panel_width, [0,side_panel_length,0], [-90,0,-90]);
        
        render_2020(side_panel_length, [0,0,side_panel_height], [-90,0,0]);
        render_2020(side_panel_length, [(side_panel_width - 20),0,side_panel_height], [-90,0,0]);
        render_2020(side_panel_width, [0,20,side_panel_height], [-90,0,-90]);
        render_2020(side_panel_width, [0,side_panel_length,side_panel_height], [-90,0,-90]);
        
        render_2020(side_panel_height, [0,0,0],[0,0,0]);
        render_2020(side_panel_height, [(side_panel_width - 20),0,0],[0,0,0]);
        render_2020(side_panel_height, [0,side_panel_length-20,0],[0,0,0]);
        render_2020(side_panel_height, [(side_panel_width - 20),side_panel_length-20,0],[0,0,0]);
        
        
        
    }
}

// Define primary dimensions
frame_width=600;
frame_length=1300;
frame_seat_length=500;
monitor_height=800;


// Bottom rectangnle frame
// 4080: 2x Length, 3x width
if(0){
render_4080(frame_length, [0,0,80], [-90,0,0]);
render_4080(frame_length, [(40+frame_width),0,80], [-90,0,0]);
render_4080(frame_width, [40,40,80], [-90,0,-90]);
render_4080(frame_width, [40,frame_length,80], [-90,0,-90]);

// Seat frame
// 4080:
// 4040:
seat_offset=80;
seat_width=500;
render_4080(frame_width, [40,frame_seat_length,80], [-90,0,-90]);
render_4040(frame_seat_length, [seat_offset,0,120], [-90,0,0]);
render_4040(frame_seat_length, [seat_offset+seat_width,0,120], [-90,0,0]);

// Monitor stands
// 4080:
monitor_offset=160;
render_4080(monitor_height, [0,frame_length-monitor_offset,80], [0,0,0]);
render_4080(monitor_height, [(40+frame_width),frame_length-monitor_offset,80], [0,0,0]);
render_4080(frame_width, [40,40+frame_length-monitor_offset,monitor_height+80], [-90,0,-90]);
}

// Side panels
// 2020: 
render_side_panel([-side_panel_width, -100, 0]);
render_side_panel([frame_width+80, -100, 0]);
// Connector
render_2020((side_panel_width*2)+frame_width+80, [-side_panel_width, -120,0], [0,90,0]);

// Keyboard tray, initial design
kbm_tray_length=250;
render_2020(frame_width+80, [0,0,side_panel_height+20], [0,90,0]);
render_2020(frame_width+80, [0,kbm_tray_length,side_panel_height+20], [0,90,0]);









