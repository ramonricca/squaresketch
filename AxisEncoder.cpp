//==============================================================================
//    File Name:        AxisEncoder.cpp
//    Description:      Handles Encoder functions for SquareSketch
//==============================================================================
#include <Arduino.h>
#include <stddef.h>
#include "AxisEncoder.h"
#include "square_sketch.h"
#include "SquareSketchEEPROM.h"

volatile uint16_t xcounter;
volatile uint16_t ycounter;
uint8_t xcurrentState;
uint8_t ycurrentState;
uint8_t xinitState;
uint8_t yinitState;
uint8_t xbuttonState;
uint8_t xbuttonCurrent;
uint8_t xbuttonLast;
uint8_t ybuttonState;
uint8_t ybuttonCurrent;
uint8_t menu_current;
uint16_t menu_count = 0;
uint8_t options_menu_current;
uint16_t options_menu_count = 0;
uint8_t ok = 0;
uint8_t numclicksX;
uint8_t numclicksY;

uint16_t clicks_per_move_option;
uint16_t debounce_time_option;
uint16_t longpress_time_option;

machine_state machinestate;

long    last_press;

SquareSketchPicture     *imageptr;
SquareSketchEEPROM      *eepromptr;

void (*mainMenuCallbackX)(void) = &handleMainMenuX;
void (*mainMenuCallbackY)(void) = &handleMainMenuY;
void (*drawXCallback)(void) = &handleDrawX;
void (*drawYCallback)(void) = &handleDrawY;
void (*pressCallback)(void) = &handleClickBy;
void (*optionsCallbackX)(void) = &handleOptionsX;
void (*optionsCallbackY)(void) = &handleOptionsY;
void (*indOptionsCallbackX)(void) = &handleIndOptionsX;
void (*indOptionsCallbackY)(void) = &handleIndOptionsY;
void (*confirmCallbackY)(void) = &handleConfirmationY;
void (*confirmCallbackN)(void) = &handleConfirmationN;

/*******************************************************************************
    function name: axisencoder_setup
    function desc: Initializes module
    inputs:
    outputs:
    returns:
*******************************************************************************/
void axisencoder_setup(uint8_t xct, uint8_t yct, SquareSketchPicture *sqskpic,SquareSketchEEPROM *eep) {
    //Serial.println("axisencoder_setup");
    pciSetup(X_ROTARY_PIN1);
    pciSetup(X_ROTARY_PIN2);
    pciSetup(X_BUTTON_PIN);
    pciSetup(Y_ROTARY_PIN1);
    pciSetup(Y_ROTARY_PIN2);
    pciSetup(Y_BUTTON_PIN);
    //Serial.println("PCI setup");
      
    imageptr = sqskpic;
    eepromptr = eep;
    
    
    clicks_per_move_option = DEF_CHOICE_CLICKS;
    debounce_time_option = DEF_CHOICE_DEBOUNCE;
    longpress_time_option = DEF_CHOICE_LONGCLICKS;
        
    clicks_per_move_option = eepromptr->get_option(CLICKS);
    debounce_time_option = eepromptr->get_option(DEBOUNCE);
    longpress_time_option = eepromptr->get_option(LONG);
    
#ifdef SQSK_DEBUG
    Serial.println("objects set up");
    Serial.print("CLICKS ");
    Serial.print(clicks_per_move_option);
    Serial.print(" DEBOUNCE ");
    Serial.print(debounce_time_option);
    Serial.print(" LONG ");
    Serial.println(longpress_time_option);
    Serial.println("eeprom settings set up");
#endif
    
    xinitState = digitalRead(X_ROTARY_PIN1);
    yinitState = digitalRead(Y_ROTARY_PIN1);
    xbuttonState = digitalRead(X_BUTTON_PIN);
    ybuttonState = digitalRead(Y_BUTTON_PIN);
    xcounter = xct * clicks_per_move_option;
    ycounter = yct * clicks_per_move_option;
    imageptr->setup(xct, yct);
    imageptr->set_params((uint16_t)clicks_per_move_option);
    machinestate = SPLASH;
    menu_current = 0;
}

