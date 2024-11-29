//==============================================================================
//    File Name: 
//    Description:
//==============================================================================
#ifndef AXIS_ENCODER_H
#define	AXIS_ENCODER_H
#include <Adafruit_SSD1306.h>
#include "SquareSketchEEPROM.h"
#include "SquareSketchPicture.h"
#include "square_sketch.h"

#define Y_ROTARY_PIN1	0       //5
#define Y_ROTARY_PIN2	1       //6
#define Y_BUTTON_PIN	2       //7
#define X_ROTARY_PIN1	13      //A1
#define X_ROTARY_PIN2	12      //A2
#define X_BUTTON_PIN	14      //A3

#define PENUP    5
#define PENDOWN  10
#define PENERASE 16

const uint8_t cursor_modulus[3] = {PENUP, PENDOWN, PENERASE};

#define MAX_OPTIONS_MENU 5
#define MAX_OPTIONS_CHOICE 3
#define MAX_MAIN_MENU 7

const char MMItem0[] PROGMEM = "About";
const char MMItem1[] PROGMEM = "Sketch/View";
const char MMItem2[] PROGMEM = "Clear Sketch";
const char MMItem3[] PROGMEM = "Save Sketch";
const char MMItem4[] PROGMEM = "Load Saved Sketch";
const char MMItem5[] PROGMEM = "Erase Saved";
const char MMItem6[] PROGMEM = "Options";

PGM_P const MainMenu[] PROGMEM = 
{
    MMItem0,
    MMItem1,
    MMItem2,
    MMItem3,
    MMItem4,
    MMItem5,
    MMItem6,
};

const char option1[] PROGMEM =  "Clicks per Move";
const char option2[] PROGMEM =  "Debounce Time";
const char option3[] PROGMEM =  "Long Click Time";
const char option4[] PROGMEM =  "Reset All Options";
const char option5[] PROGMEM =  "Return To Main";

PGM_P const options_table[] PROGMEM = 
{
    option1,
    option2,
    option3,
    option4,
    option5,
};

const char option71[] PROGMEM =  "Clicks/Move: ";
const char option72[] PROGMEM =  "Debounce(ms): ";
const char option73[] PROGMEM =  "Long(ms): ";

PGM_P const options_choice[] PROGMEM = 
{
    option71,
    option72,
    option73,
};

typedef enum {
    SPLASH,
    MAINMENU,
    DRAWMODE,
    ABOUTMODE,
    CLEARSCR,
    SAVEEEPROM,
    LOADEEPROM,
    CLEAREEPROM,
    OPTIONSMODE,
    CLICKSMODE,
    DEBOUNCEMODE,
    LONGCLICKMODE,
    RESETMODE,
    ERRORMODE,
    CLEARSCROK,
    SAVEEEPROMOK,
    LOADEEPROMOK,
    CLEAREEPROMOK,
    RESETMODEOK,
} machine_state;

void            axisencoder_setup(uint8_t, uint8_t, SquareSketchPicture*, SquareSketchEEPROM*); 
void            axisencoder_setup_again(uint8_t, uint8_t);
void            increase_x();
void            decrease_x();
void            increase_y();
void            decrease_y();
void            handle_x();
void            handle_y();        
void            pciSetup(byte pin);
uint8_t         handle_press(uint8_t is_pressed, uint8_t state, long now);
machine_state   get_machinestate();
void            set_machinestate(machine_state st);
uint8_t         get_menu_current();
uint8_t         get_options_menu_current(void);
uint8_t         get_clicks_per_move_option();
void            set_clicks_per_move_option(uint8_t);
uint16_t        get_debounce_time_option();
void            set_debounce_time_option(uint16_t);
uint16_t        get_longpress_time_option();
void            set_longpress_time_option(uint16_t);
void            set_options_menu_count(uint16_t);
void            handleDrawX(void);
void            handleDrawY(void);
void            handleMainMenuX(void);
void            handleMainMenuY(void);
void            handleClickBy(void);
void            handleOptionsX(void);
void            handleOptionsY(void);
void            handleIndOptionsX(void);
void            handleIndOptionsY(void);
void            handleConfirmationY(void);
void            handleConfirmationN(void);

#endif	/* AXIS_ENCODER_H */
