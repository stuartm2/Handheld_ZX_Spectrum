# Handheld ZX Spectrum

**Note: this project is in progress**

This repository contains code and config to build a handheld ZX Spectrum (emulated) on a Raspberry Pi Zero.

## Keyboard

The keyboard matches the original ZX keyboard layout and is built with an Arduino Pro Micro.  It also has 7 game control buttons - a 5-way joypad and two fire buttons.  These can be programmed over a 9600bps serial link by sending 7 characters to the keyboard in this sequence:

 1. Joypad button
 2. Main fire button
 3. Secondary fire button
 4. Up
 5. Right
 6. Down
 7. Left