/*******************************************************************************
    function name: axisencoder_setup_again
    function desc: reinitializes encoders and axis
    inputs:
    outputs:
    returns:
*******************************************************************************/
void axisencoder_setup_again(uint8_t xct, uint8_t yct) {
    xcounter = xct * clicks_per_move_option;
    ycounter = yct * clicks_per_move_option;
    imageptr->set_params(clicks_per_move_option);
}

/*******************************************************************************
    function name: pciSetup
    function desc: sets up pin interrupts
    inputs:
    outputs:
    returns:
*******************************************************************************/
void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}


/*******************************************************************************
    function name: PCINT3_vect
    function desc: Handle X-Axis Encoder Routing
    inputs:
    outputs:
    returns:
*******************************************************************************/
ISR (PCINT3_vect) // handle pin change interrupt for D0 to D7 here
{
#ifdef SQSK_DEBUG
    Serial.println("X!");
#endif
    // Read the current state of CLK
    xcurrentState = digitalRead(X_ROTARY_PIN1);
    xbuttonCurrent = digitalRead(X_BUTTON_PIN);
    // check button
    numclicksX = handle_press(xbuttonCurrent, xbuttonState, millis());
    
    switch (machinestate) {
        case DRAWMODE:
            handleDrawX();
            break;
        case MAINMENU:
            mainMenuCallbackX();
            break;
        case ABOUTMODE:
            pressCallback();
            break;
        case CLEARSCR:
            pressCallback();
            break;
        case SAVEEEPROM:
            pressCallback();
            break;
        case LOADEEPROM:
            pressCallback();
            break;
        case CLEAREEPROM:
            pressCallback();
            break;
        case OPTIONSMODE:
            optionsCallbackX();
            break;
        case CLICKSMODE:
            indOptionsCallbackX();
            break;
        case DEBOUNCEMODE:
            indOptionsCallbackX();
            break;
        case LONGCLICKMODE:
            indOptionsCallbackX();
            break;
        case RESETMODE:
            pressCallback();
            break;
        case RESETMODEOK:
            confirmCallbackN();
            break;
        case CLEARSCROK:
            confirmCallbackN();
            break;
        case SAVEEEPROMOK:
            confirmCallbackN();
            break;
        case LOADEEPROMOK:
            confirmCallbackN();
            break;
        case CLEAREEPROMOK:
            confirmCallbackN();
            break;
        default:
            set_machinestate(MAINMENU);
            break;
    }
    
    numclicksX = 0;
    numclicksY = 0;
 }

/*******************************************************************************
    function name: PCINT1_vect
    function desc: Handle YAxis Encoder Routing
    inputs:
    outputs:
    returns:
*******************************************************************************/
ISR (PCINT1_vect) // handle pin change interrupt for A0 to A5 here
{
#ifdef SQSK_DEBUG
    Serial.println("Y!");
#endif
    // Read the current state of CLK
    ycurrentState = digitalRead(Y_ROTARY_PIN1);
    ybuttonCurrent = digitalRead(Y_BUTTON_PIN);
    numclicksY = handle_press(ybuttonCurrent, ybuttonState, millis());
    
    switch (machinestate) {
        case DRAWMODE:
            handleDrawY();
            break;
        case MAINMENU:
            mainMenuCallbackY();
            break;
        case ABOUTMODE:
            pressCallback();
            break;
        case CLEARSCR:
            pressCallback();
            break;
        case SAVEEEPROM:
            pressCallback();
            break;
        case LOADEEPROM:
            pressCallback();
            break;
        case CLEAREEPROM:
            pressCallback();
            break;
        case OPTIONSMODE:
            optionsCallbackY();
            break;
        case CLICKSMODE:
            indOptionsCallbackY();
            break;
        case DEBOUNCEMODE:
            indOptionsCallbackY();
            break;
        case LONGCLICKMODE:
            indOptionsCallbackY();
            break;
        case RESETMODE:
            pressCallback();
            break;
        case RESETMODEOK:
            confirmCallbackY();
            break;
        case CLEARSCROK:
            confirmCallbackY();
            break;
        case SAVEEEPROMOK:
            confirmCallbackY();
            break;
        case LOADEEPROMOK:
            confirmCallbackY();
            break;
        case CLEAREEPROMOK:
            confirmCallbackY();
            break;
        default:
            break;
    }
    numclicksX = 0;
    numclicksY = 0;
}

