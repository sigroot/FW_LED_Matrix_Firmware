/*
  Written by sigroot (github.com/sigroot)
  
  rp2040_firmware.ino - Firmware code for the Framework LEDMatrix input module.

*/

#if !(SIG_FIRMWARE)
#include "rp2040_firmware.h"
#endif
#if !(SIG_PATTERNS)
#include "rp2040_patterns.h"
#endif


//    *** Constants ***

// The version information statement
const String versionStatement = "Sig FW LED Matrix FW V1.1";

// The baud rate of the rp2040 usb serial.
const int serialBaud = 1000000;
const int bootTime = 10000;


//    *** Global variables ***
int autoBootTimer;
bool newCommand;


//    *** Functions ***

// Displays a neat animation inteded for startup.
// Can be interrupted if newCommand is set.
void startupAnimation() {

  while (!newCommand) {
    
    for (int f = 0; f < 80; f++) {
      if (newCommand) break;

      // Draw background of animation.
      for (int i = 0; i < LEDHeight; i++) {
        if (newCommand) break;
        for (int j = 0; j < LEDWidth; j++) {
          if (newCommand) break;
          // writes a neat moving pattern.
          inputMatrix[i][j] = 60*cos(2*PI*(0.5*double(i*17%LEDHeight)/LEDHeight + 0.5*double(j*13%LEDWidth)/LEDWidth - double(f)/80))
                              + 80*sin(2*PI*(0.5*double(i)/LEDHeight + 0.5*double(j)/LEDWidth - double(f)/80)) + 60;
        }
      }
      
      // Add Framework gear.
      for (int i = 0; i < LEDHeight; i++) {
        if (newCommand) break;
        for (int j = 0; j < LEDWidth; j++) {
          if (newCommand) break;
          inputMatrix[i][j] = (uint16_t(inputMatrix[i][j]) + uint16_t(FWGear[f/20][i][j]))/2;
        }
      }

      // Display matrix.
      writeMatrix(inputMatrix, true);

      // Remove Framework gear.
      for (int i = 0; i < LEDHeight; i++) {
        if (newCommand) break;
        for (int j = 0; j < LEDWidth; j++) {
          if (newCommand) break;
          inputMatrix[i][j] = (2*uint16_t(inputMatrix[i][j]) - uint16_t(FWGear[f/20][i][j]));
        }
      }
    }
  }
}

// Displays a neat animation inteded for startup for a short time.
// Can not be interrupted if newCommand is set.
void singleStartupAnimation() {
  for (int k = 0; k < 4; k++){
    for (int f = 0; f < 80; f++) {

      // Draw background of animation.
      for (int i = 0; i < LEDHeight; i++) {
        if (newCommand) break;
        for (int j = 0; j < LEDWidth; j++) {
          if (newCommand) break;
          // writes a neat moving pattern.
          inputMatrix[i][j] = 60*cos(2*PI*(0.5*double(i*17%LEDHeight)/LEDHeight + 0.5*double(j*13%LEDWidth)/LEDWidth - double(f)/80))
                              + 80*sin(2*PI*(0.5*double(i)/LEDHeight + 0.5*double(j)/LEDWidth - double(f)/80)) + 60;
        }
      }
      
      // Add Framework gear.
      for (int i = 0; i < LEDHeight; i++) {
        if (newCommand) break;
        for (int j = 0; j < LEDWidth; j++) {
          if (newCommand) break;
          inputMatrix[i][j] = (uint16_t(inputMatrix[i][j]) + uint16_t(FWGear[f/20][i][j]))/2;
        }
      }

      // Display matrix.
      writeMatrix(inputMatrix, true);

      // Remove Framework gear.
      for (int i = 0; i < LEDHeight; i++) {
        if (newCommand) break;
        for (int j = 0; j < LEDWidth; j++) {
          if (newCommand) break;
          inputMatrix[i][j] = (2*uint16_t(inputMatrix[i][j]) - uint16_t(FWGear[f/20][i][j]));
        }
      }
    }
  }
}

// Turns previous matrix into fire.
// Can be interrupted if newCommand is set.
void burnPattern() {
  for(int i = 0; i < 100; i++) {
    if (newCommand == true) break;
    burn();
  }
}

// Closes the connections and returns to the bootloader
void endFirmware() {
  Serial.println("RETURNING TO BOOTLOADER");
  Wire1.end();
  Serial.end();
  bootloader();
}

//Displays a fire pattern until a new command is sent.
void fireplacePattern() {
  while (!newCommand) {
    fireplace();
  }
}

// Displays a rotating ring.
// Can be interrupted if newCommand is set.
void ringPattern(uint8_t fps) {
  int frameDelay = 1000/fps;

  for(int i = 0; !newCommand; i += 5) {
    if (newCommand == true) break;

    int timeStart = millis();

    spinningRing(i);

    // Spin until frame rate is reached
    while (millis() - timeStart < frameDelay) {
      delay(1);
    }
  }
}

