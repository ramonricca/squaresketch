//==============================================================================
//    File Name:    square_sketch.cpp
//    Description:  Main Application Loop Square Sketch
//==============================================================================
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono12pt7b.h>
#include <string.h>
#include "square_sketch.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint16_t bufsize = SCREEN_WIDTH * ((SCREEN_HEIGHT + 7) / 8);

SquareSketchEEPROM myeeprom;
SquareSketchPicture myimage;

uint8_t cursor_count = 0;
bool cleardisplay = true;
bool draw_initialized = false;
bool load_buffer = false;
uint8_t *imagebuffer; 

/*******************************************************************************
    function name: gfx_prepare
    function desc: initializes graphics for text display
    inputs:
    outputs:
    returns:
*******************************************************************************/
void gfx_prepare(void) {
    display.setFont();
    display.setTextSize(0.5);             // Normal 1:1 pixel scale
    display.setTextColor(1);
    display.setCursor(0,0);             // Start at top-left corner

}

/*******************************************************************************
    function name: draw
    function desc: main draw routine for all machine states
    inputs:
    outputs:
    returns:
*******************************************************************************/
void draw() {
    if (cleardisplay) {
        display.clearDisplay();
        cleardisplay = false;
        delay(100);
    }
    
    char textbuf[22];
    char textoutput[22];
    switch (get_machinestate()) {
        case DRAWMODE :
        {
            if (load_buffer) {
                display.clearDisplay();
                load_sketch();
                load_buffer = false;
            }
            point current = myimage.get_current_loc();
            if (cursor_count % (cursor_modulus[0])) {
                display.drawPixel(current.x, current.y, SSD1306_WHITE);
            }
            else {
                display.drawPixel(current.x, current.y, SSD1306_BLACK);
            }
        }
        break;
            
        case MAINMENU :
        {
            gfx_prepare();            
            for( uint8_t i = 0; i < MAX_MAIN_MENU; i++ ) { 
                strcpy_P(textbuf, (PGM_P)pgm_read_word(&(MainMenu[i])));
                if ( i == get_menu_current() ) { 
                    display.setTextColor(0, 1);
                }
                else {
                    display.setTextColor(1, 0);
                }
                sprintf(textoutput, "%-21s", textbuf);
                display.println(textoutput);
            }
        }
        break;
 
        case ABOUTMODE :
        {
            gfx_prepare();
            for (uint8_t i=0; i<7; i++) {
                strcpy_P(textbuf, (PGM_P)pgm_read_word(&(about_table[i])));
                sprintf(textoutput, "%-21s", textbuf);
                display.println(textoutput);
            }
        }
        break;
            
        case SPLASH :
            gfx_prepare();
            display.drawBitmap((SCREEN_WIDTH - logo_width) / 2, (SCREEN_HEIGHT - logo_height) / 2, logo_data, logo_width, logo_height, 1);
            for (uint8_t i=0; i < 6; i++) {
                display.println();
            }
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(splash_table[0])));
            display.println(textbuf);
            break;
            
        case CLEARSCR :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[0])));
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[5])));
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
            draw_initialized = false;
        }
        break;
        
        case SAVEEEPROM :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[7])));
            display.println(textbuf);
            display.display();
            write_to_eeprom();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[1])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[5])));
            display.println(textbuf);
        }
        break;
        
        case LOADEEPROM :
        {
            uint16_t tmpclick;
            tmpclick = get_clicks_per_move_option();
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[8])));
            display.println(textbuf);
            display.display();
            read_from_eeprom();
            setCurrentPixel(64*tmpclick,32*tmpclick);
            myimage.reset_pen();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[2])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[5])));
            display.println(textbuf);
            draw_initialized = true;
        }
        break;
        
        case CLEAREEPROM :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[9])));
            display.println(textbuf);
            display.display();
            clear_eeprom();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[3])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[5])));
            display.println(textbuf);
        }
        break;

        case OPTIONSMODE :
        {
            gfx_prepare();
            for (uint8_t i=0; i<MAX_OPTIONS_MENU; i++) {
                strcpy_P(textbuf, (PGM_P)pgm_read_word(&(options_table[i])));
                if ( i == get_options_menu_current() ) { 
                    display.setTextColor(0, 1);
                }
                else {
                    display.setTextColor(1, 0);
                }
                sprintf(textoutput, "%-21s", textbuf);
                display.println(textoutput);
            }
        }
        break;
        case CLICKSMODE :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(options_choice[0])));
            char str[4];
            sprintf(str, "%d", get_clicks_per_move_option());
            strcat(textbuf, (const char*)&str);
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
        }
        break;
        case DEBOUNCEMODE :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(options_choice[1])));
            char str[4];
            sprintf(str, "%d", get_debounce_time_option());
            strcat(textbuf, (const char*)&str);
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
       }
        break;
        case LONGCLICKMODE :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(options_choice[2])));
            char str[4];
            sprintf(str, "%d", get_longpress_time_option());
            strcat(textbuf, (const char*)&str);
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
        }
        break;
        case RESETMODE :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[4])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[5])));
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
        }
        break;
        case ERRORMODE :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[6])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(modes_table[5])));
            sprintf(textoutput, "%-21s", textbuf);
            display.println(textoutput);
        }
        break;
        case CLEARSCROK :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[2])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[1])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[0])));
            display.println(textbuf);            
        }
        break;
        case SAVEEEPROMOK :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[3])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[1])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[0])));
            display.println(textbuf);            
        }
        break;
        case LOADEEPROMOK :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[4])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[1])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[0])));
            display.println(textbuf);            
        }
        break;
        case CLEAREEPROMOK :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[5])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[1])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[0])));
            display.println(textbuf);            
        }
        break;
        case RESETMODEOK :
        {
            gfx_prepare();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[6])));
            display.println(textbuf);
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[1])));
            display.println(textbuf);
            display.println();
            strcpy_P(textbuf, (PGM_P)pgm_read_word(&(confirm_table[0])));
            display.println(textbuf);            
        }
        break;
     }
    display.display();
 
}