/*******************************************************************************
    function name: handle_press
    function desc: Is click a short or long press
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint8_t handle_press(uint8_t is_pressed, uint8_t state, long now) {
#ifdef SQSK_DEBUG
    Serial.print("machinestate:");
    Serial.println(machinestate);
    Serial.println("handle_press();");
    Serial.print("is_pressed: ");
    Serial.println(is_pressed);
#endif
    
    if (is_pressed != state) {
        if (!last_press) {
            last_press = now;
            ok = 0;
        }
    }
    else if (last_press) {
        long difftime = now - last_press;
        //Serial.print("difftime: ");
        //Serial.println(difftime);
        if ((difftime >= debounce_time_option) && (difftime < longpress_time_option)) {
            ok = 1;
        }
        else if ((difftime >= debounce_time_option) && (difftime >= longpress_time_option)) {
            ok = 2;
        }
        else {
            ok = 0;
        }
        
        last_press = 0;
    }
    else {
        ok = 0;
    }
    
#ifdef SQSK_DEBUG
    Serial.print("last_press:");
    Serial.println(last_press);
    Serial.print("ok=");
    Serial.println(ok);
#endif 
    
    return ok;
}

/*******************************************************************************
    function name: get_machinestate
    function desc: Returns what state in finite state machine
    inputs:
    outputs:
    returns:
*******************************************************************************/
machine_state get_machinestate() {
    return machinestate;
}

/*******************************************************************************
    function name: set_machinestate
    function desc: change finite state machine state
    inputs:
    outputs:
    returns:
*******************************************************************************/
void set_machinestate(machine_state st) {
    if ((machinestate != DRAWMODE) && (st != DRAWMODE)) {
        cleardisplay = true;
    }
    
    if ((st == DRAWMODE) && (machinestate == MAINMENU)) {
        if (draw_initialized) {
            load_buffer = true;
            delay(100);
        }
        else {
            draw_initialized = true;
            cleardisplay = true;
            load_buffer = false;
        }
    }
    else {
        cleardisplay = true;
        load_buffer = false;
    }
    
    if ((machinestate == DRAWMODE) && (st == MAINMENU)) {
        save_sketch();
        delay(100);
        cleardisplay = true;
    }
    
    machinestate = st;
}

/*******************************************************************************
    function name: get_menu_current
    function desc: returns current menu item
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint8_t get_menu_current() {
    return menu_current;
}

/*******************************************************************************
    function name: get_options_menu_current
    function desc: returns current options menu item
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint8_t get_options_menu_current() {
    return options_menu_current;
}

/*******************************************************************************
    function name: set_options_menu_count
    function desc: set current options menu count
    inputs:
    outputs:
    returns:
*******************************************************************************/
void set_options_menu_count(uint16_t omc) {
    options_menu_count = omc;
}


/*******************************************************************************
    function name: get_clicks_per_move_option
    function desc: returns clicks per move option
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint8_t get_clicks_per_move_option() {    
    return clicks_per_move_option;
}

/*******************************************************************************
    function name: set_clicks_per_move_option
    function desc: sets clicks per move option
    inputs:
    outputs:
    returns:
*******************************************************************************/
void set_clicks_per_move_option(uint8_t cpm) {    
    clicks_per_move_option = cpm;
}

/*******************************************************************************
    function name: get_debounce_time_option
    function desc: returns debounce time option
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint16_t get_debounce_time_option() {
    return debounce_time_option;
}

/*******************************************************************************
    function name: set_debounce_time_option
    function desc: sets debounce time option
    inputs:
    outputs:
    returns:
*******************************************************************************/
void set_debounce_time_option(uint16_t dto) {
    debounce_time_option = dto;
}

/*******************************************************************************
    function name: get_longpress_time_option
    function desc: returns long press option
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint16_t get_longpress_time_option() {    
    return longpress_time_option;
}

/*******************************************************************************
    function name: set_longpress_time_option
    function desc: sets long press option
    inputs:
    outputs:
    returns:
*******************************************************************************/
void set_longpress_time_option(uint16_t lto) {    
    longpress_time_option = lto;
}

