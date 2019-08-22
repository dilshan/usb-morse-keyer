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

#include "morse.h"
#include "pwm.h"

void unitDelay()
{
    switch((systemConfig >> OPT_SPEED) & 0x03)
    {
        case 1:     // 120ms (10WPM)
            __delay_ms(105);
            break;
        case 2:     // 80ms (15WPM)
            __delay_ms(65);
            break;
        default:    // 240ms (5WPM)
            __delay_ms(225);
    }
}

void unitDelayEx(unsigned char unitCount)
{
    unsigned char delayPos;
    
    for(delayPos = 0; delayPos < unitCount; delayPos++)
    {
        unitDelay();
    }
}

void dot()
{
    enablePulse();
    unitDelay();
    disablePulse();
    unitDelayEx(3);
}

void dash()
{
    enablePulse();
    unitDelay();
    unitDelay();
    unitDelay();
    disablePulse();
    unitDelayEx(3);
}

void encodeCharacter(unsigned char character)
{
    // Convert lower case character to upper case.
    if((character > 96) && (character < 123))
    {
        character -= 32; 
    }
    
    switch(character)
    {
        case 65:    // A
            dit; dah;
            break;
        case 66:    // B
            dah; dit; dit; dit;
            break;  
        case 67:    // C
            dah; dit; dah; dit;
            break;
        case 68:    // D
            dah; dit; dit;
            break;
        case 69:    // E
            dit;
            break;
        case 70:    // F
            dit; dit; dah; dit;
            break;
        case 71:    // G
            dah; dah; dit;
            break;
        case 72:    // H
            dit; dit; dit; dit;
            break;
        case 73:    // I
            dit; dit;
            break;
        case 74:    // J
            dit; dah; dah; dah;
            break;
        case 75:    // K
            dah; dit; dah;
            break;
        case 76:    // L
            dit; dah; dit; dit;
            break;
        case 77:    // M
            dah; dah;
            break;
        case 78:    // N
            dah; dit;
            break;
        case 79:    // O
            dah; dah; dah;
            break;
        case 80:    // P
            dit; dah; dah; dit;
            break;
        case 81:    // Q
            dah; dah; dit; dah;
            break;
        case 82:    // R
            dit; dah; dit;
            break;
        case 83:    // S
            dit; dit; dit;
            break;
        case 84:    // T
            dah;
            break;
        case 85:    // U
            dit; dit; dah;
            break;
        case 86:    // V
            dit; dit; dit; dah;
            break;
        case 87:    // W
            dit; dah; dah;
            break;
        case 88:    // X
            dah; dit; dit; dah;
            break;
        case 89:    // Y
            dah; dit; dah; dah;
            break;
        case 90:    // Z
            dah; dah; dit; dit;
            break;
            
        case 48:    // 0
            dah; dah; dah; dah; dah;
            break;
        case 49:    // 1
            dit; dah; dah; dah; dah;
            break;
        case 50:    // 2 
            dit; dit; dah; dah; dah;
            break;
        case 51:    // 3
            dit; dit; dit; dah; dah;
            break;
        case 52:    // 4
            dit; dit; dit; dit; dah;
            break;
        case 53:    // 5
            dit; dit; dit; dit; dit;
            break;
        case 54:    // 6
            dah; dit; dit; dit; dit;
            break;
        case 55:    // 7
            dah; dah; dit; dit; dit;
            break;
        case 56:    // 8
            dah; dah; dah; dit; dit;
            break;
        case 57:    // 9
            dah; dah; dah; dah; dit;
            break;
            
        case 32:    // SPACE
            unitDelayEx(4);
            break;
    }
}

void initMorseBuffer(morseBuffer *buffer)
{
    unsigned char bufferPos;
    
    for(bufferPos = 0; bufferPos < MORSE_BUFFER_SIZE; bufferPos++)
    {
        buffer->morseCodeBuffer[bufferPos] = 0;
    }
    
    buffer->bufferPos = 0;
}