/*******************************************************************************
    function name: setup
    function desc: main setup function
    inputs:
    outputs:
    returns:
*******************************************************************************/
void setup() {
    Serial.begin(115200);
#ifdef SQSK_DEBUG
    Serial.println("setup();");
#endif
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

    imagebuffer = (uint8_t *)malloc(bufsize);

    // Clear the buffer
    display.clearDisplay();
    display.setFont(&FreeMono12pt7b);
    display.setCursor(0, 0); // Back to top-left corner
    display.setTextWrap(false); // Allow spaces to go off right edge
    myeeprom.begin(bufsize);
    axisencoder_setup(64,32, &myimage, &myeeprom);
    pinMode(PEN_LED, OUTPUT);
}

/*******************************************************************************
    function name: loop
    function desc: main loop
    inputs:
    outputs:
    returns:
*******************************************************************************/
void loop() {
    draw();
    machine_state macstate = get_machinestate();
    switch (macstate) {
        case SPLASH :
            delay(SPLASHDELAY);
            display.clearDisplay();
            set_machinestate(MAINMENU);
            break;
        case DRAWMODE :
            cursor_count++;
            if (cursor_count >= (PENDOWN*4)) {
                cursor_count = 0;
            }
 
            switch(myimage.what_pen()) {
                case 0:
                    digitalWrite(PEN_LED, LOW);
                    break;
                case 1:
                    digitalWrite(PEN_LED, HIGH);
                    break;
                case 2:
                    if (cursor_count > (PENDOWN*2)) {
                        digitalWrite(PEN_LED, HIGH);
                    }
                    else {
                        digitalWrite(PEN_LED, LOW);
                    }
                    break;
                default:
                    break;
            }
            break;
        case CLEARSCR :
        {
            clear_save_buffer();
            myimage.reset_pen();
            axisencoder_setup_again(64,32);
            uint8_t cpm = get_clicks_per_move_option();
            setCurrentPixel(64*cpm,32*cpm);
        }
        break;
        case RESETMODE :
        {
            point cur;
            cur = myimage.get_current_loc();

            myeeprom.set_option(CLICKS, DEF_CHOICE_CLICKS);
            myeeprom.set_option(DEBOUNCE, DEF_CHOICE_DEBOUNCE);
            myeeprom.set_option(LONG, DEF_CHOICE_LONGCLICKS);
            set_clicks_per_move_option(DEF_CHOICE_CLICKS);
            set_debounce_time_option(DEF_CHOICE_DEBOUNCE);
            set_longpress_time_option(DEF_CHOICE_LONGCLICKS);
            axisencoder_setup_again(cur.x,cur.y);
            set_options_menu_count(3 * DEF_CHOICE_CLICKS);
        }
        break;
        default:
            digitalWrite(PEN_LED, LOW);
            break;
    }
}

