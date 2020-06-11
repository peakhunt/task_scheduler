RISC OS Pico
------------

RISC OS Pico is a fairly minimal version of RISC OS for the Raspberry Pi:

  http://www.raspberrypi.org/

with some related goodies. We created this distro to celebrate the 50th
birthday of the original BASIC programming language:

  http://en.wikipedia.org/wiki/Dartmouth_BASIC

We also believe it may also be of general interest to Raspberry Pi hackers and
modders.

It is supplied as a ZIP file containing everything you need to boot a
Raspberry Pi directly into BBC BASIC to give you the full retro BASIC
experience. Just unpack the contents of the ZIP file into the root of a
FAT formatted SD card (most memory cards are FAT formatted when bought in
the shop).

We've also included some demo programs to illustrate how BBC BASIC programs
work and how you can use them to do things like twiddling the Pi's GPIO lines.
To run one, type for example:

  LOAD "EXAMPLES.SNOW"
  RUN

or just:

  CHAIN "EXAMPLES.SNOW"

to do both in one command. (The '.' character is the directory separator on
RISC OS.) Use:

  *CAT

or:

  *CAT EXAMPLES

to show what files are available. Other useful commands include:

  LIST  to show the current program
  SAVE  to save the current program
  EDIT  to edit the current program
  HELP  for lots more help!

About the examples
------------------

FruitMachine - a faithful emulation of a pub fruit machine. A description of
               how it works is given in the text file alongside the program,
               though sadly it doesn't dispense real money.
Lissajous - the staple swirly trace on an oscilloscope in the lab of
            any movie mad scientist. Look at the mathematics behind how
            they are created.
NanoArm - control a robot arm from the comfort of your keyboard. Requires
          a USB connected arm from Maplin Electronics (order code A37JN).
PiGlow - an example of how to use the IIC bus on the Pi to control a matrix
         of LED lights. Requires external hardware plugged into the expansion
         connector from Maplin Electronics (order code A77LU).
PiLED - an example of how to use the GPIO functions built into Pico to control
        input/output lines on the Pi. Starting from this example, which uses
        the "OK" LED on some models of Pi, you can see how more complicated
        projects might use the expansion port.
SimonSays - play classic 70's game "Simon" by remembering sequences of
            lights and repeating them back to Simon (the Pi) using buttons.
            Requires external hardware.
Snow - relax while the screen is filled with a flurry of tumbling snow. But
       how is the effect achieved using just BBC BASIC? Can you make other
       meteorological phenomonen?

Extras
------

As if that wasn't enough, there's also a free PDF copy of Martyn Fox's
excellent book, "First Steps in Programming RISC OS Computers", along with all
of his associated demo programs.

If you want to show your love for BASIC and RISC OS, please consider making a
donation towards RISC OS Open's work:

  http://www.riscosopen.org/content/donations

Help and advice can be found on our forums:

  http://www.riscosopen.org/forum/

Thanks!

-- The ROOL Team, 2017
