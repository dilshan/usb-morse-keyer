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

#include "main.h"
#include "lcd1602.h"
#include "uart.h"
#include "pwm.h"
#include "ringbuffer.h"
#include "morse.h"
#include "mem_manager.h"

int main() 
{
    //Initialize all peripherals, global variables and data structures.
    unsigned char currentChar = 0;
    unsigned char isSleep = 0;
    
    shadowPortC = PORTC;
    systemConfig = loadSystemSettings();
    
    initSystem();
    updateSystemSettings();
    
    initRingBuffer(&dataBuffer);
    initMorseBuffer(&morseCodeBuffer);
            
    __delay_ms(20);
    
    // Activate LCD screen and it's the backlight.
    lastInputStatus = PORTB & PORTB_MASK;
    clearLCD();
    setCursor(1, 1);
    
    __delay_ms(10);
    
    // Enable interrupts to serve user actions.
    enableInterrupts();
    
    // Activate LCD backlight.
    shadowPortC |= 0x20;
    PORTC = shadowPortC;
    
    while(1)
    {
        // Continue main service loop if sleep flag is cleared.
        while(isSleep == 0)
        {
            currentInputStatus = PORTB & PORTB_MASK;

            // Rotary encoder button pressed. Open the system menu.
            if(IS_BUTTON_PRESS(BTN_ROTARY_ENCODER))
            {
                PIE1 = 0x00;
                PIR1 = 0x00;
                sleepCounter = 0;
                
                __delay_ms(50);
                systemMenuHandler();
                saveSystemSettings(systemConfig);
                
                enableInterrupts();
                sleepCounter = 0;

                setCursor(1, 1);
                clearLCD();  
            }

            // Check for PTT override key press.
            if(IS_BUTTON_PRESS(BTN_PTT_OVERRIDE))
            {
                pttOverride = ~pttOverride;
                shadowPortC &= 0xF7;

                if(pttOverride == TRUE)
                {
                    shadowPortC |= 0x08; 
                }

                PORTC = shadowPortC;
                sleepCounter = 0;
                __delay_ms(50);
            }

            // Check for memory keying button press.
            if(IS_BUTTON_PRESS(BTN_MEM_MANAGER))
            {
                __delay_ms(150);
                sleepCounter = 0;
                
                memoryKeyHandler();
                sleepCounter = 0;
                
                setCursor(1, 1);
                clearLCD();
            }

            if(operatingMode == 0x0000)
            {
                // System is in USB mode.
                if(popFromBuffer(&dataBuffer, &currentChar) == 0)
                {
                    printWindow(currentChar);
                    encodeCharacter(currentChar);
                    
                    sleepCounter = 0;
                }
            }
            else 
            {
                // System is in KEY mode.
                generateMorseOutput();

                if(popFromBuffer(&dataBuffer, &currentChar) == 0)
                {
                    printWindow(currentChar);
                    
                    sleepCounter = 0;
                }
            }

            // Check for system idle state.
            if(sleepCounter > SLEEP_TIME_LIMIT)
            {
                sleepCounter = 0;
                
                // Mute AF power amplifier and disable all MCU interrupts.
                PORTC = 0x10;
                INTCON = 0x00;    
                PIE1 = 0x00;
                PIR1 = 0x00;
                
                isSleep = TRUE;
                break;
            }

            lastInputStatus = currentInputStatus;
        }
        
        // Entering sleep mode state...
        __delay_ms(200);
        
        // Sleep mode service routine.
        while(isSleep == TRUE)
        {
            // Wake-up system on PORTB status change.
            if((PORTB & 0x7F) != 0x7F)
            {
                isSleep = 0x00;
                sleepCounter = 0;
            
                shadowPortC |= 0x20;
                PORTC = shadowPortC;
                
                enableInterrupts();
                
                __delay_ms(150);
                currentInputStatus = PORTB & PORTB_MASK;
                lastInputStatus = currentInputStatus;
                
                break;
            }
            __delay_ms(10);
        }
    }
}

void generateMorseOutput()
{
    if(keyerTypeId == 0x00)
    {
        // Handle generic morse keyer.
        if((PORTB & keyerPortMask) == keyerPortMask)
        {
            disablePulse();
        }
        else 
        {
            enablePulse();
        }
    }
    else 
    {
        // Handle paddle type morse keyer and emit output based on active 
        // paddle.
        if((PORTB & keyerPortMask) != keyerPortMask)
        {
            if((PORTB & 0x10) == 0x00)
            {
                // Handle dah (dash) with 3 delay units.
                enablePulse();
                unitDelay();
                unitDelay();
                unitDelay();
                disablePulse();
            }
            else
            {
                // Handle dit (dot) with single delay unit.)
                enablePulse();
                unitDelay();
                disablePulse();
            }
        }
    }
}

