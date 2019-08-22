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

#include "lcd1602.h"

void sendCommand(unsigned char cmd)
{
    PORTA &= 0x03;
    __delay_us(50);
    PORTA |= cmd << 4;
    PORTA |= 0x08;
    __delay_ms(4);
    PORTA &= 0xF7;
}

void initLCD()
{
    PORTA = 0x00;
    __delay_ms(5);
    
    // Try to reset the HD44780 controller.
    sendCommand(0x03);
    __delay_ms(5);
    sendCommand(0x03);
    __delay_ms(15);
    sendCommand(0x03);
    
    // Initialize display with default character set font size.
    sendCommand(0x02);
    sendCommand(0x02);
    sendCommand(0x08);
    sendCommand(0x00);
    sendCommand(0x0C);
    sendCommand(0x00);
    sendCommand(0x06);
}

void clearLCD()
{
    sendCommand(0x00);
    sendCommand(0x01);
    
    displayRow = 1;
    displayCol = 1;
}

void printChar(char value)
{
    char lowWord = (value & 0x0F) << 4;
    
    // Send high value of the byte.
    PORTA |= 0x04;
    PORTA = (PORTA & 0x0F) | (value & 0xF0);
    PORTA |= 0x08;
    
    // Send low value of the byte.
    __delay_us(50);
    PORTA &= 0xF7;
    PORTA = (PORTA & 0x0F) | lowWord;
    PORTA |= 0x08;
    
    __delay_us(50);
    PORTA &= 0xF7;
}

void printStr(char *str) 
{
    unsigned char pos;
    for(pos = 0; str[pos]!='\0'; pos++)
    {
        printChar(str[pos]);
    }
}

void printWindow(char value)
{
    if((++displayCol) > MAX_DISPLAY_LENGTH)
    {
        // End of column reached and switch to the next row of the display.
        displayCol = 1;
        if((++displayRow) == 3)
        {
            // All display rows are filled, lets clear the display.
            displayRow = 1;
            clearLCD();
        }
        
        setCursor(displayRow, displayCol);
    }
    
    printChar(value);
}

void setCursor(unsigned char row, unsigned char col)
{
    unsigned char cmd = (row == 1 ? 0x80 : 0xC0) + col - 1;
    sendCommand(cmd >> 4);
    sendCommand(cmd & 0x0F);
    
    displayRow = row;
    displayCol = col;
}

void clearRow(unsigned char row) 
{
    unsigned char pos;
    setCursor(row, 1);
    for(pos = 0; pos < 16; pos++)
    {
        printChar(' ');
    }
    setCursor(row, 1);
}

void clearScrollBuffer()
{
    unsigned char scrollBufferPos;
    
    scrollPos = 0;
    
    for(scrollBufferPos = 0; scrollBufferPos < MAX_DISPLAY_LENGTH; scrollBufferPos++)
    {
        scrollBuffer[scrollBufferPos] = 32;
    }
    
    scrollBuffer[MAX_DISPLAY_LENGTH] = 0;
}

void printScroll(char value)
{
    unsigned char scrollBufferPos;
    
    setCursor(2,1);
    
    if(scrollPos >= MAX_DISPLAY_LENGTH)
    {
        // Scroll buffer is full. Shift each character of the display to left side.
        for(scrollBufferPos = 1; scrollBufferPos < MAX_DISPLAY_LENGTH; scrollBufferPos++)
        {
            scrollBuffer[scrollBufferPos - 1] = scrollBuffer[scrollBufferPos];
        }
        
        // Append current character into the tail of the scroll buffer.
        scrollBuffer[scrollBufferPos - 1] = value;
    }
    else 
    {
        // Scroll buffer has enough space for character(s).
        scrollBuffer[scrollPos] = value;
        scrollPos++;
    }
    
    // Update display with scroll buffer content.
    printStr(scrollBuffer);
}
