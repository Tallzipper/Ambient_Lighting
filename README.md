# Ambient Lighting Sampling Engine

## Intro

  This project will work to set up 'Ambient Lighting' to your TV or Monitor which immerses you in your media. With the use of internal based capture-software it removes the need of an external camera-capture based hardware which is visually obstructive at best or inaccurate at worst. The program directly reads the pixels on the border of your screen and relays this information to the LEDs placed behind your monitor. 
  
  The demo/diagnostics file has been completed fully functional and the main program will be updated once the remaining hardware arrives. 

## Demonstration

Gif will be added along with a youtube video

## Hardware

- ESP32 module
- LED strips
- 5V 10A Power Supply AC/DC Adapter
- Video Capture Card, 4K USB3.0 HDMI to USB C, 1080P 60FPS
- HDMI Splitter 4K@60Hz, HDMI Splitter 1 in 2 out
- Two HDMI cables

## Roadmap   

- [x]   Diagnostics File: Shows what the LEDs would be picking up and tests your capture card, hdmi-splitter, and input device
- [ ]   Main program: Using LEDs and ESP32 as an addion
- [ ]   Improving Pixel Capture: Fixing the firework issue and brightening colors
- [ ]   Adaptably Zooming in: When pressing full screen on Diagnostics file, will fill screen 
- [ ]   Raspberry Pi compatability: A Raspberry Pi can run the program rather than be stuck to your computer
  
## Installation Guide

  Will be added when an executable file with a guide at a later date

## Known issues

- My HDMI splitter won't accept input from the Nintendo Switch 2, this is likely from the anti-piracy software forbidding it
- Zooming into the Ambilight Command Center window will not adjust the window to the monitor
- Fireworks and other changes to the screen that aren't big enough to resgister to the border aren't put on the LEDs