void isrTimer0()
{
    // Timer 0 - 250Hz (40ms) interrupt handler (reserved for low priority routines).
    if(T0IF)
    {
        // Check rotary encoder status.
        if((PORTB & 0x03) != 0x03)
        {
            if((!RB0) && (lastEncoderVal))
            {
                if(RB1)
                {
                    encoderPosition++;
                }
                else 
                {
                    encoderPosition = (encoderPosition > 0) ? (encoderPosition - 1) : ROTARY_ENCODER_END;
                }
            }
        }
        
        // Increase sleep counter to detect system idle.
        if(sleepCounter < MAX_SHORT)
        {
            sleepCounter++;
        }
        
        // Restore timer 0 with 250Hz timing cycles.
        lastEncoderVal = RB0; 
        TMR0 = 6;
        T0IF = 0;
    }
}

void isrTimer1()
{
    static unsigned char holdCounter = 0;
    static unsigned char lastMorseCode = 0;
    static unsigned char flagChar = TRUE;
    static unsigned char flagWord = TRUE;
    static unsigned char releaseCounter = MAX_BYTE;
    static unsigned char unitDelayRef = 0;
    
    unsigned char tempDecodeChar;
    
    // Timer 1 - 100Hz (10ms) interrupt handler for time based events.
    if(TMR1IF)
    {
        if(operatingMode == 0x0001)
        {
            // Calculate actual delay for delay unit based on selected WPM.
            switch(keySpeed)
            {
                case 1:
                    unitDelayRef = 12;
                    break;
                case 2:
                    unitDelayRef = 8;
                    break;
                default:
                    unitDelayRef = 24;
            }

            if((PORTB & keyerPortMask) == keyerPortMask)
            {
                // KEY UP state.
                
                // Count key release (idle) time.
                if(releaseCounter < MAX_BYTE)
                {
                    releaseCounter++;
                }

                if((releaseCounter > unitDelayRef) && (lastMorseCode != CODE_EMPTY))
                {
                    // End of morse signal reached.
                    updateMorseBuffer(&morseCodeBuffer, lastMorseCode);
                    lastMorseCode = CODE_EMPTY;
                }

                if((releaseCounter > unitDelayRef * 4) && (flagChar == FALSE))
                {
                    // End of character reached.
                    tempDecodeChar = decodeCharacter(&morseCodeBuffer);
                    if(tempDecodeChar > 0)
                    {
                        pushToBuffer(&dataBuffer, tempDecodeChar);
                    }

                    initMorseBufferISR(&morseCodeBuffer);
                    flagChar = TRUE;
                }

                if((releaseCounter > unitDelayRef * 10) && (flagWord == FALSE))
                {
                    // End of word reached and pushed SPACE into the buffer.
                    pushToBuffer(&dataBuffer, 32);
                    flagWord = TRUE;
                }

                // Detect last key down time and decode morse symbol from that.
                if(holdCounter > 0)
                {
                    if(keyerTypeId == 0x0000)
                    {
                        // Generic morse code key handler to determine keyed symbol.
                        lastMorseCode = (holdCounter >= unitDelayRef * 2) ? CODE_DASH : CODE_DOT;
                    }

                    holdCounter = 0;
                }
            }
            else 
            {
                // KEY DOWN state.   
                if(holdCounter < MAX_BYTE)
                {
                    holdCounter++;
                }

                if(keyerTypeId == 0x0000)
                {
                    // For normal key, wait until user hold the key to determine the symbol.
                    lastMorseCode = CODE_EMPTY;
                }
                else 
                {
                    // Detect which paddle is keyed.
                    lastMorseCode = ((PORTB & 0x10) == 0x00) ? CODE_DASH : CODE_DOT;
                }

                releaseCounter = 0;
                flagChar = FALSE;
                flagWord = FALSE;
            }
        }
        
        // Restore timer 1 with 100Hz timing cycles.
        TMR1H = 177;
        TMR1L = 229;
        TMR1IF = 0;
    }
}

