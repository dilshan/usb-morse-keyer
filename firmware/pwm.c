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

#include "pwm.h"
#include "main.h"

void initPWM()
{
    // PWM registers are configured for 750Hz output with 50% duty cycle.
    T2CON = 0x07;
    PR2 = 0xA6;
    CCPR1L = 0x53;
    CCP1CON = 0x20; 
}

void enablePulse()
{
    switch(toneType)
    {
        case 0x00:
            // PTT only.
            shadowPortC |= 0x08;
            PORTC = shadowPortC;
            break;
        case 0x01:
            // Tone (PWM) only.
            CCP1CON = 0x2C;
            break;
        case 0x02:
            // PTT + Tone option.
            shadowPortC |= 0x08;
            PORTC = shadowPortC;
            CCP1CON = 0x2C;
            break;
    }
}

void disablePulse()
{
    // If PTT override is active, keep PTT state as it is. Otherwise release PTT state.
    if(pttOverride != TRUE)
    {
        shadowPortC &= 0xF7;
        PORTC = shadowPortC;
    }
    
    CCP1CON = 0x20;
}
