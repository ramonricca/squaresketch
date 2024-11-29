//==============================================================================
//    File Name:    SquareSketchEEPROM.h
//    Description:  EEPROM Interface
//==============================================================================

#ifndef SQUARESKETCHEEPROM_H
#define	SQUARESKETCHEEPROM_H
#include <Arduino.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include "point.h"

// NUMPARAMS = number of options * 2
#define NUMPARAMS   6

typedef enum {
    CLICKS,
    DEBOUNCE,
    LONG
} optionindex;


#define DEF_CHOICE_CLICKS 4
#define DEF_CHOICE_DEBOUNCE 5
#define DEF_CHOICE_LONGCLICKS 550
#define MIN_CHOICE_CLICKS 1
#define MAX_CHOICE_CLICKS 32
#define MIN_CHOICE_DEBOUNCE 5
#define MAX_CHOICE_DEBOUNCE 100
#define MIN_CHOICE_LONGCLICKS 200
#define MAX_CHOICE_LONGCLICKS 1000

class SquareSketchEEPROM {
public:    
                            SquareSketchEEPROM(void);
    void                    begin(uint16_t);
    void                    Reset_EEPROM(void);
    void                    Buffer_to_EEPROM(uint8_t *);
    void                    EEPROM_to_Buffer(uint8_t *);
    void                    read(uint16_t, uint8_t*);
    void                    write(uint16_t, uint8_t);
    void                    set_option(optionindex, uint16_t);
    uint16_t                get_option(optionindex);
    
private:
    int8_t                  FIRST_ADDRESS;
    uint16_t                buffer_size;    
};

#endif	/* SQUARESKETCHEERAM_H */

