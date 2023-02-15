# Digital Systems Practical - White Light Communication

_Credits to Mike Spivey for teaching the course and providing the template materials._

## Overview
This project involves transmitting binary data from a transmission computer to the BBC Micro:bit v1 through white light. The transmission computer creates a window that flashes between black and white, encoding the binary message. The microbit uses its analogue to digital converter to sample the voltages on columns 1 to 3 of its LED array. This allows for a light sensing function. The microbit records the light levels for some time and then decodes the information.

## Protocol
We use a simple asynchronous transmission protocol. White light represents a high value (the default), while a black screen represents a low value. We merely convert each character into its binary form (of its ASCII representation). Each character is prepended by a start-bit (0) and appended by 3 stop-bits (1). 

On the receiving end, the microbit uses the first 10 samples to determine the high value and creates a threshold accordingly. This allows the light levels to be converted into a digital bit stream.

The microbit will then search for a start bit by searching for a falling edge to indicate a start bit. We determine if it is indeed a start bit and then sample the 3 bits around the center of each of the next 7 data bits to determine the encoded character.


## Further Work
We can probably improve this further by a better way to determine the threshold between high and low light values. We can also improve how we process the bit stream to determine the values of the transmitted characters. However, these are all not within the scope of the Digital Systems course.