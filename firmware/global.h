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

#ifndef GLOBAL_H
#define	GLOBAL_H

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT    // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF               // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON               // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON               // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF                 // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON               // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF               // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON               // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF                // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR21V   // Brown-out Reset Selection bit (Brown-out Reset set to 2.1V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#define _XTAL_FREQ 8000000

#include <xc.h>

#define TRUE    0xFF
#define FALSE   0x00

#define MAX_SHORT   0xFFFF
#define MAX_BYTE    0xFF

#define RING_BUFFER_SIZE    64
#define MORSE_BUFFER_SIZE   6

#define OPT_INPUT_MODE      0
#define OPT_KEYER_TYPE      2
#define OPT_SPEED           4
#define OPT_SPEAKER_OUT     6
#define OPT_LOOP_SEND       8
#define OPT_TONE_TYPE       10

volatile typedef struct 
{
    unsigned char morseBuffer[RING_BUFFER_SIZE];
    unsigned char writePos;
    unsigned char readPos;
} ringBuffer;

volatile typedef struct
{
    char morseCodeBuffer[MORSE_BUFFER_SIZE];
    unsigned char bufferPos;
} morseBuffer;

unsigned short systemConfig = 0x00;
unsigned char shadowPortC = 0x00;

#endif	/* GLOBAL_H */

