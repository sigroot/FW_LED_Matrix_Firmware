# Framework LED Matrix Firmware
![](readme/matrix.fig)
## Preamble
This repository contains custom firmware for the [Framework LED Matrix](https://frame.work/products/16-led-matrix).

The [default firmware](https://github.com/FrameworkComputer/inputmodule-rs/tree/main) for the Framework LED Matrix can not produce all 256 brightness levels possible on the LED Matrix hardware. The provided interface software is also difficult to develop with. This custom firmware solves both issues. 
## Capabilities
Framework LED Matrix physical documentation is located [here](https://github.com/FrameworkComputer/InputModules/blob/main/Electrical/LEDMatrix/README.md).
IS31FL3741A 9x34 LED board documentation is located [here](https://lumissil.com/assets/pdf/core/IS31FL3741A_DS.pdf).

This firmware allows programs to write both PWM and scale values (as defined by the IS31FL3741A documentation) for either single LED's, all LED's, or as a 9x34 pixel image to the Framework LED Matrix. It also contains several demo animations such as a custom boot-up animation. The LED matrix can be fully refreshed at approximately 80 frames per second.
## Usage
### Installation
This firmware is programmed in the Arduino language and can be installed to the LED matrix from the uf2 file or by using the Arduino IDE.

To install this firmware from the uf2 file:
1. Remove the LED Matrix input module from the laptop.
2. Set the LED Matrix to bootloader mode by gently setting swtich '2' to ON (switch to the right).
3. Place the LED Matrix input module back into the laptop. The LED Matrix should appear as a folder in the file manager labled "RPI-RP2" or similar.
4. Open the LED Matrix folder.
5. Place [rp2040_firmware.ino.uf2](rp2040_firmware/build/rp2040.rp2040.generic/rp2040_firwmare.ino.uf2) from this repository into the LED Matrix folder.
6. Eject the LED Matrix
7. Remove the LED Matrix input module from the laptop.
8. Reset the LED Matrix from bootloader mode by gently setting switch '2' to OFF (switch to the left)
9. Place the LED Matrix input module back into the laptop. The LED Matrix should display a startup animation.

To install this firmware using the Arduino IDE, follow Joe Schroedl's [instructions](https://jschroedl.com/rgb-start/) for reinstalling firmware for his own RGB LED Matrix. **However, instead of following steps 8-10 install the Arduino code in this repository and open it in the Arduino IDE. The Adafruit NeoPixel Library is not necessary.**
### Associated Software
[FW_LED_Matrix_Interface](https://github.com/sigroot/FW_LED_Matrix_Interface) is a Rust library for interfacing between this firmware and other Rust programs.

[FW_LED_Matrix_Board](https://github.com/sigroot/FW_LED_Matrix_Board) divides the 9x34 LED Matrix into 3 smaller 9x10 "applets" and provides a language agnostic interface between these applets and other programs.

[FW_LED_Matrix_Applet](https://github.com/sigroot/FW_LED_Matrix_Applet) is a Rust Library for interfacing between Rust programs and [FW_LED_Matrix_Board](https://github.com/sigroot/FW_LED_Matrix_Board).
### Communication
Communication is over a serial port. Commands are entered from the computer as 8-bit characters. Each command may require additional 8-bit parameters and may return a response. For example, sending an 'M' over the serial port will write new PWM values to the LED Matrix. The next 306 8-bit values over the serial port will be accepted as the PWM values in reading-order. The LED Matrix will then return an 'M' to indicate a successful write. The command list is specified below:

Opcode | Description | Parameters | Return Values
--- | --- | --- | ---
000 | Do nothing | No parameters | No return values
'a' | Play startup animation until interrupted | No parameters | No return values
'A' | Play startup animation once, then blank | No parameters | No return values
'b' | Turn the current image into a fire animation | No parameters | No return values
'c' | Clear the queue between the LED Matrix command reader and command processor | No parameters | No return values
'd' | Display a diamond animation | 1 8-bit framerate value | No return values
'e' | Send rp2040 to bootloader | No parameters | No return values
'f' | Display fireplace animation until a new command is received | No parameters | No return values
'g' | Display a spinning gear until a new command is received | 1 8-bit framerate value | no return values
'm' | Write a new image to the matrix LEDs' PWM | 306 8-bit PWM values | no return values
'M' | Write a new image to the matrix LEDs' PWM, then send a response for software blocking | 306 8-bit PWM values | a single 'M'
'n' | Write a new image to the matrix LEDs' scale | 306 8-bit scale values | no return values
'N' | Write a new image to the matrix LEDs' scale, then send a response for software blocking | 306 8-bit scale values | a single 'N'
'p' | Set a matrix LED's PWM | 1 8-bit x-value (0-8), 1 8-bit y-value (0-33), 1 8-bit PWM value | no return value
'q' | Set a matrix LED's scale | 1 8-bit x-value (0-8), 1 8-bit y-value (0-33), 1 8-bit scale value | no return value
'r' | Display a spinning ring animation until a new command is received | 1 8-bit framerate value | no return value
's' | Set the scale for every LED | 1 8-bit scale value | no return value
't' | Run a test pattern on every LED and for every PWM | no parameters | no return values
'w' | Set the PWM for every LED | 1 8-bit PWM value | no return value
127 | Return a known string to confirm correct firmware | no parameters | returns e.g. "Sig FW LED Matrix FW V1.1"

## This project is licensed under the MIT license

Copyright © 2025 Sigroot

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
