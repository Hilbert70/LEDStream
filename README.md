LEDStream
=========

This is an Adalight compatible LPD6803 and others LEDstream

The FastSPI_LED.cpp  FastSPI_LED.h was copied from

http://code.google.com/p/fastspi/

So those files are not my own!

I can confirm that at least the strand test of the fastspi is working with the WS2801 with the addition of

FastSPI_LED.setDataRate(3); 

But this is already in the LEDStream code ;-)

