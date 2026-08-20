# Ambient Lighting Sampling Engine

  This project will work to set up 'Ambient Lighting' to your TV or Monitor which immerses you in your media. With the use of internal based capture-software it removes the need of an external camera-capture based hardware which is visually obstructive at best or inaccurate at worst. The program directly reads the pixels on the border of your screen and relays this information to the LEDs placed behind your monitor. 
  
  The demo/diagnostics file has been completed fully functional and the main program will be updated once the remaining hardware arrives. 

  ---

## Demonstration

Gif will be added along with a youtube video

## Hardware

- ESP32 module
- LED strips
- 5V 10A Power Supply AC/DC Adapter
- Video Capture Card, 4K USB3.0 HDMI to USB C, 1080P 60FPS
- HDMI Splitter 4K@60Hz, HDMI Splitter 1 in 2 out
- Two HDMI cables
- Wires
- Electrical Tape
- 330 OHM Resistor

## Roadmap   

- [x]   Diagnostics File: Shows what the LEDs would be picking up and tests your capture card, hdmi-splitter, and input device
- [x]   Main program: Using LEDs and ESP32 as an addion
- [x]   Improving Pixel Capture: Fixing the firework issue and brightening colors
- [x]   Adaptably Zooming in: When pressing full screen on Diagnostics file, will fill screen
- [ ]   Not using a window and instead just focusing on the lights
- [ ]   Raspberry Pi compatability: A Raspberry Pi can run the program rather than be stuck to your computer
  
## Installation Guide

  Will be added when an executable file with a guide at a later date

## Known issues

- My HDMI splitter won't accept input from the Nintendo Switch 2, this is likely from the anti-piracy software forbidding it (NOTE: Using the firestick will however allow everything on there to still show)
- ~~Zooming into the Ambilight Command Center window will not adjust the window to the monitor~~
- ~~Fireworks and other changes to the screen that aren't big enough to resgister to the border aren't put on the LEDs~~
- Pressing only 'q' is unintuitive, I'll add escape to also kill the program, an 'L' button to toggle the pixels/LEDs on the borders and 'F' to stop zooming in.
- Only works for Windows

## Authors note in progress

- 8/9/2026: Over the last two weeks I have been going over this project as well as making the overall set-up safer. For one, a resistor is now used to not overload the LEDs. I'll also add a warning that this set-up currently only works for lights behind a monitor. You'll need shifters and a better AC/DC adaptor for this to work for anything much larger than that since it can be an electrical hazard. The system is currently only engineered to my screen. I'll be adding a .json file as well as a way of altering it in order to make it universal. The system also only works for windows api so I'll be changing that as well. The Raspberry pi implementation will take a backseat in favor of adding an installation guide which has been delayed due to it only working well on my device. This should be a lot smoother though since the project itself was the heavy lifting. 

- 8/19/2026: I tried making a server set-up to make the process smoother but I realized the issue was that since I am streaming and putting the hdmi up to my computer it won't work so what I'll have to do is start streaming onto a different screen to get it to be able to actually test this. The server will be recycled and be used to control the screen through a webpage so it can be toggled remotely without needing to touch the computer.
