//==============================================================================
//    File Name:    point.h
//    Description:  Structure definitions
//==============================================================================
// How Points work in drawing
// When xcounter or ycounter changes:
//      draw previous pixel
//          Pen Down: White
//          Erase: Black
//          Move: previous pixel state
//      get current pixel state of new pixel
//==============================================================================
#ifndef DRAWING_H
#define	DRAWING_H

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t is_drawn;
} point;

typedef union {
    uint16_t    int16;
    uint8_t     int8[2];
} eepromoption;

#endif	/* DRAWING_H */
