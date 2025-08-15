/*
  Written by sigroot (github.com/sigroot)

  rp2040_firmware.h - Definitions for LEDMatrix input module communication.
  
*/

#define SIG_FIRMWARE 1

#include <Wire.h>


//    *** LED Matrix Controller constant register addresses, keys, defaults, and constants ***

// The 7 bit I2C address of LED Matrix Controller.
// The two lowest bits are determined by the grounded ADDR pin.
const uint8_t LEDMatrixControllerAddress = 0b0110000;

// The register that must be written to with the associated key to unlock the 
// command register for writing.
const uint8_t commandRegisterLock = 0xFE;

// The key used to unlock the command register by writing it to the command
// register lock.
const uint8_t commandLockKey = 0xC5;

// The register that determines the current page of RAM being written to.
const uint8_t commandRegister = 0xFD;

// The register containing the 8 bit address of the LED Matrix Controller.
// This should be equal to LEDMatrixControllerAddress with an appended '0'.
const uint8_t IDRegister = 0xC5;

// The register that determines the PWM frequency.
// The highest 4 bits are always 0000.
// Low four bits settings:
//  0b0000 - 29kHz (default)
//  0b0011 - 3.6kHz
//  0b0111 - 1.8kHz
//  0b1011 - 900Hz
const uint8_t PWMRegister = 0x36;

// Set the PWM frequency setting register to 29kHz.
uint8_t PWMRegisterDefault = 0b00000000;

// The register that determines what pull up/down reistors are used.
// The highest 4 bits are for the rows' pull down resistors.
// The lowest 4 bits are for the column's pull up resistors.
// The highest bit of both is 0.
// Low 3 bits of both:
//  0b000 - No resistor
//  0b001 - 0.5kOhms
//  0b011 - 1.0kOhms
//  0b011 - 2.0kOhms
//  0b100 - 4.0kOhms
//  0b101 - 8.0kOhms
//  0b110 - 16kOhms
//  0b111 - 32kOhms
const uint8_t PUDResistorsRegister = 0x02;

// Set the pull down/up resistors to 16k ohms each.
uint8_t PUDResistorsRegisterDefault = 0b01100110;

// The register that resets all of the LED Matrix Controller's registers.
// The reset register is activated by writing the value 0xAE to it.
const uint8_t resetRegister = 0x3F;

// The key that activates the reset register once it it written to it.
const uint8_t resetRegisterKey = 0xAE;

// The register that controls the proportion of current allowed to flow through
// the LEDs. 
// This can be set from 0 (no current) to 255 (full current).
const uint8_t globalCurrentControlRegister = 0x01;

// Set the current control register to 255.
uint8_t globalCurrentDefault = 0xFF;

// The register that controls the LED Matrix controller's configuration 
// options.
//  The highest four bits control which columns are skipped while writing LEDs
//    the value is the count (in binary) of columns skiped starting with SW1 
//    (the right side of the matrix)
//  The next highest bit controls the logic voltage level on the matrix 
//  controller's pins:
//    0 - Low < 0.4 V, High > 1.4V
//    1 - Low < 0.6 V, High > 2.4V 
//  The next two highest bits control open and short detection
//    01 - Trigger open detection once
//    10 - Trigger short detection once
//  The next highest bit controls whether the LED Matrix controller is in
//  software shutdown mode. Software shutdown mode disables the display to
//  save power.
//    0 - Software shutdown mode
//    1 - Normal Operation mode
const uint8_t configRegister = 0x00;

// Set the SSD bit to normal operation and the logic bit to high voltage.
uint8_t configRegisterDefault = 0b00001001;

// This page contains the first page of PWM registers:
const uint8_t PWMPage1 = 0x00;

// This page contains the second page of PWM registers:
const uint8_t PWMPage2 = 0x01;

// This page contains the first page of scaling registers:
const uint8_t scalePage1 = 0x03;

// This page contains the second page of scaling registers:
const uint8_t scalePage2 = 0x04;

// This page contains the function registers:
const uint8_t functionPage = 0x04;

// This is the x-width of the LED matrix
const uint8_t LEDWidth = 9;

//This is the y-height of the LED matrix
const uint8_t LEDHeight = 34;

// This is the number of LED registers on pages 0 and 2
const uint8_t regPage0 = 0xB3;

