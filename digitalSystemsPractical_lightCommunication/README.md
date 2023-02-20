# Digital Systems Practical - White Light Communication

_Credits to Mike Spivey for teaching the course and providing the template materials._

## Overview
This project involves transmitting binary data between two BBC Micro:bit v1 boards through light. 

## Part 1: Monitor to Microbit
We start by trying to get a monitor to transmit information into the microbit. Using `whiteLightTransmitter.py`, the transmission computer creates a window that flashes between black and white, encoding the binary message. The microbit uses its analogue to digital converter to sample the voltages on columns 1 to 3 of its LED array. This allows for a light sensing function. The microbit records the light levels for some time and then decodes the information.

### Transmission
We use a simple asynchronous transmission protocol. White light represents a high value (the default), while a black screen represents a low value. We merely convert each character into its binary form (of its ASCII representation). Each character is prepended by a start-bit (0) and appended by 3 stop-bits (1). 
- Since we are only looking at printable characters, we only need the least significant 7 bits of the character. 
- We send the bits from most significant bit to least signficant bit.

### Receiving
The microbit runs a timer that triggers its analog to digital converter to read the voltage values from the LEDs (columns 1,2,3). These values will be stored in a circular buffer.

On start-up, the microbit first fills up half the buffer and then computes the threshold light level difference as the average of the minimum and maximum light values that it records. This threshold is used to convert the light values into the 1s and 0s with a bit stream.

It will then flush the buffer and repeatedly scan for any falling edges that possibly indicate a start bit. It will then wait for enough samples to check if it really is a start bit. We do this check by checking if there are sufficiently many samples within the bit that are low.

If a start bit is detected, we wait again for sufficiently many samples to be taken such that we record the entire character. Then we check the average of the 3 center sample bits for each data bit to determine the value of each data bit within the character.

Characters detected are decoded and printed out to the serial port.

## Part 2: Microbit to Microbit Cmmunication
We go further with `redLightTransmitter.c` which has a CLI over the serial port that allows users to type in messages. These messages are then encoded and sent as flashes on the LED panel. 

The receiving programme stays the same as the previous part.

A combination of 2 microbits running `redLightTransmitter.c` and `whiteLightReceiver.c` allow 2 computers to communicate "wirelessly".

## Further Work
We can probably improve this further by a better way to determine the threshold between high and low light values. We can also improve how we process the bit stream to determine the values of the transmitted characters. However, these are all not within the scope of the Digital Systems course.
