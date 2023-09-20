use <utils.scad>
include <rack.scad>

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
    render_rect_holes([x_offset, (y_u - 45.0)]
        ,220.0 + (rack_overlap * 2)
        ,40.0
        ,10.0);    

    // Bottom
    render_rect_holes([x_offset, 5.0]
        ,220.0 + (rack_overlap * 2)
        ,40.0
        ,10.0);    

    // Screw holes
    render_rect_holes([10.0, 25.0]
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

module render_panel(pos, height){
    translate(pos) {            
        dims=[(rack_inside_width + (rack_overlap * 2))
            ,height];
      
        difference() {
        union(){
            translate([20.0 - rack_overlap, 0]){
                square(dims ,center=false);
            }
            render_panel_tabs(height);
        }
        render_panel_tabs_cuts(height);

        children();
        }
    }
}

