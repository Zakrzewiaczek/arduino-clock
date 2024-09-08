## How to use program?

1. Upload program
2. [If you want the clock to change brightness automatically]
   
   (Every time you turn on the device)
     If it's day: open the blinds, curtains, etc. Make sure the room is not dark (as it should be so that the clock lights up normally)
     If it's night: turn on light.
   
   [If you want to set the clock to change brightness at fixed times]
      Add '//' at the beginning of line 8
      In line no. 17 (const uint8_t hours[2] = {X, Y}; //8) replace 'X' with the hour when the clock is to turn on the bright mode. In place of 'Y', insert the time when the clock should turn on dark mode.

3. When you want to enable debug mode, remove the '//' characters on line no. 7 (Serial 9600 bd)

  If the clock does not show the time, try reloading the program.

If you have error, make new Issue. I correct code ASAP.

Enjoy :)