/*******************************************************************************
    function name: handleDrawX
    function desc: Draw Mode X Axis
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleDrawX(void) {
    if (numclicksX == 1) {
        imageptr->next_pen();
#ifdef SQSK_DEBUG
        Serial.print("PEN: ");
        Serial.println(imageptr->what_pen());  
#endif
    }
    else if (numclicksX == 2) {
        correctCursor();
        set_machinestate(MAINMENU);
        delay(100);
    } 

    if (xcurrentState != xinitState  && xcurrentState == 1) {
       if (digitalRead(X_ROTARY_PIN2) != xcurrentState) {
           imageptr->set_last_loc();
           xcounter--;
#ifdef SQSK_DEBUG
           Serial.print("xcounter=");
           Serial.println(xcounter);
#endif
           if (xcounter < 0) {
               xcounter = 0;
           }
       } 
       else {
           imageptr->set_last_loc();
           xcounter++;
#ifdef SQSK_DEBUG
           Serial.print("xcounter=");
           Serial.println(xcounter);
#endif
           if (xcounter > (127 * clicks_per_move_option)) {
               xcounter = (127 * clicks_per_move_option);
           }
       }

       correctCursor();
       setCurrentPixel(xcounter, ycounter);
       // Draw Pixels now
       if (imageptr->what_pen()) {
            drawPixelWithPen(imageptr->get_last_loc());
        }
        else {
            drawPixelPrev(imageptr->get_last_loc());
        }
    }

    // Remember last CLK state for next cycle
    xinitState = xcurrentState;    
}

/*******************************************************************************
    function name: handleDrawY
    function desc: Draw mode Y Axis
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleDrawY(void) {
    if (numclicksY == 1) {
        imageptr->lift_pen();
#ifdef SQSK_DEBUG
        Serial.print("PEN: ");
        Serial.println(imageptr->what_pen());
#endif
    }
    else if (numclicksY == 2) {
        correctCursor();
        set_machinestate(MAINMENU);
        delay(100);
    } 

    // If last and current state of CLK are different, then we can be sure that the pulse occurred
    if (ycurrentState != yinitState  && ycurrentState == 1) {
        // Encoder is rotating counterclockwise so we decrement the counter
        if (digitalRead(Y_ROTARY_PIN2) != ycurrentState) {
            imageptr->set_last_loc();
            ycounter--;
            if (ycounter < 0) {
                ycounter = 0;
            }
        } 
        else {
            imageptr->set_last_loc();
            // Encoder is rotating clockwise so we increment the counter
            ycounter++;
            if (ycounter > 63 * clicks_per_move_option) {
                ycounter = 63 * clicks_per_move_option;
            }
        }
        correctCursor();
        setCurrentPixel(xcounter, ycounter);
        // Draw Pixels now
        if (imageptr->what_pen()) {
            drawPixelWithPen(imageptr->get_last_loc());
        }
        else {
            drawPixelPrev(imageptr->get_last_loc());
        }
    }
 
    // Remember last CLK state for next cycle
    yinitState = ycurrentState;
}

/*******************************************************************************
    function name: handleMainMenuX
    function desc: Main Menu Mode Handling X Encoder
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleMainMenuX(void) {
    if (xcurrentState != xinitState  && xcurrentState == 1) {
       if (digitalRead(X_ROTARY_PIN2) != xcurrentState) {
           if (menu_count > 0)
               menu_count--;
           else
               menu_count = 0;
       } 
       else {
           menu_count++;
       }
    }

    menu_current = menu_count / clicks_per_move_option;

    if (menu_count > MAX_MAIN_MENU * clicks_per_move_option) {
       menu_count = (MAX_MAIN_MENU * clicks_per_move_option) - 1; 
    }

    if (menu_current >= MAX_MAIN_MENU) {
       menu_current = MAX_MAIN_MENU - 1;
    }

    if (numclicksX) {
       switch (menu_current) {
           case 0:
#ifdef SQSK_DEBUG
               Serial.println("Selected About");
#endif
               set_machinestate(ABOUTMODE);
               break;
           case 1:
#ifdef SQSK_DEBUG
               Serial.println("Selected Draw");
#endif
               set_machinestate(DRAWMODE);
               break;
           case 2:
#ifdef SQSK_DEBUG
               Serial.println("Selected Clear Screen");
#endif
               set_machinestate(CLEARSCROK);
               break;
           case 3:
#ifdef SQSK_DEBUG
               Serial.println("Selected Save EEPROM");
#endif
               set_machinestate(SAVEEEPROMOK);
               break;
           case 4:
#ifdef SQSK_DEBUG
               Serial.println("Selected Load EEPROM");
#endif
               set_machinestate(LOADEEPROMOK);
               break;
           case 5:
#ifdef SQSK_DEBUG
               Serial.println("Selected Clear EEPROM");
#endif
               set_machinestate(CLEAREEPROMOK);
               break;
           case 6:
#ifdef SQSK_DEBUG
               Serial.println("Selected Options");
#endif
               options_menu_count = 0;
               options_menu_current = 0;
               set_machinestate(OPTIONSMODE);
               break;
           default:
                set_machinestate(ERRORMODE);
               break;
        }
        delay(100);
    }

    // Remember last CLK state for next cycle
    xinitState = xcurrentState;       
}

/*******************************************************************************
    function name: handleMainMenuY
    function desc: Main Menu Mode Handling Y Encoder
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleMainMenuY(void) {
    if (ycurrentState != yinitState  && ycurrentState == 1) {
       if (digitalRead(Y_ROTARY_PIN2) != ycurrentState) {
           if (menu_count > 0)
               menu_count--;
           else
               menu_count = 0;
       } 
       else {
           menu_count++;
       }
    }

    menu_current = menu_count / clicks_per_move_option;

    if (menu_count > MAX_MAIN_MENU * clicks_per_move_option) {
       menu_count = (MAX_MAIN_MENU * clicks_per_move_option) - 1; 
    }

    if (menu_current >= MAX_MAIN_MENU) {
       menu_current = MAX_MAIN_MENU - 1;
    }

    if (numclicksY) {
       switch (menu_current) {
           case 0:
#ifdef SQSK_DEBUG
               Serial.println("Selected About");
#endif
               set_machinestate(ABOUTMODE);
               break;
           case 1:
#ifdef SQSK_DEBUG
               Serial.println("Selected Draw");
#endif
               set_machinestate(DRAWMODE);
               break;
           case 2:
#ifdef SQSK_DEBUG
               Serial.println("Selected Clear Screen");
#endif
               set_machinestate(CLEARSCROK);
               break;
           case 3:
#ifdef SQSK_DEBUG
               Serial.println("Selected Save EEPROM");
#endif
               set_machinestate(SAVEEEPROMOK);
               break;
           case 4:
#ifdef SQSK_DEBUG
               Serial.println("Selected Load EEPROM");
#endif
               set_machinestate(LOADEEPROMOK);
               break;
           case 5:
#ifdef SQSK_DEBUG
               Serial.println("Selected Clear EEPROM");
#endif
               set_machinestate(CLEAREEPROMOK);
               break;
           case 6:
#ifdef SQSK_DEBUG
               Serial.println("Selected Options");
#endif
               options_menu_count = 0;
               options_menu_current = 0;
               set_machinestate(OPTIONSMODE);
               break;
           default:
                set_machinestate(ERRORMODE);
               break;
        }
        delay(100);
    }

    // Remember last CLK state for next cycle
    yinitState = ycurrentState;       
}

/*******************************************************************************
    function name: handleClickBy
    function desc: handle Tap to exit
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleClickBy(void) {
    if (numclicksX) {
        if (machinestate == RESETMODE) {
            set_machinestate(OPTIONSMODE);
        }
        else {
            set_machinestate(MAINMENU);
        }
    } 
    else if (numclicksY) {
        if (machinestate == RESETMODE) {
            set_machinestate(OPTIONSMODE);
        }
        else {
            set_machinestate(MAINMENU);
        }
    }
    numclicksX = 0;
    numclicksY = 0;
}

/*******************************************************************************
    function name: handleConfirmationY
    function desc: handle OK
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleConfirmationY(void) {
    machine_state cur_st = get_machinestate();
#ifdef SQSK_DEBUG
    Serial.print("handleConfirmation: ");
    Serial.println(cur_st);
    Serial.print("X:");
    Serial.print(numclicksX);
    Serial.print("Y:");
    Serial.println(numclicksY);
#endif
    if (numclicksY) {
        switch(cur_st) {
            case CLEARSCROK:
                delay(500);
                set_machinestate(CLEARSCR);
                break;
            case SAVEEEPROMOK:
                delay(500);
                set_machinestate(SAVEEEPROM);
                break;
            case LOADEEPROMOK:
                delay(500);
                set_machinestate(LOADEEPROM);
                break;
            case CLEAREEPROMOK:
                delay(500);
                set_machinestate(CLEAREEPROM);
                break;
            case RESETMODEOK:
                delay(500);
                set_machinestate(RESETMODE);
                break;
            default:
                break;
        }
    }
    numclicksX = 0;
    numclicksY = 0;
 }

/*******************************************************************************
    function name: handleConfirmationY
    function desc: handle NO
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleConfirmationN(void) {
    machine_state cur_st = get_machinestate();
#ifdef SQSK_DEBUG
    Serial.print("handleConfirmation: ");
    Serial.println(cur_st);
    Serial.print("X:");
    Serial.print(numclicksX);
    Serial.print("Y:");
    Serial.println(numclicksY);
#endif
    if (numclicksX) {
        if (cur_st == RESETMODEOK) {
            delay(500);
            set_machinestate(OPTIONSMODE);
        }
        else {
            delay(500);
            set_machinestate(MAINMENU);
        }
    }
    numclicksX = 0;
    numclicksY = 0;
}


/*******************************************************************************
    function name: handleOptionsX
    function desc: Options Menu Mode Handling X Encoder
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleOptionsX(void) {
#ifdef SQSK_DEBUG
    Serial.print("options_menu_count:");
    Serial.println(options_menu_count);
    Serial.print("xcurrentState:");
    Serial.println(xcurrentState);
#endif
    if (xcurrentState != xinitState  && xcurrentState == 1) {
        uint8_t rot2 = digitalRead(X_ROTARY_PIN2);
        if (rot2 != xcurrentState) {
            if (options_menu_count > 0) {
                options_menu_count--;
            }
            else {
                options_menu_count = 0;
            }
        } 
        else {
            options_menu_count++;
        }
    }
    options_menu_current = options_menu_count / clicks_per_move_option;

    if (options_menu_count > MAX_OPTIONS_MENU * clicks_per_move_option) {
        options_menu_count = (MAX_OPTIONS_MENU * clicks_per_move_option) - 1; 
    }

    if (options_menu_current >= MAX_OPTIONS_MENU) {
        options_menu_current = MAX_OPTIONS_MENU - 1;
    }

    if (numclicksX) {
        point cur;
        cur = imageptr->get_current_loc();
        switch (options_menu_current) {
            case 0:
#ifdef SQSK_DEBUG
                Serial.println("Selected CPM");
#endif
                set_machinestate(CLICKSMODE);
                break;
            case 1:
#ifdef SQSK_DEBUG
                Serial.println("Selected Debounce");
#endif
                set_machinestate(DEBOUNCEMODE);
                break;
            case 2:
#ifdef SQSK_DEBUG
                Serial.println("Selected Long Click");
#endif
                set_machinestate(LONGCLICKMODE);
                break;
            case 3:
#ifdef SQSK_DEBUG
                Serial.println("Selected Reset All");
#endif
                set_machinestate(RESETMODEOK);
                break;
            case 4:
#ifdef SQSK_DEBUG
                Serial.println("Selected Return To Main");
#endif
                axisencoder_setup_again(cur.x,cur.y);
                menu_count = 6 * clicks_per_move_option;
                set_machinestate(MAINMENU);
                break;
            default:
                set_machinestate(ERRORMODE);
                break;
        }
        delay(100);
    }

    xinitState = xcurrentState;       
}

/*******************************************************************************
    function name: handleOptionsY
    function desc: Options Menu Mode Handling Y Encoder
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleOptionsY(void) {
#ifdef SQSK_DEBUG
    Serial.print("options_menu_count:");
    Serial.println(options_menu_count);
    Serial.print("ycurrentState:");
    Serial.println(ycurrentState);
    Serial.print("numclicksY:");
    Serial.println(numclicksY);
#endif
    if (ycurrentState != yinitState  && ycurrentState == 1) {
        uint8_t rot2 = digitalRead(Y_ROTARY_PIN2);
        if (rot2 != ycurrentState) {
            if (options_menu_count > 0) {
                options_menu_count--;
            }
            else {
                options_menu_count = 0;
            }
        } 
        else {
            options_menu_count++;
        }
    }
    options_menu_current = options_menu_count / clicks_per_move_option;

    if (options_menu_count > MAX_OPTIONS_MENU * clicks_per_move_option) {
        options_menu_count = (MAX_OPTIONS_MENU * clicks_per_move_option) - 1; 
    }

    if (options_menu_current >= MAX_OPTIONS_MENU) {
        options_menu_current = MAX_OPTIONS_MENU - 1;
    }

    if (numclicksY) {
        point cur;
        cur = imageptr->get_current_loc();
        switch (options_menu_current) {
            case 0:
#ifdef SQSK_DEBUG
                Serial.println("Selected CPM");
#endif
                set_machinestate(CLICKSMODE);
                break;
            case 1:
#ifdef SQSK_DEBUG
                Serial.println("Selected Debounce");
#endif
                set_machinestate(DEBOUNCEMODE);
                break;
            case 2:
#ifdef SQSK_DEBUG
                Serial.println("Selected Long Click");
#endif
                set_machinestate(LONGCLICKMODE);
                break;
            case 3:
#ifdef SQSK_DEBUG
                Serial.println("Selected Reset All");
#endif
                set_machinestate(RESETMODEOK);
                break;
            case 4:
#ifdef SQSK_DEBUG
                Serial.println("Selected Return To Main");
#endif
                axisencoder_setup_again(cur.x,cur.y);
                menu_count = 6 * clicks_per_move_option;
                set_machinestate(MAINMENU);
                break;
            default:
                set_machinestate(ERRORMODE);
                break;
        }
        delay(100);
    }

    yinitState = ycurrentState;       
}

/*******************************************************************************
    function name: handleIndOptionsX
    function desc: Handle individual options settings X Encoder
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleIndOptionsX(void) {
    if (xcurrentState != xinitState  && xcurrentState == 1) {
        if (digitalRead(X_ROTARY_PIN2) != xcurrentState) {
            if (machinestate == CLICKSMODE) {
                if (clicks_per_move_option > MIN_CHOICE_CLICKS) {
                    clicks_per_move_option--;
                }
                else {
                    clicks_per_move_option = MIN_CHOICE_CLICKS;
                }
            }
            else if (machinestate == DEBOUNCEMODE) {
                if (debounce_time_option > MIN_CHOICE_DEBOUNCE) {
                    debounce_time_option -= 5;
                }
                else {
                    debounce_time_option = MIN_CHOICE_DEBOUNCE;
                }
            }
            else if (machinestate == LONGCLICKMODE) {
                if (longpress_time_option > MIN_CHOICE_LONGCLICKS) {
                    longpress_time_option -= 10;
                }
                else {
                    longpress_time_option = MIN_CHOICE_LONGCLICKS;
                }
            }
        }
        else {
            if (machinestate == CLICKSMODE) {
                clicks_per_move_option++;
                if (clicks_per_move_option > MAX_CHOICE_CLICKS)
                    clicks_per_move_option = MAX_CHOICE_CLICKS;
            }
            else if (machinestate == DEBOUNCEMODE) {
                debounce_time_option += 5;
                if (debounce_time_option > MAX_CHOICE_DEBOUNCE)
                    debounce_time_option = MAX_CHOICE_DEBOUNCE;
            }
            else if (machinestate == LONGCLICKMODE) {
                longpress_time_option += 10;
                if (longpress_time_option > MAX_CHOICE_LONGCLICKS)
                    longpress_time_option = MAX_CHOICE_LONGCLICKS;
            }
        }
    }

    // Remember last CLK state for next cycle
    xinitState = xcurrentState;       
    if (numclicksX) {
        uint16_t tmp;
        switch (machinestate) {
            case CLICKSMODE:
                tmp = eepromptr->get_option(CLICKS);
                if (tmp != clicks_per_move_option) {
                    eepromptr->set_option(CLICKS, clicks_per_move_option);
                    options_menu_count = ((uint8_t)machinestate - 9) * clicks_per_move_option;
                    menu_count = 6 * clicks_per_move_option;
                    point cur;
                    cur = imageptr->get_current_loc();
                    axisencoder_setup_again(cur.x,cur.y);
                }
                break;
            case DEBOUNCEMODE:
                tmp = eepromptr->get_option(DEBOUNCE);
                if (tmp != debounce_time_option) {
                    eepromptr->set_option(DEBOUNCE, debounce_time_option);
                }
                break;
            case LONGCLICKMODE:
                tmp = eepromptr->get_option(LONG);
                if (tmp != longpress_time_option) {
                    eepromptr->set_option(LONG, longpress_time_option);
                }
                break;
            default:
                set_machinestate(ERRORMODE);
                break;
        }
        set_machinestate(OPTIONSMODE);
    }
    
    cleardisplay = true;
}

/*******************************************************************************
    function name: handleIndOptionsY
    function desc: Handle individual options settings Y Encoder
    inputs:
    outputs:
    returns:
*******************************************************************************/
void handleIndOptionsY(void) {
    if (ycurrentState != yinitState  && ycurrentState == 1) {
        if (digitalRead(Y_ROTARY_PIN2) != ycurrentState) {
            if (machinestate == CLICKSMODE) {
                if (clicks_per_move_option > MIN_CHOICE_CLICKS) {
                    clicks_per_move_option--;
                }
                else {
                    clicks_per_move_option = MIN_CHOICE_CLICKS;
                }
            }
            else if (machinestate == DEBOUNCEMODE) {
                if (debounce_time_option > MIN_CHOICE_DEBOUNCE) {
                    debounce_time_option -= 5;
                }
                else {
                    debounce_time_option = MIN_CHOICE_DEBOUNCE;
                }
            }
            else if (machinestate == LONGCLICKMODE) {
                if (longpress_time_option > MIN_CHOICE_LONGCLICKS) {
                    longpress_time_option -= 10;
                }
                else {
                    longpress_time_option = MIN_CHOICE_LONGCLICKS;
                }
            }
        }
        else {
            if (machinestate == CLICKSMODE) {
                clicks_per_move_option++;
                if (clicks_per_move_option > MAX_CHOICE_CLICKS)
                    clicks_per_move_option = MAX_CHOICE_CLICKS;
            }
            else if (machinestate == DEBOUNCEMODE) {
                debounce_time_option += 5;
                if (debounce_time_option > MAX_CHOICE_DEBOUNCE)
                    debounce_time_option = MAX_CHOICE_DEBOUNCE;
            }
            else if (machinestate == LONGCLICKMODE) {
                longpress_time_option += 10;
                if (longpress_time_option > MAX_CHOICE_LONGCLICKS)
                    longpress_time_option = MAX_CHOICE_LONGCLICKS;
            }
        }
    }

    // Remember last CLK state for next cycle
    yinitState = ycurrentState;       
    if (numclicksY) {
        uint16_t tmp;
        switch (machinestate) {
            case CLICKSMODE:
                tmp = eepromptr->get_option(CLICKS);
                if (tmp != clicks_per_move_option) {
                    eepromptr->set_option(CLICKS, clicks_per_move_option);
                    options_menu_count = ((uint8_t)machinestate - 9) * clicks_per_move_option;
                    menu_count = 6 * clicks_per_move_option;
                    point cur;
                    cur = imageptr->get_current_loc();
                    axisencoder_setup_again(cur.x,cur.y);
                }
                break;
            case DEBOUNCEMODE:
                tmp = eepromptr->get_option(DEBOUNCE);
                if (tmp != debounce_time_option) {
                    eepromptr->set_option(DEBOUNCE, debounce_time_option);
                }
                break;
            case LONGCLICKMODE:
                tmp = eepromptr->get_option(LONG);
                if (tmp != longpress_time_option) {
                    eepromptr->set_option(LONG, longpress_time_option);
                }
                break;
            default:
                set_machinestate(ERRORMODE);
                break;
        }
        set_machinestate(OPTIONSMODE);
    }
    
    cleardisplay = true;
}
