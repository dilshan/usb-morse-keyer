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

#ifndef MAIN_H
#define	MAIN_H

#include "global.h"

#define ROTARY_ENCODER_END  127
#define SLEEP_TIME_LIMIT    15000

#define BTN_ROTARY_ENCODER  0x04
#define BTN_PTT_OVERRIDE    0x20
#define BTN_MEM_MANAGER     0x40

#define PORTB_MASK  0xFC

#define IS_BUTTON_PRESS(id) (((lastInputStatus & id) == 0x00) && (currentInputStatus & id) == id)

volatile signed char encoderPosition = 0;
volatile unsigned short sleepCounter = 0;

unsigned char lastInputStatus = MAX_BYTE;
unsigned char currentInputStatus = MAX_BYTE;
unsigned char lastEncoderVal = 0;

unsigned char keyerPortMask = 0;
unsigned char operatingMode = 0;
unsigned char keyerTypeId = 0;
unsigned char keySpeed = 0;
unsigned char toneType = 0;
unsigned char loopMessage = 0;

unsigned char pttOverride = 0;
unsigned char tempDecodeChar = 0;

ringBuffer dataBuffer;
morseBuffer morseCodeBuffer;

void initSystem(void);
void enableInterrupts(void);
void initUART(void);

void systemMenuHandler(void);
void memoryKeyHandler(void);

void generateMorseOutput(void);

void updateSystemSettings(void);

#endif	/* MAIN_H */

