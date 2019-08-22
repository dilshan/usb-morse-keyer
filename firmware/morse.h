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

#ifndef MORSE_H
#define	MORSE_H

#include "global.h"

#define CODE_DOT    1
#define CODE_DASH   3
#define CODE_EMPTY  0

#define dit dot()
#define dah dash()

void unitDelay(void);
void unitDelayEx(unsigned char unitCount);

void dot(void);
void dash(void);

void encodeCharacter(unsigned char character);

void initMorseBuffer(morseBuffer *buffer);
void initMorseBufferISR(morseBuffer *buffer);

unsigned char updateMorseBuffer(morseBuffer *buffer, char morseCode);
unsigned char decodeCharacter(morseBuffer *buffer);

#endif	/* MORSE_H */