/*******************************************************************************
    function name: drawPixelWithPen
    function desc: draw the pixel with current pen state
    inputs:
    outputs:
    returns:
*******************************************************************************/
void drawPixelWithPen(point pt) {
    uint8_t current_pen = myimage.what_pen();
#ifdef SQSK_DEBUG
    Serial.print("Pen: ");
    Serial.println(current_pen);
    Serial.print("drawPixelWithPen ");
    Serial.println(current_pen);
#endif
    if (current_pen == 1) {
        display.drawPixel(pt.x, pt.y, SSD1306_WHITE);
    }
    else if (current_pen == 2) {
        display.drawPixel(pt.x, pt.y, SSD1306_BLACK);
    }
}

/*******************************************************************************
    function name: drawPixelPrev
    function desc: draw pixel to what it was originally
    inputs:
    outputs:
    returns:
*******************************************************************************/
void drawPixelPrev(point pt) {
#ifdef SQSK_DEBUG
    Serial.print("Prev Pixel: ");
    Serial.println(pt.is_drawn);
    Serial.print("drawPixelPrev is_drawn:");
    Serial.println(pt.is_drawn );
#endif
     if (pt.is_drawn == 1) {
        display.drawPixel(pt.x, pt.y, SSD1306_WHITE);
    }
    else {
        display.drawPixel(pt.x, pt.y, SSD1306_BLACK);
    }
}

/*******************************************************************************
    function name: write_to_eeprom
    function desc: copy buffer to eeprom
    inputs:
    outputs:
    returns:
*******************************************************************************/
void write_to_eeprom(void) {
    myeeprom.Buffer_to_EEPROM(imagebuffer);
}

/*******************************************************************************
    function name: read_from_eeprom
    function desc: copy eeprom to buffer
    inputs:
    outputs:
    returns:
*******************************************************************************/
void read_from_eeprom(void) {
    myeeprom.EEPROM_to_Buffer(imagebuffer);
}

/*******************************************************************************
    function name: clear_eeprom
    function desc: erase eeprom image
    inputs:
    outputs:
    returns:
*******************************************************************************/
void clear_eeprom(void) {
    myeeprom.Reset_EEPROM();
}

/*******************************************************************************
    function name: save_sketch
    function desc: save framebuffer to memory
    inputs:
    outputs:
    returns:
*******************************************************************************/
void save_sketch(void) {
    memcpy(imagebuffer, display.getBuffer(), bufsize);
}

/*******************************************************************************
    function name: load_sketch
    function desc: load framebuffer from memory
    inputs:
    outputs:
    returns:
*******************************************************************************/
void load_sketch(void) {
    memcpy(display.getBuffer(), imagebuffer, bufsize);
}

/*******************************************************************************
    function name: clear_save_buffer
    function desc: erase save buffer
    inputs:
    outputs:
    returns:
*******************************************************************************/
void clear_save_buffer(void) {
    memset(imagebuffer, 0, bufsize);
}

/*******************************************************************************
    function name: setCurrentPixel
    function desc: set current location with original pixel
    inputs:
    outputs:
    returns:
*******************************************************************************/
void setCurrentPixel(uint16_t x, uint16_t y) {   
    uint8_t cpm;
    cpm = get_clicks_per_move_option();
    uint8_t xx;
    uint8_t yy;
    xx = x / (uint16_t)cpm;
    yy = y / (uint16_t)cpm;
    
    myimage.set_current_loc(xx, yy, (display.getPixel(xx,yy) == 1 ? 1 : 0));
}

/*******************************************************************************
    function name: correctCursor
    function desc: try to avoid problems with cursor blinking artifacts
    inputs:
    outputs:
    returns:
*******************************************************************************/
void correctCursor(void) {
    point current = myimage.get_current_loc();
    display.drawPixel(current.x, current.y, current.is_drawn);
}