// This is the number of LED registers on pages 1 and 3
const uint8_t regPage1 = 0xAA;


//    *** RP2040 Pins by GPIO value ***

// GPIO Pin numbers used on the rp2040 I2C.
const int SDAPin = 26;

// GPIO Pin numbers used on the rp2040 I2C.
const int SCLPin = 27;

// GPIO pin number connected to the matrix controller Shutdown pin (SDB).
const int SDBPin = 29;


//    *** I2C constants ***

// Set the I2C clock rate.
const int i2cClockRate = 400000;


//    *** Structs ***

// Allows an error code to be returned with a value.
struct Result {
  uint8_t value;
  uint8_t error;
};


//    ** Global Variables ***
uint8_t currentPage = 0xFF;


//    *** Functions ***

// Changes the curve of PWM brightness to be more visible across its entirety
// by the human eye.
uint8_t getGamma(uint8_t pwm) {
  return uint16_t(pwm)*uint16_t(pwm)/255;
}


//  *** LED matrix command functions ***

// Every time the command register is written to, it must be unlocked first.
// Unlocks the Command Register. Returns the I2C error code.
uint8_t unlockCommandRegister() {
  // Begin writing the I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the address of the command unlock register to the I2C message.
  Wire1.write(commandRegisterLock);

  // Add the value of the unlock byte to the I2C message.
  Wire1.write(commandLockKey);
  
  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error = Wire1.endTransmission();

  // Return the transmission error code.
  return error;
}

// Selects a page of RAM to begin writing to. Returns the I2C error code.
uint8_t selectPage(uint8_t page){
  // If the page is outside the selectable range, return an error
  if (page > 4) {
    return 5;
  }

  // If the currently selected page is the one requested, no action is necessary
  if (currentPage == page) {
    return 0;
  }

  // Unlock the command register and save the I2C error code.
  uint8_t unlockErr = unlockCommandRegister();

  // Begin writing the I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the address of the command register to the I2C message.
  Wire1.write(0xFD);

  // Add the desired page of RAM to the I2C message.
  Wire1.write(page);

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error = Wire1.endTransmission();
  
  // If there have been no errors, change the known current page
  // If there was an error, set the current known page to an impossible value.
  if (max(error, unlockErr) == 0) {
    currentPage = page;
  } else {
    currentPage = 0xFF;
  }
  
  // Return a non-zero error code if either transmission had an error.
  return max(error, unlockErr);
}

// Read the current value from a register on a page. 
Result readCommand(uint8_t page, uint8_t reg) {
  // Select the page to read from.
  uint8_t pageErr = selectPage(page);

  // Select the register to read from.
  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for the write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the desired register to read from to the I2C message.
  Wire1.write(reg);

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error = Wire1.endTransmission();
  
  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 1 for the read bit.
  Wire1.requestFrom(LEDMatrixControllerAddress, 1);

  // Recieve the value from the LED Matrix controller.
  uint8_t response = Wire1.read();

  // Return a Result with both the value returned and the error code.
  return {response, max(pageErr, error)};
}

// Write to a register on a page.
// Pages 1-2 are for PWM, Pages 3-4 are for Scaling.
// See IS31FL3741A (LED matrix) datasheet for details.
uint8_t writeCommand(uint8_t page, uint8_t reg, uint8_t value){
  // Select the page to write to.
  uint8_t pageErr = selectPage(page);

  // Select the register to write to.
  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the desired register to read from to the I2C message.
  Wire1.write(reg);

  // Add the desired value for the register to the I2C message.
  Wire1.write(value);

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error = Wire1.endTransmission();

  // Return a non-zero error code if either operation had an error.
  return max(pageErr, error);
}

