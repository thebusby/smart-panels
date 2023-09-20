// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 
//                                      START UTIL MODULES
// -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - -- - 

module render_hole(position, radius){
    translate(position) {
        circle(radius);
    }
}

module render_rect_holes(pos, x_alt, y_alt, screw_radius){
  translate(pos) {
    render_hole([0,0], screw_radius);
    render_hole([0,y_alt], screw_radius);
    render_hole([x_alt,0], screw_radius);
    render_hole([x_alt,y_alt], screw_radius);
}}