void isrUART()
{
    // UART data receive interrupt.
    unsigned char tempData;
    
    if(RCIF)
    {
        if(operatingMode == 0x0000)
        {
            tempData = readChar();
            // Limit characters to A..Z, a..z, 0..9 and SPACE.
            if(((tempData > 47) && (tempData < 58)) || ((tempData > 64) && (tempData < 91)) || ((tempData > 96) && (tempData < 123)) || (tempData == 32))
            {
                pushToBuffer(&dataBuffer, tempData);
            }
        }
        else 
        {
            // In keying mode ignore data received from the UART.
            RCREG;
        }
    }
}

void __interrupt() systemISR()
{
    // Timer 0 ISR, reserved for low priority tasks such as monitoring UI ports.
    isrTimer0();
    
    // Timer 1 SIR, reserved for morse keying detection.
    isrTimer1();
    
    // UART ISR, used to capture data received from USB endpoint.
    isrUART();
}

unsigned char systemSubMenuHandler(unsigned char selection, char *menuName, char **menuItems, unsigned char menuItemCount)
{
    signed char lastEncoderPosition = ROTARY_ENCODER_END;
    
    // Display heading of the sub menu.
    clearLCD();
    setCursor(1, 1);
    printStr(menuName);
    
    // Restore last user selection in menu system.
    encoderPosition = selection;
    lastInputStatus = PORTB & PORTB_MASK;
    
    while(1)
    {
        currentInputStatus = PORTB & PORTB_MASK;
        
        if(lastEncoderPosition != encoderPosition)
        {
            clearRow(2);
            lastEncoderPosition = encoderPosition;
            
            // Handle end of rotary encoder limit.
            if(encoderPosition == ROTARY_ENCODER_END)
            {
                encoderPosition = menuItemCount - 1;
            }
            
            // Handle last element of the selection list.
            if(encoderPosition == menuItemCount)
            {
                encoderPosition = 0;
            }
            
            printStr(menuItems[encoderPosition]);
        }
        
        // Check for user confirmation action.
        if(IS_BUTTON_PRESS(BTN_ROTARY_ENCODER))
        {
            return encoderPosition;
        }
        
        lastInputStatus = currentInputStatus;
    }
}

void updateSystemSettings()
{
    // Update global variables based on settings value.
    operatingMode = systemConfig & 0x0003;
    keyerTypeId = systemConfig & 0x000C;
    keySpeed = (systemConfig >> OPT_SPEED) & 0x03;
    keyerPortMask = ((keyerTypeId == 0x0000) ? 0x08: 0x18);
    toneType = (systemConfig >> OPT_TONE_TYPE) & 0x03;
    loopMessage = (systemConfig >> OPT_LOOP_SEND) & 0x03;
    
    // Update audio amplifier mute state.    
    shadowPortC &= 0xEF;
    
    if((systemConfig & 0x00C0) != 0x0000)
    {
        shadowPortC |= 0x10;
    }
    
    PORTC = shadowPortC;
}