// Set the PWM of each visible LED using the LED Matrix's Address Auto 
// Increment mode.
uint8_t writeAll(uint8_t value) {
  // Switch to page 0
  uint8_t pageErr1 = selectPage(0);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register to the I2C message.
  for (int i = 0; i <= 0xB3; i++) {
    Wire1.write(value);
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error1 = Wire1.endTransmission();

  // Switch to page 1
  uint8_t pageErr2 = selectPage(1);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register to the I2C message.
  for (int i = 0; i <= 0xAA; i++) {
    // Skip unused LEDs
    if (i >= 60 && i <= 89) {
      Wire1.write(0);
      continue;
    }
    if (i == 116 || i == 125 || i == 134 || i == 143 || i == 152) {
      Wire1.write(0);
      continue;
    }
    
    Wire1.write(value);
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error2 = Wire1.endTransmission();

  // Return a non-zero error code if any operation had an error.
  return max(pageErr1, max(pageErr2, max(error1, error2)));
}

// Set the Scale of each visible LED using the LED Matrix's Address Auto 
// Increment mode.
// *NOTE: excessive scale (LED current) may damage the LED Matrix (usually 
// kept to 0x7F).
uint8_t writeAllScale(uint8_t value) {
  // Switch to page 0
  uint8_t pageErr1 = selectPage(2);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register to the I2C message.
  for (int i = 0; i <= 0xB3; i++) {
    Wire1.write(value);
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error1 = Wire1.endTransmission();

  // Switch to page 1
  uint8_t pageErr2 = selectPage(3);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register to the I2C message.
  for (int i = 0; i <= 0xAA; i++) {
    // Skip unused LEDs
    if (i >= 60 && i <= 89) {
      Wire1.write(0);
      continue;
    }
    if (i == 116 || i == 125 || i == 134 || i == 143 || i == 152) {
      Wire1.write(0);
      continue;
    }
    
    Wire1.write(value);
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error2 = Wire1.endTransmission();

  // Return a non-zero error code if any operation had an error.
  return max(pageErr1, max(pageErr2, max(error1, error2)));
}

// Writes pwm values to each LED matching the values of an inputted matrix by 
// using the LED Matrix's Address Auto Increment mode.
uint8_t writeMatrix(uint8_t inputMatrix[LEDHeight][LEDWidth], bool useGamma = false) {
  // Switch to page 0
  uint8_t pageErr1 = selectPage(0);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register from the matrix to the I2C message.
  for (int i = 0; i <= 0xB3; i++) {
    uint8_t x = (LEDWidth-1)-i/30;
    uint8_t y = i%30;
    if (useGamma) {
      Wire1.write(getGamma(inputMatrix[y][x]));
    } else {
      Wire1.write(inputMatrix[y][x]);
    }
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error1 = Wire1.endTransmission();

  // Switch to page 1
  uint8_t pageErr2 = selectPage(1);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register from the matrix to the I2C message.
  for (int i = 0; i <= 0xA0; i++) {
    uint8_t x;
    uint8_t y;

    // Translate the current LED number to x and y coordinates
    if (i <= 59) {
      x = 2-i/30;
      y = i%30;
    } 
    // Skip unused LEDs
    else if (i <= 89 || i == 116 || i == 125 || i == 134 || i == 143 || i == 152) {
      Wire1.write(0);
      continue;
    } 
    // Write bottom rows of LEDs (highest 4 y values)
    else if ((i-90) % 9 <= 3) {
      x = (LEDWidth-1) - (i-90) / 9;
      y = (i-90) % 9 + 30;
    } 
    // Write left most column of LEDs
    else if (i <= 107) {
      x = 0;
      if ((i-90) % 9 == 4) {
        y = ((i-90) / 9) * 5 + 4;
      } else {
        y = ((i-90) / 9) * 5 + (((i-90) % 9) - 5);
      }
    } else {
      x = 0;
      if ((i-90) % 9 == 4) {
        y = ((i-90) / 9 + 1) * 4 + 1;
      } else {
        y = (((i-90) / 9) * 4 + 2) + (((i-90) % 9) - 5);
      }
    }
    if (useGamma) {
      Wire1.write(getGamma(inputMatrix[y][x]));
    }else {
      Wire1.write(inputMatrix[y][x]);
    }
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error2 = Wire1.endTransmission();

  // Return a non-zero error code if any operation had an error.
  return max(pageErr1, max(pageErr2, max(error1, error2)));
}

// Writes scale values to each LED matching the values of an inputted matrix by 
// using the LED Matrix's Address Auto Increment mode.
uint8_t writeMatrixScale(uint8_t inputMatrix[LEDHeight][LEDWidth], bool useGamma = false) {
  // Switch to page 2
  uint8_t pageErr1 = selectPage(2);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register from the matrix to the I2C message.
  for (int i = 0; i <= 0xB3; i++) {
    uint8_t x = (LEDWidth-1)-i/30;
    uint8_t y = i%30;
    if (useGamma) {
      Wire1.write(getGamma(inputMatrix[y][x]));
    } else {
      Wire1.write(inputMatrix[y][x]);
    }
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error1 = Wire1.endTransmission();

  // Switch to page 3
  uint8_t pageErr2 = selectPage(3);

  // Begin writing I2C message. 7 bits of LED Matrix controller address plus 0 for write bit.
  Wire1.beginTransmission(LEDMatrixControllerAddress);

  // Add the first register to the I2C message.
  Wire1.write(0x00);

  // Add the desired value for each register from the matrix to the I2C message.
  for (int i = 0; i <= 0xA0; i++) {
    uint8_t x;
    uint8_t y;

    // Translate the current LED number to x and y coordinates
    if (i <= 59) {
      x = 2-i/30;
      y = i%30;
    } 
    // Skip unused LEDs
    else if (i <= 89 || i == 116 || i == 125 || i == 134 || i == 143 || i == 152) {
      Wire1.write(0);
      continue;
    } 
    // Write bottom rows of LEDs (highest 4 y values)
    else if ((i-90) % 9 <= 3) {
      x = (LEDWidth-1) - (i-90) / 9;
      y = (i-90) % 9 + 30;
    } 
    // Write left most column of LEDs
    else if (i <= 107) {
      x = 0;
      if ((i-90) % 9 == 4) {
        y = ((i-90) / 9) * 5 + 4;
      } else {
        y = ((i-90) / 9) * 5 + (((i-90) % 9) - 5);
      }
    } else {
      x = 0;
      if ((i-90) % 9 == 4) {
        y = ((i-90) / 9 + 1) * 4 + 1;
      } else {
        y = (((i-90) / 9) * 4 + 2) + (((i-90) % 9) - 5);
      }
    }
    if (useGamma) {
      Wire1.write(getGamma(inputMatrix[y][x]));
    }else {
      Wire1.write(inputMatrix[y][x]);
    }
  }

  // Push the I2C message over wire. Can return an error code (non-zero is error).
  uint8_t error2 = Wire1.endTransmission();

  // Return a non-zero error code if any operation had an error.
  return max(pageErr1, max(pageErr2, max(error1, error2)));
}

// Set the PWM frequency setting register.
uint8_t setPWMFrequencyRegister(uint8_t value) {
  // Set the value for PWM frequency setting register.
  // (eg. 0b 
  //  0000 [always]
  //  0000 [PFS - set PWM frequency setting to 29kHz]
  //  )
  return writeCommand(functionPage, PWMRegister, value);
}

// Set the pull down/up resistors on the LEDs.
uint8_t setPullResistorsRegister(uint8_t value) {
  // Set the value for the pull down/up resistors register .
  // (eg. 0b 
  //  0 [always]
  //  101 [PDR - set row pull down resistors to 8k ohms]
  //  0 [always]
  //  101 [PUR - set column pull up resistors to 8k ohms] 
  //  )
  return writeCommand(functionPage, PUDResistorsRegister, value);
}

// Use the reset register to reset all of the LED matrix register values to 
// their default values. 
uint8_t setResetRegister() {
  // If the reset register is set to 0xAE, the matrix controller's registers 
  // are reset.
  return writeCommand(functionPage, resetRegister, resetRegisterKey);
}

// Set the value of the global current control register.
uint8_t setGlobalCurrentControlRegister(uint8_t value) {
  // Set the value of the global current control register in 255 step 
  // increments.
  return writeCommand(functionPage, globalCurrentControlRegister, value);
}

// Set the value of the configuration register.
uint8_t setConfigurationRegister(uint8_t value) {
  // Set the value of the configuration register .
  // (eg. 0b 
  //  0000 [SWS - scan all columns {SW pins}]
  //  0 [LGC - Set logic voltage to 1.4 V high/0.4 V low]
  //  00 [OSDE - Disable open and short detection] 
  //  1 [SSD - Set to normal operation]
  //  )
  return writeCommand(functionPage, configRegister, value);
}

// Set the function registers of the LED matrix to resonable defaults.
void setFunctionRegistersDefault() {
  // Set the SSD bit to normal operation.
  setConfigurationRegister(configRegisterDefault);
  // Set the current control to default.
  setGlobalCurrentControlRegister(globalCurrentDefault);
  // Set the pull down/up resistors to default.
  setPullResistorsRegister(PUDResistorsRegisterDefault);
  // Set the PWM frequency setting register to default.
  setPWMFrequencyRegister(PWMRegisterDefault);
}

// Reset LED matrix and then set default values
void matrixReset() {
  setResetRegister();
  setFunctionRegistersDefault();
}


//    *** Abstractions ***

// Set a pixel on the matrix based on x and y coordinants to a brightness 
// based on pwm.
uint8_t setPixel(uint8_t x, uint8_t y, uint8_t pwm) {
  // LEDNumber is the LED Matrix controller's ID for the LED being modified.
  // page is the page of ram to be modified.
  uint8_t LEDNumber, page;

  // Coordinate outside of range
  if (x < 0 || x > LEDWidth || y < 0 || y > LEDHeight) {
    return 1;
  }

  // Write if the coordinate is on page 0
  if (x >= 3 && y <=29) {
    LEDNumber = 30*(8-x) + y;
    page = 0;
  }

  // Write if the coordiate is on page 1
  else if (1 <= x && x <=2 && 0 <= y && y <= 29) {
    LEDNumber = 30*(2-x) + y;
    page = 1;
  }
  else if (1 <= x && 30 <= y) {
    LEDNumber = 9*(8-x) + (y-30) + 90;
    page = 1;
  }
  else if (x == 0) {
    page = 1;
    if (y <= 9) {
      LEDNumber = ((y+1)%5) + (9*(y/5)) + 94;
    } else {
      LEDNumber = ((y-1)%4) + (9*((y-2)/4)) + 94;
    }
  }
  else {
    // Somehow the function completely missed the inputted value despite it 
    // being within range.
    return 2;
  }

  writeCommand(page, LEDNumber, pwm);
  return 0;
}

// Set a pixel on the matrix based on x and y coordinants to a current scale.
// *NOTE: excessive scale (LED current) may damage the LED Matrix (default 
// scale is 0x7F).
// Current I = 383
uint8_t setPixelScale(uint8_t x, uint8_t y, uint8_t scale) {
  // LEDNumber is the LED Matrix controller's ID for the LED being modified.
  // page is the page of ram to be modified.
  uint8_t LEDNumber, page;

  // Coordinate outside of range
  if (x < 0 || x > LEDWidth || y < 0 || y > LEDHeight) {
    return 1;
  }

  // Write if the coordinate is on page 0
  if (x >= 3 && y <=29) {
    LEDNumber = 30*(8-x) + y;
    page = 2;
  }

  // Write if the coordiate is on page 1
  else if (1 <= x && x <=2 && 0 <= y && y <= 29) {
    LEDNumber = 30*(2-x) + y;
    page = 3;
  }
  else if (1 <= x && 30 <= y) {
    LEDNumber = 9*(8-x) + (y-30) + 90;
    page = 3;
  }
  else if (x == 0) {
    page = 3;
    if (y <= 9) {
      LEDNumber = ((y+1)%5) + (9*(y/5)) + 94;
    } else {
      LEDNumber = ((y-1)%4) + (9*((y-2)/4)) + 94;
    }
  }
  else {
    // Somehow the function completely missed the inputted value despite it 
    // being within range.
    return 2;
  }

  writeCommand(page, LEDNumber, scale);
  return 0;
}


//    *** rp2040 functions ***

// Send the rp2040 to the bootloader to change the uf2 file.
void bootloader() {
  rom_reset_usb_boot(0, 0);
}

// Prepare the rp2040 for I2C with the LED matrix controller.
void wireInit() {
  // Set the LED matrix controller shutdown pin to active (Default GPIO 29).
  pinMode(SDBPin, OUTPUT);
  digitalWrite(SDBPin, HIGH);
  
  // Ensure the controller is not shut down.
  if (digitalRead(SDBPin) == LOW) {
    rom_reset_usb_boot(0, 0);
  }

  // Redefine the I2C Pins to the documented rp2040 values (Default GPIO 26 & 27).
  Wire1.setSDA(SDAPin);
  Wire1.setSCL(SCLPin);
}