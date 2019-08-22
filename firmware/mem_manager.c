/******************************************************************************
 * Copyright (C) 2019 Dilshan R Jayakody.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *****************************************************************************/

#include "mem_manager.h"

void saveSystemSettings(unsigned short saveBuffer)
{
    unsigned short tempBuffer;
    
    // Check value of the existing configuration.
    tempBuffer = (eeprom_read(1) << 8) & 0xFF00;
    __delay_ms(5);
    tempBuffer |= eeprom_read(0);
    
    // perfrom E2PROM write only if supplied value is different from existing value.
    if(tempBuffer != saveBuffer)
    {
        eeprom_write(0, saveBuffer & 0x00FF);
        eeprom_write(1, (saveBuffer >> 8) & 0x00FF);
    }
}

unsigned short loadSystemSettings()
{
    unsigned short tempBuffer;
    
    tempBuffer = (eeprom_read(1) << 8) & 0xFF00;
    __delay_ms(5);
    tempBuffer |= eeprom_read(0);
    
    // If E2PROM is empty, switch system to it's default configuration.
    if(tempBuffer == MAX_SHORT)
    {
        tempBuffer = 0x0000;
    }
    
    return tempBuffer;
}

void saveMsgBuffer(unsigned char* buffer, unsigned char channel)
{
    unsigned char memAddr = MEM_MSG_BASE + (channel * (MEM_MSG_SIZE + 1));
    unsigned char memPos = 0;
    
    while(memPos < (MEM_MSG_SIZE + 1))
    {
        eeprom_write(memAddr + memPos, buffer[memPos]);
        __delay_ms(5);
        
        if(buffer[memPos] == END_OF_MESSAGE)
        {
            break;
        }
        
        memPos++;
    }
}