void memoryKeyHandler()
{
    signed char lastEncoderPosition = ROTARY_ENCODER_END;
    unsigned char memAddr = MEM_MSG_BASE;
    unsigned char memData = 0;
    unsigned char memPos = 0;
    unsigned char buttonHoldCounter = 0;
    unsigned char currentChar;
    unsigned char charCount = MEM_MSG_SIZE;
    unsigned char waitTimeCount = 0;
    unsigned char userCancel = 0;

    // Initiate a memory manager.
    clearLCD();
    setCursor(1, 1);
    printStr("Memory slot");
    
    encoderPosition = 0;
    lastInputStatus = PORTB & PORTB_MASK;
    
    while(1)
    {
        currentInputStatus = PORTB & PORTB_MASK;
        
        // Check for PTT override key press.
        if(IS_BUTTON_PRESS(BTN_PTT_OVERRIDE))
        {
            pttOverride = ~pttOverride;
            shadowPortC &= 0xF7;
            
            if(pttOverride == TRUE)
            {
                shadowPortC |= 0x08; 
            }
            
            PORTC = shadowPortC;
            __delay_ms(50);
        }
        
        // Rotary encoder button is pressed to play the selected slot.
        if(IS_BUTTON_PRESS(BTN_ROTARY_ENCODER))
        {
            __delay_ms(50);
            
            // Play routine is built into this function to save stack levels.
            
            // Get a first byte from the selected memory page.
            memAddr = MEM_MSG_BASE + (encoderPosition * (MEM_MSG_SIZE + 1));
            currentChar = eeprom_read(memAddr);
            
            // Check for empty slot.
            if(currentChar != END_OF_MESSAGE)
            {
                clearRow(2);
                clearScrollBuffer();
                
                charCount = 0;
                lastInputStatus = PORTB & PORTB_MASK;
                userCancel = 0;
                
                // Message looping cycle. This loop iterates only if LOOPING 
                // is enabled in system settings. 
                while(userCancel == 0)
                {
                    while(charCount < MEM_MSG_SIZE + 1)
                    {
                        currentInputStatus = PORTB & PORTB_MASK;

                        // Wait for stop action (cancel) from user.
                        if((currentInputStatus & BTN_MEM_MANAGER) == 0x00)
                        {
                            clearRow(2);
                            printStr("CANCEL");

                            while((PORTB & BTN_MEM_MANAGER) == 0x00)
                            {
                                __delay_ms(10);
                            }

                            currentInputStatus = PORTB & PORTB_MASK;
                            lastInputStatus = currentInputStatus;
                            userCancel = TRUE;
                            
                            __delay_ms(150);
                            break;
                        }

                        // Check for end of message mark to stop the playback.
                        if(currentChar == END_OF_MESSAGE)
                        {
                           break; 
                        }

                        // Print current character and release morse code.
                        printScroll(currentChar);
                        encodeCharacter(currentChar);

                        // Reading next character from the memory slot.
                        currentChar = eeprom_read(++memAddr);

                        lastInputStatus = currentInputStatus;
                    }
                    
                    // End of message transmission and now check looping flag.
                    if(loopMessage == 0x00)
                    {
                        // Read first character of the message from the memory slot.
                        memAddr = MEM_MSG_BASE + (encoderPosition * (MEM_MSG_SIZE + 1));
                        currentChar = eeprom_read(memAddr);
                        
                        clearRow(2);
                        printStr("LOOPING  ");
                
                        charCount = 0;
                        waitTimeCount = 0;
                        
                        // Break looping if user issues cancel. 
                        if(userCancel == TRUE)
                        {
                            break;
                        }
          
                        // Lets keep 21 units of spacing between each message. 
                        while(waitTimeCount < 21)
                        {
                            waitTimeCount++;
                            currentInputStatus = PORTB & PORTB_MASK;
                            
                            // Update looping progress indicator.
                            if((waitTimeCount % 3) == 0)
                            {
                                printChar(0xFF);
                            }
                            
                            unitDelay();
                            
                            // Check for cancel action from user.
                            if((currentInputStatus & BTN_MEM_MANAGER) == 0x00)
                            {
                                clearRow(2);
                                printStr("CANCEL");

                                while((PORTB & BTN_MEM_MANAGER) == 0x00)
                                {
                                    __delay_ms(10);
                                }

                                currentInputStatus = PORTB & PORTB_MASK;
                                lastInputStatus = currentInputStatus;
                                userCancel = TRUE;
                                
                                __delay_ms(150);
                                break;
                            }
                            
                        }
                        
                        clearRow(2);
                        clearScrollBuffer();
                        lastInputStatus = PORTB & PORTB_MASK;
                    }
                    else 
                    {
                        // Looping is not active.
                        break;
                    }
                    
                    // Handle cancel state issued by the user.
                    if(userCancel == TRUE)
                    {
                        __delay_ms(10);
                        break;
                    }
                }
                
                clearLCD();
                setCursor(1, 1);
                printStr("Memory slot");
                lastEncoderPosition = ROTARY_ENCODER_END;
            }
        }
        
        // Toggle MEM button to close the memory manager.
        if(IS_BUTTON_PRESS(BTN_MEM_MANAGER))
        {
            __delay_ms(50);
            break;
        }
        
        // Watch button hold down counter to initiate the recording session.
        if((currentInputStatus & BTN_MEM_MANAGER) == 0x00)
        {
            buttonHoldCounter++;
            
            if(buttonHoldCounter >= 60)
            {
                buttonHoldCounter = 0;
                memAddr = 0;
                charCount = MEM_MSG_SIZE;
                
                // Start recording session. In here recording routine is in the 
                // same function to save the stack.
                clearLCD();
                setCursor(1, 1);
                printStr("Recording...");

                clearScrollBuffer();
                
                while((PORTB & BTN_MEM_MANAGER) == 0x00)
                {
                    __delay_ms(10);
                }

                __delay_ms(20);
                lastInputStatus = PORTB & PORTB_MASK;
                
                while(1)
                {
                    currentInputStatus = PORTB & PORTB_MASK;

                    // Wait for stop action (cancel) from user.
                    if(IS_BUTTON_PRESS(BTN_MEM_MANAGER))
                    {
                        __delay_ms(150);
                        break;
                    }

                    // Rotary encoder button event to save captured message to the memory.
                    if(IS_BUTTON_PRESS(BTN_ROTARY_ENCODER))
                    {
                        eepromBuffer[memAddr] = END_OF_MESSAGE;
                        saveMsgBuffer(eepromBuffer, encoderPosition);

                        __delay_ms(10);
                        break;
                    }

                    // Last 10 blocks of the memory space is reached and inform this to user.
                    if(charCount < 10)
                    {
                        setCursor(1, MAX_DISPLAY_LENGTH);
                        printChar(charCount + 48);
                    }

                    // Check end of memory space.
                    if(charCount > 0)
                    {
                        // Record user inputs into temporary memory buffer.
                        if(operatingMode == 0x0000)
                        {
                            // System is in USB mode.
                            if(popFromBuffer(&dataBuffer, &currentChar) == 0)
                            {
                                printScroll(currentChar);
                                encodeCharacter(currentChar);
                                charCount--;

                                eepromBuffer[memAddr] = currentChar;
                                memAddr++;
                            }
                        }
                        else 
                        {
                            // System is in KEY mode.
                            generateMorseOutput();

                            if(popFromBuffer(&dataBuffer, &currentChar) == 0)
                            {
                                printScroll(currentChar);
                                charCount--;

                                eepromBuffer[memAddr] = currentChar;
                                memAddr++;
                            }
                        }
                    }

                    lastInputStatus = currentInputStatus;
                }
                
                // End of recording session.
                clearLCD();
                setCursor(1, 1);
                printStr("Memory slot");
                lastEncoderPosition = ROTARY_ENCODER_END;
            }
        }
        
        // Change slot and display it's content if available.
        if(lastEncoderPosition != encoderPosition)
        {
            if(encoderPosition == 6)
            {
                encoderPosition = 0;
            }
            else if(encoderPosition == ROTARY_ENCODER_END)
            {
                encoderPosition = 5;
            }
            
            lastEncoderPosition = encoderPosition;
            setCursor(1, 13);
            printChar(encoderPosition + 49);
            
            // Try to preview content of the slot.
            memAddr = MEM_MSG_BASE + (encoderPosition * (MEM_MSG_SIZE + 1));
            memData = eeprom_read(memAddr);
            clearRow(2);            
            
            if(memData == END_OF_MESSAGE)
            {
                printStr("Empty slot");
            }
            else 
            {
                // The selected slot has content. Preview first few characters in display.
                memPos = 1;
                printChar(memData);
                
                while(memPos < (MAX_DISPLAY_LENGTH -1))
                {
                    memData = eeprom_read(++memAddr);
                    if(memData == END_OF_MESSAGE)
                    {
                        break;
                    }
                    printChar(memData);
                    memPos++;
                }
                
                // Message length is higher than the display buffer.
                if(memPos >= MAX_DISPLAY_LENGTH - 1) 
                {
                    printChar(0x7E);
                }
            }
        }
        
        __delay_ms(20);
        lastInputStatus = currentInputStatus;
    }
}