// Displays each frame of the spinning framework gear in order.
// Can be interrupted if newCommand is set.
void rotateGear(uint8_t fps) {
  int frameDelay = 1000/fps;
  while (!newCommand) {
    for (int j = 0; j < 4; j++) {
      if (newCommand) break;

      int startTime = millis();

      writeMatrix(FWGear[j]);

      // Spin until frame rate is reached
      while (startTime - millis() < frameDelay) {
        delay(1);
      }
    }
  }
}

// Displays a neat diamond pattern.
// Can be interrupted if newCommand is set.
void diamondPattern(uint8_t fps) {
  int frameDelay = 1000/fps;
  for(int i = 0; i <= 255; i += 1) {
    if (newCommand == true) break;

    int startTime = millis();

    diamonds(i);

    // Spin until frame rate is reached
    while (millis() - startTime < frameDelay) {
      delay(1);
    }
  }
}

// Writes PWM code to the matrix.
// Core 0 wrote the matrix for communication.
// Can be interrupted if newCommand is set.
void serialWriteMatrix(bool useGamma = false) {
  if (!newCommand) {
    writeMatrix(inputMatrix, useGamma);
  }
}

// Writes PWM code to the matrix, then sends a serial response for blocking.
// Core 0 wrote the matrix for communication.
// Can be interrupted if newCommand is set.
void serialWriteMatrixBlocking(bool useGamma = false) {
  if (!newCommand) {
    writeMatrix(inputMatrix, useGamma);
  }
  Serial.write('M');
}

// Writes scale code to the matrix.
// Core 0 wrote the matrix for communication.
// Can be interrupted if newCommand is set.
void serialWriteMatrixScale(bool useGamma = false) {
  if (!newCommand) {
    writeMatrixScale(inputMatrix, useGamma);
  }
}

// Writes scale code to the matrix, then sends a serial response for blocking.
// Core 0 wrote the matrix for communication.
// Can be interrupted if newCommand is set.
void serialWriteMatrixScaleBlocking(bool useGamma = false) {
  if (!newCommand) {
    writeMatrixScale(inputMatrix, useGamma);
  }
  Serial.write('N');
}

// Writes every brightness to every pixel.
// Can be interrupted if newCommand is set.
void testAllPixel() {
  for (int i = 0; i < LEDHeight; i++) {
    if (newCommand) break;
    for (int j = 0; j < LEDWidth; j++) {
      if (newCommand) break;

      // Debug information.
      Serial.print(j); Serial.print(" "); Serial.println(i);

      // Display all brightnesses.
      for (int k = 0; k < 256; k += 25) {
        inputMatrix[i][j] = k;
        writeMatrix(inputMatrix);
        delay(1);
      }

      // Set LED to off.
      inputMatrix[i][j] = 0;
    }
  }
}

// setup and loop run on core 0 of the rp2040.
// This core reads commands and sends them to core 1.
void setup() {
  // Start serial communication through the usb_d port.
  Serial.begin(serialBaud);

  // Set parameters for I2C communication between the rp2040 and the LED Matrix
  // Controller.
  wireInit();

  // Begin I2C on the appropriate pins.
  Wire1.begin();

  // Set the I2C clock to an appropriate rate (default 400000 hz)
  Wire1.setClock(i2cClockRate);

  // Reset all of the LED Matrix controller registers
  matrixReset();

  // Push startup animation
  rp2040.fifo.push('s');
  rp2040.fifo.push(127);
  newCommand = true;
  rp2040.fifo.push('A');
  newCommand = true;
}

void loop() {
  // If core 1 is still processing the last command, restart loop.
  if (newCommand == true) return;

  // Get current byte from serial.
  int readByte = Serial.read();

  // If the data is not a real number, restart loop.
  if (readByte == -1) return;
  
  // Convert the read byte to unsigned byte format.
  uint8_t codeByte = readByte;

  // Send the byte to core 1.
  rp2040.fifo.push(codeByte);

  // Some commands need extra variables
  int availableBytes;
  // For certain commands, accept extra input.
  int startTime = millis();
  switch (codeByte) {
    // Accept range and fps for diamond pattern.
    case 'd':
      for(int i = 0; i < 3; i++) {
        rp2040.fifo.push(uint8_t(Serial.read()));
      }
      break;
    // Accept fps for gear pattern.
    case 'g':
      rp2040.fifo.push(uint8_t(Serial.read()));
      break;
    // Accept full matrix of pwm for matrix set pwm.
    case 'm':
      for (int i = 0; i < LEDHeight; i++) {
        for (int j = 0; j < LEDWidth; j++) {
          inputMatrix[i][j] = Serial.read();
        }
      }
      break;
    // Accept full matrix of pwm for matrix set pwm blocking.
    case 'M':
      for (int i = 0; i < LEDHeight; i++) {
        for (int j = 0; j < LEDWidth; j++) {
          inputMatrix[i][j] = Serial.read();
        }
      }
      break;
    // Accept full matrix of scale for matrix set scale.
    case 'n':
      for (int i = 0; i < LEDHeight; i++) {
        for (int j = 0; j < LEDWidth; j++) {
          inputMatrix[i][j] = Serial.read();
        }
      }
      break;
    // Accept full matrix of scale for matrix set pwm blocking.
    case 'N':
      for (int i = 0; i < LEDHeight; i++) {
        for (int j = 0; j < LEDWidth; j++) {
          inputMatrix[i][j] = Serial.read();
        }
      }
      break;
    // Accept x, y, and pwm for setPixel.
    case 'p':
      for(int i = 0; i < 3; i++) {
        rp2040.fifo.push(uint8_t(Serial.read()));
      }
      break;
    // Accept x, y, and scale for setPixelScale.
    case 'q':
      for(int i = 0; i < 3; i++) {
        rp2040.fifo.push(uint8_t(Serial.read()));
      }
      break;
    // Accept fps for ringPattern.
    case 'r':
      rp2040.fifo.push(uint8_t(Serial.read()));
      break;
    // Accept byte for setting the matrix's scale.
    case 's':
      rp2040.fifo.push(uint8_t(Serial.read()));
      break;
    // Accept byte for setting the matrix's PWM.
    case 'w':
      rp2040.fifo.push(uint8_t(Serial.read()));
      break;
  }
  
  // Inform core 1 that a new command was pushed.
  newCommand = true;
}


