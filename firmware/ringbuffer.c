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

#include "ringbuffer.h"

void initRingBuffer(ringBuffer *buffer)
{
    unsigned char bufferPos;
    
    for(bufferPos = 0; bufferPos < RING_BUFFER_SIZE; bufferPos++)
    {
        buffer->morseBuffer[bufferPos] = 0;
    }
    
    buffer->readPos = 0;
    buffer->writePos = 0;
}

unsigned char pushToBuffer(ringBuffer *buffer, unsigned char data)
{
    unsigned char newPos = buffer->writePos + 1;
    
    // Check for end of the ring buffer to setup continuation.
    if(newPos >= RING_BUFFER_SIZE)
    {
        newPos = 0;
    }
    
    if(newPos == buffer->readPos)
    {
        // Ring buffer is full.
        return 1;
    }
    
    // Push data into ring buffer and update new position.
    buffer->morseBuffer[buffer->writePos] = data;
    buffer->writePos = newPos;
    return 0;
}

unsigned char popFromBuffer(ringBuffer *buffer, unsigned char *data)
{
    unsigned char newPos = buffer->readPos + 1;
    
    if(newPos >= RING_BUFFER_SIZE)
    {
        newPos = 0;
    }
    
    if(buffer->readPos == buffer->writePos)
    {
        // Ring buffer is empty.
        return 1;
    }
    
    // Pop data from ring buffer and update new position.
    *data = buffer->morseBuffer[buffer->readPos];
    buffer->readPos = newPos;
    return 0;
}
