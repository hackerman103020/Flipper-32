# Flipper-32
A Flipper Zero-like device based on an ESP32.

## Supported Protocols
Flipper-32 supports the following IR protocols:
- **NEC, NECext, NEC42, NEC42ext**
- **Samsung32**
- **RC6, RC5, RC5X**
- **SIRC, SIRC15, SIRC20**
- **Kaseikyo**

**Note:** RCA and Pioneer protocols are *not* supported. RAW data support will be added soon.

---

## Setup Guide
### Requirements
To set up and upload the code to your ESP32, you will need:
- **Arduino IDE** (latest version recommended)
- **ESP32 board**
- **Required Libraries** (Install via Arduino Library Manager):
  - [Adafruit GFX Library (1.11.11)](https://github.com/adafruit/Adafruit-GFX-Library)
  - [Adafruit SSD1306 (2.5.13)](https://github.com/adafruit/Adafruit_SSD1306)
  - [SD Library (1.3.0)](https://www.arduino.cc/en/Reference/SD)
  - [ezButton (1.0.6)](https://github.com/ArduinoGetStarted/ezButton)
  - [IRMP (3.6.4)](https://github.com/ukw100/IRMP)

### Installation Steps
1. **Download Files**
   - Download `ir-parsed-testing.ino` and `PinDefinitionsAndMore.h`.
   - Open `Arduino IDE`, create a new project, and copy the code into it.
   - Alternatively, open `ir-parsed-testing.ino`, add a new tab in Arduino IDE, and copy-paste `PinDefinitionsAndMore.h`. Make sure the filename matches.

2. **Connect ESP32**
   - Plug in your ESP32 to your computer via USB.
   - Select the correct **board** and **port** in Arduino IDE.

3. **Upload Code**
   - Click **Upload** and wait for the process to complete.

---

## Required Components
- **ESP32** microcontroller
- **4-pin 64x128 OLED Display**
- **MicroSD card to SD card adapter**
- **4 buttons**
- **IR LED**
- **Breadboard / Solder-board and wires**

---

## Wiring Guide
### **SD Card to ESP32 Wiring**
| SD Card Pin | ESP32 GPIO Pin |
|------------|----------|
| 1          | 5        |
| 2          | 23       |
| 3          | GND      |
| 4          | 3.3V     |
| 5          | 18       |
| 6          | GND      |
| 7          | 19       |

![SD Card Wiring](https://github.com/user-attachments/assets/cc19777b-53b5-4adb-aee7-3fdfbca30b4f)

### **OLED Screen to ESP32 Wiring**
| OLED Pin | ESP32 GPIO Pin |
|---------|----------|
| VCC     | 3.3V     |
| GND     | GND      |
| SDA     | 21       |
| SCL     | 22       |

### **Button Wiring**
- Ensure each button is connected so that **ground and the pin are not connected unless the button is pressed**.

| Button | ESP32 GPIO Pin | Purpose |
|--------|----------|-----| 
| Button 1 | 12 | Plus   |
| Button 2 | 14 | Minus  |
| Button 3 | 27 | Equal  |
| Button 4 | 26 | Back   |

![Button Wiring](https://github.com/user-attachments/assets/7a499785-861d-4e20-a064-888535d4156e)

### **IR LED Wiring**
- **GPIO 4** to **+** of IR LED.
- **GND** to **-** of IR LED.

---

## Additional Modifications
- Added a **battery** to power the ESP32 via VIN and GND with a **switch** for easy control.

![Device Setup](https://github.com/user-attachments/assets/a5d47e33-c7c8-4f8e-8083-07ea85bd02af)

---

## Notes
- Ensure all connections are **secure** and **correct** before powering on the device.
- Double-check that all required **libraries** are installed in Arduino IDE.
- If you encounter issues, check the **Serial Monitor** for debugging information.
- Bluetooth functionality will be added soon!


---

Happy hacking! 🚀

