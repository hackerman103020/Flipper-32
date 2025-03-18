# Flipper-32
A flipper-zero like device, based on a ESP-32
# Protocols
it supports NEC, NECext, NEC42, NEC42ext, Samsung32, RC6, RC5, RC5X, SIRC, SIRC15, SIRC20, Kaseikyo, 
it does not support RCA or pioneer 



# How to set code up
you will need the ardunio IDE, a esp-32, and the following libraries: Adafruit GFX library (1.11.11), Adafruit SSD1306 (2.5.13), SD (1.3.0), ezButton (1.0.6), IRMP (3.6.4)
download ir-parsed-testing.ino and PinDefinitionsAndMore.h and put both in a Arduino IDE environment or add a new tab after opening the .ino and just copy and paste the code and name
then plug in your esp-32 and upload the code!



# Required parts
1. esp32
2. 4 pin 64x128 oled
3. micro sd card to sd card adapter
4. 4 buttons
5. breadoard / solder-board and wires


 # how to wire
sd cards pins to esp32 pins:
1 to 5
2 to 23
3 to gnd
4 to vcc(3.3v)
5 to 18
6 to gnd
7 to 19
![maxresdefault](https://github.com/user-attachments/assets/cc19777b-53b5-4adb-aee7-3fdfbca30b4f)
