//==============================================================================
//    File Name:    SquareSketchPicture.h
//    Description:  Header Picture Class
//==============================================================================

#ifndef SQUARESKETCHPICTURE_H
#define	SQUARESKETCHPICTURE_H

#include <Arduino.h>
#include <stdint.h>
#include "point.h"

typedef enum {
    MOVE,
    DRAW,
    ERASE
} pen;

class SquareSketchPicture {
    public:                
        void                setup(uint8_t x, uint8_t yd);
        uint8_t             what_pen();
        void                next_pen();
        void                lift_pen();
        void                dump_point(point *pt);
        void                set_current_loc(uint8_t, uint8_t, uint8_t);
        point               get_current_loc(void);
        void                set_last_loc(void);
        point               get_last_loc(void);
        void                reset_pen();
        void                set_params(uint8_t param);
       
    private:
        point               current_location;
        point               last_location;
        pen                 pen_state;
        pen                 last_pen;
        uint8_t             clicks_per_move;
};

#endif	/* SQUARESKETCHPICTURE_H */
