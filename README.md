# Handheld ZX Spectrum

This repository contains code and config to build a handheld ZX Spectrum (emulated) on a Raspberry Pi Zero.  The full project is documented on Instructables.com: https://www.instructables.com/id/Pocket-ZX-Handheld-ZX-Spectrum/

## Keyboard & Joystick

The keyboard matches the original ZX keyboard layout and is built with an Arduino Pro Micro.  It also has a joypad control with a 5-way switch and two action buttons.  These will function as either cursor key presses, a USB joystick or a USB mouse depending on which mode it's in.  The mode is toggled by pressing the main action button in combination with either the left (joystick) or right (mouse) shift key.

## Shutdown script

This script watches for a low signal on GPIO3 and initiates system shutdown when it sees one.