void systemMenuHandler() 
{
    signed char lastEncoderPosition = ROTARY_ENCODER_END;
    char *subMenuItemList[3];
    char *subMenuTitle = 0;
    unsigned char subMenuItemCount = 0;
    unsigned char optPosition = 0;
    unsigned short optTemp;
    unsigned char tempEncoderPos = 0;

    // Sub menu items.
    char *menuInputMode = "Input mode";
    char *menuKeyerType = "Keyer type";
    char *menuMorseSpeed = "Morse speed";
    char *menuSpeakerOut = "Speaker out";
    char *menuRepeatPlay = "Send in loop";
    char *menuKeyingType = "Keying type";
    
    clearLCD();
    setCursor(1, 1);
    printStr("System settings");
    
    lastInputStatus = PORTB & PORTB_MASK;
    encoderPosition = 0;
    
    while(1)
    {
        currentInputStatus = PORTB & PORTB_MASK;
        
        // Rotary encoder button pressed. Open the system menu.
        if(IS_BUTTON_PRESS(BTN_ROTARY_ENCODER))
        {
            __delay_ms(50);
            
            switch(encoderPosition)
            {
                case 0: 
                    // Input mode sub menu.
                    subMenuItemList[0] = "Host terminal";
                    subMenuItemList[1] = "Keyer";
                    subMenuItemCount = 2;
                    subMenuTitle = menuInputMode;
                    optPosition = OPT_INPUT_MODE;
                    break;
                case 1:
                    // Keyer type sub menu.
                    subMenuItemList[0] = "Morse key";
                    subMenuItemList[1] = "Morse paddle";
                    subMenuItemCount = 2;
                    subMenuTitle = menuKeyerType;
                    optPosition = OPT_KEYER_TYPE;
                    break;
                case 2:
                    // WPM selection sub menu.
                    subMenuItemList[0] = "5 WPM";
                    subMenuItemList[1] = "10 WPM";
                    subMenuItemList[2] = "15 WPM";
                    subMenuItemCount = 3;
                    subMenuTitle = menuMorseSpeed;
                    optPosition = OPT_SPEED;
                    break;
                case 3:
                    // Speaker status sub menu.
                    subMenuItemList[0] = "Active";
                    subMenuItemList[1] = "Mute";
                    subMenuItemCount = 2;
                    subMenuTitle = menuSpeakerOut;
                    optPosition = OPT_SPEAKER_OUT;
                    break;
                case 4:
                    // Send recorded message in loop.
                    subMenuItemList[0] = "On";
                    subMenuItemList[1] = "Off";
                    subMenuItemCount = 2;
                    subMenuTitle = menuRepeatPlay;
                    optPosition = OPT_LOOP_SEND;
                    break;
                case 5:
                    // Keying type sub menu.
                    subMenuItemList[0] = "PTT";
                    subMenuItemList[1] = "Tone";
                    subMenuItemList[2] = "PTT + Tone";
                    subMenuItemCount = 3;
                    subMenuTitle = menuKeyingType;
                    optPosition = OPT_TONE_TYPE;
                    break;
                case 6:
                    // Exit from menu system.
                    lastInputStatus = MAX_BYTE;
                    return;
            }
            
            // Open selected sub menu item.
            __delay_ms(50);
            tempEncoderPos = encoderPosition;
            optTemp = systemSubMenuHandler((systemConfig >> optPosition) & 0x03, subMenuTitle, subMenuItemList, subMenuItemCount);
            
            // Update system configuration variable with user selected options.
            systemConfig &= ~(0x03 << optPosition);
            optTemp &= 0x0003;
            systemConfig |= optTemp << optPosition;
            
            updateSystemSettings();
            
            // Return to main menu from sub menu item.
            clearLCD();
            setCursor(1, 1);
            lastEncoderPosition = ROTARY_ENCODER_END;
            encoderPosition = tempEncoderPos;
            printStr("System settings");
        }
        
        // Update LCD if rotary encoder position is changed.
        if(lastEncoderPosition != encoderPosition)
        {
            clearRow(2);
            lastEncoderPosition = encoderPosition;
            
            switch(encoderPosition)
            {
                case 0:
                    printStr(menuInputMode);
                    break;
                case 1:
                    printStr(menuKeyerType);
                    break;
                case 2:
                    printStr(menuMorseSpeed);
                    break;
                case 3:
                    printStr(menuSpeakerOut);
                    break;
                case 4:
                    printStr(menuRepeatPlay);
                    break;
                case 5:
                    printStr(menuKeyingType);
                    break;
                case 6:
                    printStr("Exit");
                    break; 
                case ROTARY_ENCODER_END:
                    encoderPosition = 6;
                    break;
                default:
                    encoderPosition = 0;
            }
        }
        
        lastInputStatus = currentInputStatus;
    }
}

void enableInterrupts()
{
    PIE1 = 0x21;
    PIR1 = 0x00;

    INTCON = 0xE4;
}

void initSystem()
{
    // Set MCU internal oscillator to 8MHz. 
    OSCCON = 0x70;
    INTCON = 0x00;
    
    // Setting up timer0 to 250Hz.
    OPTION_REG = 0x04;
    TMR0 = 6;
    
    WPUB = 0x7F;
    SSPCON = 0x00;
    
    // Setting up GPIO ports.
    TRISA = 0x00;
    TRISB = 0x7F;
    TRISC = 0x00;
    
    PORTA = 0x00;
    PORTB = 0x7F;
    PORTC = 0x00;
    
    // Disable ADC and Comparator components of the MCU.
    ANSEL = 0x00;
    ANSELH = 0x00;
    ADCON0 = 0x00;
    CM1CON0 = 0x00;
    CM2CON0 = 0x00;
    
    T1CON = 0x0D;
    TMR1H = 177;
    TMR1L = 229;
    
    // Initialize peripherals which is used by the firmware.
    initUART();
    initPWM();
    
    initLCD();
}