// Implementation of initMorseBuffer for ISRs.
void initMorseBufferISR(morseBuffer *buffer)
{
    unsigned char bufferPos;
    
    for(bufferPos = 0; bufferPos < MORSE_BUFFER_SIZE; bufferPos++)
    {
        buffer->morseCodeBuffer[bufferPos] = 0;
    }
    
    buffer->bufferPos = 0;
}

unsigned char updateMorseBuffer(morseBuffer *buffer, char morseCode)
{
    if(morseCode != CODE_EMPTY)
    {
        if(buffer->bufferPos >= MORSE_BUFFER_SIZE)
        {
            // More buffer is full.
            return 1;
        }

        buffer->morseCodeBuffer[buffer->bufferPos] = morseCode;
        buffer->bufferPos++;
    }
    
    return 0;
}

unsigned char decodeCharacter(morseBuffer *buffer)
{
    unsigned char tempBuffer = 0;
    unsigned char tempPos = (buffer->bufferPos - 1);
    unsigned char tempData;
    unsigned char shfitPos = 0;
    
    // Check for empty morse buffer.
    if(buffer->bufferPos == 0)
    {
        return 0;
    }
    
    // Reverse content of the morse buffer to start decoding.
    while(shfitPos < buffer->bufferPos)
    {
        tempData = (buffer->morseCodeBuffer[tempPos--] == CODE_DOT) ? 0 : 1;
        tempBuffer |= tempData << shfitPos;
        shfitPos++;
    }
    
    if(buffer->bufferPos == 1)
    {
        // Handle more codes with single symbol.
        switch(tempBuffer)
        {
            case 0:     // .
                return 69;          // E
            case 1:     // -
                return 84;          // T
        }
    }
    else if(buffer->bufferPos == 2)
    {
        // Handle more codes with 2 symbols.
        switch(tempBuffer)
        {
            case 0:     // ..
                return 73;          // I
            case 1:     // .-
                return 65;          // A
            case 2:     // -.
                return 78;          // N
            case 3:     // --
                return 77;          // M
        }
    }
    else if(buffer->bufferPos == 3)
    {
        // Handle more codes with 3 symbols.
        switch(tempBuffer)
        {
            case 0:     // ...
                return 83;          // S
            case 1:     // ..-
                return 85;          // U
            case 2:     // .-.
                return 82;          // R
            case 3:     // .--
                return 87;          // W
            case 4:     // -..
                return 68;          // D
            case 5:     // -.-
                return 75;          // K
            case 6:     // --.
                return 71;          // G
            case 7:     // ---
                return 79;          // 0
        }
    }
    else if(buffer->bufferPos == 4)
    {
        // Handle more codes with 4 symbols.
        switch(tempBuffer)
        {
            case 0:     // ....
                return 72;         // H
            case 1:     // ...-
                return 86;         // V 
            case 2:     // ..-.
                return 70;         // F
            case 4:     // .-..
                return 76;         // L
            case 6:     // .--.
                return 80;         // P
            case 7:     // .---
                return 74;         // J
            case 8:     // -...
                return 66;         // B
            case 9:     // -..-
                return 88;         // X
            case 10:    // -.-.
                return 67;         // C
            case 11:    // -.--
                return 89;         // Y
            case 12:    // --..
                return 90;         // Z
            case 13:    // --.-
                return 81;         // Q
            default:    // ---.
                return 63;         // ? (unknown)
        }
    }
    else if(buffer->bufferPos == 5)
    {
        // Handle more codes with 5 symbols.
        switch(tempBuffer)
        {
            case 15:     // .----
                return 49;          // 1
            case 7:     // ..---
                return 50;          // 2
            case 3:     // ...--
                return 51;          // 3
            case 1:     // ....-
                return 52;          // 4
            case 0:     // .....
                return 53;          // 5
            case 16:     // -....
                return 54;          // 6
            case 24:     // --...
                return 55;          // 7
            case 28:     // ---..
                return 56;          // 8
            case 30:     // ----.
                return 57;          // 9
            case 31:     // -----
                return 48;          // 0
        }
    }
    
    // Unknown symbol.
    return 63;
}