// setup1 and loop1 run on core 1 of the rp2040.
// This core executes commands in an interruptable manner.
void setup1() {
  newCommand = false;
}

void loop1() {
  // If there is no new command, restart loop1.
  if (newCommand == false) return;

  // If there is a new command, set the indicator to false and run the command.
  newCommand = false;

  // Get code of command
  uint8_t commandCode = rp2040.fifo.pop();

  // Potentially needed variables
  uint16_t range;
  uint8_t fps;
  // Run command. Often can be interrupted.
  switch(commandCode) {
    // No op
    case 0:
      break;
    // Run the startup animation until interrupted.
    case 'a':
      startupAnimation();
      break;
    // Run the startup animation once.
    case 'A':
      singleStartupAnimation();
      writeAll(0);
      break;
    // Burn the current inputMatrix.
    case 'b':
      burnPattern();
      break;
    // Clear the intercore fifo queue.
    case 'c':
      rp2040.fifo.clear();
      break;
    // Run the diamond pattern with serial arguments.
    case 'd':
      if (rp2040.fifo.available()) {
        fps = rp2040.fifo.pop();
      }
      diamondPattern(fps);
      break;
    // Send the rp2040 to the bootloader.
    case 'e':
      endFirmware();
      break;
    // Display a fire until a new command is run.
    case 'f':
      fireplacePattern();
      break;
    // Display the Framwork gear spinning at a given framerate.
    case 'g':
      if (rp2040.fifo.available()) {
        fps = rp2040.fifo.pop();
      }
      rotateGear(fps);
      break;
    // Write the entire matrix PWM from the serial port.
    case 'm':
      serialWriteMatrix();
      break;
    // Write the entire matrix PWM from the serial port, then send a response
    // for blocking.
    case 'M':
      serialWriteMatrixBlocking();
      break;
    // Write the entire matrix scale from the serial port.
    case 'n':
      serialWriteMatrixScale();
      break;
    // Write the entire matrix scale from the serial port, then send a response
    // for blocking.
    case 'N':
      serialWriteMatrixScaleBlocking();
      break;
    // Set a given pixel's PWM using serial arguments.
    case 'p':
      if (rp2040.fifo.available() >= 3) {
        setPixel(rp2040.fifo.pop(), rp2040.fifo.pop(), rp2040.fifo.pop());
      } else {
        Serial.println("ERROR: too few arguments for setPixel");
      }
      break;
    // Set a given pixel's Scale using serial arguments.
    case 'q':
      if (rp2040.fifo.available() >= 3) {
        setPixelScale(rp2040.fifo.pop(), rp2040.fifo.pop(), rp2040.fifo.pop());
      } else {
        Serial.println("ERROR: too few arguments for setPixelScale");
      }
      break;
    // Display the ring with an fps from serial arguments.
    case 'r':
      if (rp2040.fifo.available()) {
        fps = rp2040.fifo.pop();
      }
      ringPattern(fps);
      break;
    // Set the scale of every LED.
    case 's':
      if (rp2040.fifo.available()) {
        writeAllScale(rp2040.fifo.pop());
      } else {
        Serial.println("ERROR: too few arguments for writeAllScale");
      }
      break;
    // Run a test patten on every LED and PWM.
    case 't':
      testAllPixel();
      break;
    // Set the PWM of every LED.
    case 'w':
      if (rp2040.fifo.available()) {
        writeAll(rp2040.fifo.pop());
      } else {
        Serial.println("ERROR: too few arguments for writeAllPWM");
      }
      break;
    // Print a known statement to confirm this firmware.
    case 127:
      Serial.print(versionStatement);
  }
}
