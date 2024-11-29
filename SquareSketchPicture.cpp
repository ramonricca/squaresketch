//==============================================================================
//    File Name:    SquareSketchPicture.cpp
//    Description:  Class for sketch
//==============================================================================
#include "SquareSketchPicture.h"

/*******************************************************************************
    function name: setup
    function desc: initialize 
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::setup(uint8_t x, uint8_t y) {
    current_location.x = x;
    current_location.y = y;
    current_location.is_drawn = 0;
    pen_state = MOVE;
    last_pen = MOVE;
}

/*******************************************************************************
    function name: set_params
    function desc: set clicks per move
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::set_params(uint8_t param) {
    clicks_per_move = param;
}

/*******************************************************************************
    function name: what_pen
    function desc: return pen state
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint8_t SquareSketchPicture::what_pen() {
    return (uint8_t)pen_state;
}

/*******************************************************************************
    function name: next_pen
    function desc: move to next pen
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::next_pen() {
    switch(pen_state) {
        case MOVE :
            pen_state = DRAW;
            break;
        case DRAW :
            pen_state = MOVE;
            break;
        case ERASE :
            pen_state = last_pen;
            break;
        default:
            pen_state = MOVE;
            break;
    }
}

/*******************************************************************************
    function name: lift_pen
    function desc: set to erase
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::lift_pen() {
    if ((pen_state == MOVE) || (pen_state == DRAW)) {
        last_pen = pen_state;
    }
    else {
        last_pen = MOVE;
    }
    
    pen_state = ERASE;
}

/*******************************************************************************
    function name: dump_point
    function desc: helper to dump point structure
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::dump_point(point *pt) {
    Serial.print(" x->");
    Serial.print(pt->x);
    Serial.print(" y->");
    Serial.print(pt->y);
    Serial.print(" is_drawn->");
    Serial.println(pt->is_drawn);
}

/*******************************************************************************
    function name: set_current_loc
    function desc: set current location according to clicks per move
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::set_current_loc(uint8_t x, uint8_t y, uint8_t thepen) {
    //uint16_t xx = x / clicks_per_move;
    //uint16_t yy = y / clicks_per_move;
            
    current_location.x = x;
    current_location.y = y;
    current_location.is_drawn = thepen;
#ifdef SQSK_DEBUG
    Serial.println("set_current_loc");
    dump_point(&current_location);
#endif
}

/*******************************************************************************
    function name: get_current_loc
    function desc: get point of current location
    inputs:
    outputs:
    returns:
*******************************************************************************/
point SquareSketchPicture::get_current_loc(void) {
    return current_location;
}

/*******************************************************************************
    function name: set_last_loc
    function desc: set the last location structure
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::set_last_loc(void) {         
    last_location.x = current_location.x;
    last_location.y = current_location.y;
    last_location.is_drawn = current_location.is_drawn;
#ifdef SQSK_DEBUG
    Serial.println("set_last_loc");
    dump_point(&last_location);
#endif
}

/*******************************************************************************
    function name: get_last_loc
    function desc: get the last location structure
    inputs:
    outputs:
    returns:
*******************************************************************************/
point SquareSketchPicture::get_last_loc(void) {
    return last_location;
}

/*******************************************************************************
    function name: reset_pen
    function desc: reset pen to MOVE
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchPicture::reset_pen(void) {
    pen_state = MOVE;
}
