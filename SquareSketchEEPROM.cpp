//==============================================================================
//    File Name:    SquareSketchEEPROM.cpp
//    Description:  EEPROM Handling for SquareSketch
//==============================================================================
#include "SquareSketchEEPROM.h"

/*******************************************************************************
    function name: SquareSketchEEPROM
    function desc: constructor
    inputs:
    outputs:
    returns:
*******************************************************************************/
SquareSketchEEPROM::SquareSketchEEPROM()
{
    //Wire.printStatus(Serial);
}

/*******************************************************************************
    function name: begin
    function desc: initialize object
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::begin(uint16_t bufsize) {
#ifdef SQSK_DEBUG
    Serial.println("EEPROM.begin();");
#endif
    this->buffer_size = bufsize;
    this->FIRST_ADDRESS = NUMPARAMS;
}

/*******************************************************************************
    function name: Reset_EEPROM
    function desc: clear image
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::Reset_EEPROM() {
#ifdef SQSK_DEBUG
    Serial.println("EEPROM.Reset_EEPROM();");
#endif
    
    for(uint16_t i = 0; i < (this->buffer_size + NUMPARAMS); i++) {
        eeprom_write_byte((uint8_t*)(i), 0x00); 
    }
}

/*******************************************************************************
    function name: read
    function desc: read a byte from eeprom
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::read(const uint16_t address, uint8_t *value)
{
#ifdef SQSK_DEBUG
    Serial.println("EEPROM.read();");
#endif
    
    *value = eeprom_read_byte((uint8_t *)address);
}

/*******************************************************************************
    function name: write
    function desc: write a byte to eeprom
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::write(const uint16_t address, const uint8_t value) 
{
#ifdef SQSK_DEBUG
    Serial.println("EEPROM.write();");
#endif
    
    eeprom_write_byte((uint8_t *)address, value);
}

/*******************************************************************************
    function name: Buffer_to_EEPROM
    function desc: copy entire buffer to eeprom
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::Buffer_to_EEPROM(uint8_t *the_buffer) {
    for(uint16_t i=0; i < this->buffer_size; i++) {
        uint16_t address = (uint16_t)(this->FIRST_ADDRESS + i);
        write(address, *the_buffer);
        the_buffer++;
    }
}

/*******************************************************************************
    function name: EEPROM_to_Buffer
    function desc: copy eeprom to buffer
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::EEPROM_to_Buffer(uint8_t *the_buffer) {
    for(uint16_t i=0; i < this->buffer_size; i++) {
        uint16_t address = (uint16_t)(this->FIRST_ADDRESS + i);
        read(address, the_buffer);
        the_buffer++;
    }
}

/*******************************************************************************
    function name: set_option
    function desc: save option in eeprom
    inputs:
    outputs:
    returns:
*******************************************************************************/
void SquareSketchEEPROM::set_option(optionindex opt, uint16_t value) {
#ifdef SQSK_DEBUG
    Serial.print("setting option ");
    Serial.println(opt);
#endif
    uint16_t address;
    address = (uint16_t)(opt*2);
    eepromoption mydefault;
    mydefault.int16 = value;
    eeprom_update_byte((uint8_t*)address, mydefault.int8[1]);
    address++;
    eeprom_update_byte((uint8_t*)address, mydefault.int8[0]);
}
    
/*******************************************************************************
    function name: get_option
    function desc: get option from eeprom or set defaults
    inputs:
    outputs:
    returns:
*******************************************************************************/
uint16_t SquareSketchEEPROM::get_option(optionindex opt) {
#ifdef SQSK_DEBUG
    Serial.print("getting option ");
    Serial.println((uint8_t)opt);
#endif
    uint16_t address;
    address = (uint16_t)(opt*2);
    uint16_t origaddress = address;
    
    eepromoption value;
    
    value.int8[1] = eeprom_read_byte((uint8_t*)address);
    address++;
    value.int8[0] = eeprom_read_byte((uint8_t*)address);
#ifdef SQSK_DEBUG
    Serial.print("Read value ");
    Serial.println(value.int16, HEX);
#endif 
    
    if (opt == CLICKS) {
        if ((value.int16 < MIN_CHOICE_CLICKS) || (value.int16 > MAX_CHOICE_CLICKS)) {
#ifdef SQSK_DEBUG
            Serial.println("Setting clicks ");
#endif
            eepromoption mydefault;
            mydefault.int16 = DEF_CHOICE_CLICKS;
            eeprom_update_byte((uint8_t*)origaddress, mydefault.int8[1]);
            origaddress++;
            eeprom_update_byte((uint8_t*)origaddress, mydefault.int8[0]);
            return mydefault.int16;
        }
        else {
            return value.int16;
        }
    }
    else if (opt == DEBOUNCE) {
        if ((value.int16 < MIN_CHOICE_DEBOUNCE) || (value.int16 > MIN_CHOICE_DEBOUNCE)) {
#ifdef SQSK_DEBUG
            Serial.println("Setting debounce ");
#endif
            eepromoption mydefault;
            mydefault.int16 = DEF_CHOICE_DEBOUNCE;
            eeprom_update_byte((uint8_t*)origaddress, mydefault.int8[1]);
            origaddress++;
            eeprom_update_byte((uint8_t*)origaddress, mydefault.int8[0]);
            return mydefault.int16;
        }
        else {
            return value.int16;
        }
    }
    else if (opt == LONG){
        if ((value.int16 < MIN_CHOICE_LONGCLICKS) || (value.int16 > MAX_CHOICE_LONGCLICKS)) {
#ifdef SQSK_DEBUG
            Serial.println("Setting long ");
#endif
            eepromoption mydefault;
            mydefault.int16 = DEF_CHOICE_LONGCLICKS;
            eeprom_update_byte((uint8_t*)origaddress, mydefault.int8[1]);
            origaddress++;
            eeprom_update_byte((uint8_t*)origaddress, mydefault.int8[0]);
            return mydefault.int16;
        }
        else {
#ifdef SQSK_DEBUG
            Serial.print("Returning ");
            Serial.println(value.int16, HEX);
#endif
            return value.int16;
        }
    }
    
    return 0;
}
