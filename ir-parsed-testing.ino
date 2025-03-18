#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include "BluetoothSerial.h"
#include "PinDefinitionsAndMore.h"
//#include <IRremote.hpp>
#include <ezButton.h>
#include <irsndSelectAllProtocols.h>
#include <irsnd.hpp>
#include "TinyIRSender.hpp"


// OLED display settings 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <Arduino.h>
#include "PinDefinitionsAndMore.h"
#define IRSND_IR_FREQUENCY          38000

#if ! (defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
#define IRSND_PROTOCOL_NAMES        1
#endif
//#define IRSND_GENERATE_NO_SEND_RF // for back to back tests

IRMP_DATA irsnd_data;

// Bluetooth
BluetoothSerial SerialBT;

// SD Card
#define CS_PIN 5
File currentDir;
String currentPath = "/";

// File browsing
String fileList[1000];
int fileCount = 0;
int selectedIndex = 0;
int topIndex = 0;
const int maxDisplayLength = 20;
const int maxVisibleItems = 5;
bool isAtRoot = true;

// Scroll position memory system
const int MAX_DEPTH = 10;
int currentDepth = 0;
int FileLoopBackIndex[MAX_DEPTH] = {0}; // Stores positions for each directory level

ezButton buttonPlus(12);
ezButton buttonMinus(14); 
ezButton buttonEqual(27);
ezButton buttonBack(26); 
bool buttonPlusReady = true;
bool buttonMinusReady = true;
bool buttonEqualReady = true;
bool buttonBackReady = true;

// IR Commands
#define MAX_COMMANDS 100
#define MAX_RAW_DATA 200
struct IRCommand {
  String name;
  String type;
  String protocol;
  uint32_t address;
  uint32_t command;
  int frequency;
  float dutyCycle;
  uint16_t rawData[MAX_RAW_DATA];
  String data;
  int rawDataLength;
};

IRCommand commands[MAX_COMMANDS];
int irCommandCount = 0;
int selectedCommandIndex = 0;
int topCommandIndex = 0;

enum AppState { STATE_BROWSING, STATE_COMMAND };
AppState currentState = STATE_BROWSING;

int parseProtocol(String protocolName) {
  protocolName.toLowerCase();
  if(protocolName == "nec") return 1;
  else if(protocolName == "necext") return 1;
  else if(protocolName == "nec42") return 1;
  else if(protocolName == "nec42ext") return 1;
  else if(protocolName == "samsung32") return 1;
  else if(protocolName == "rc5") return 1;
  else if(protocolName == "rc5x") return 1;
  else if(protocolName == "rc6") return 1;
  else if(protocolName == "sirc") return 1;
  else if(protocolName == "sirc15") return 1;
  else if(protocolName == "sirc20") return 1;
  else if(protocolName == "kaseikyo") return 1;
  return -1;
}




void setup() {
  Serial.begin(115200);
  //IrSender.begin();
  buttonPlus.setDebounceTime(10); 
  buttonMinus.setDebounceTime(10); 
  buttonEqual.setDebounceTime(10); 
  buttonBack.setDebounceTime(10); 
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.setRotation(0);
  
  SerialBT.begin("mitnick's Airpods");
  
  if(!SD.begin(CS_PIN)) {
    Serial.println("SD Card initialization failed!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,28);
    display.println("SD CARD FAILED");
    display.display();
    return;
  }
  
  currentDir = SD.open("/");
  listFiles();

    #if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
        || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
        delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
    #endif
    #if defined(ESP8266)
        Serial.println(); // to separate it from the internal boot output
    #endif

        irsnd_data.protocol;
        irsnd_data.address; // in decimel, not hex. 
        irsnd_data.command;
        irsnd_data.flags = 0;

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRMP));

    Serial.print(F("Send sample frequency="));
    Serial.print(F_INTERRUPTS);
    Serial.println(F(" Hz"));

    irsnd_init();
    irmp_irsnd_LEDFeedback(true); // Enable send signal feedback at LED_BUILTIN

    Serial.println(F("Send IR signals at pin " STR(IRSND_OUTPUT_PIN)));
    delay(1000);
}

void formatForIRMP(uint8_t address, uint8_t command, uint16_t &outAddress, uint16_t &outCommand) {
    uint8_t invertedAddress = ~address;  // Invert bits for NEC format
    uint8_t invertedCommand = ~command;  // Invert bits for NEC format

    outAddress = (invertedAddress << 8) | address;   // Properly order address
    outCommand = (invertedCommand << 8) | command;   // Properly order command
    irsnd_data.address = outAddress; // in decimel, not hex. 
    irsnd_data.command = outCommand;
}

void loop() {
  handleBluetoothInput();
  buttonPlus.loop(); 
  buttonMinus.loop(); 
  buttonEqual.loop(); 
  buttonBack.loop(); 

  if(currentState == STATE_BROWSING) {
    if(buttonPlus.isPressed() && buttonPlusReady == true){
      buttonPlusReady = false; 
      if(selectedIndex > 0) selectedIndex--;
      if(selectedIndex < topIndex) topIndex = selectedIndex;
      displayFileList();
    }
    if(buttonPlus.isReleased()){
      buttonPlusReady = true;
    }
    
     if(buttonMinus.isPressed() && buttonMinusReady == true) {
      buttonMinusReady = false;
      if(selectedIndex < fileCount-1) selectedIndex++;
      if(selectedIndex >= topIndex + maxVisibleItems)
        topIndex = selectedIndex - maxVisibleItems + 1;
      displayFileList();
    }
    if(buttonMinus.isReleased()){
      buttonMinusReady = true;
    }
    
    if(buttonEqual.isPressed() && buttonEqualReady == true) {
      buttonEqualReady = false;
      if(fileList[selectedIndex] == "../") {
        goBackToParent();
      } else if(fileList[selectedIndex].endsWith("/")) {
        // Save current position before entering folder
        FileLoopBackIndex[currentDepth] = selectedIndex;
        if(currentDepth < MAX_DEPTH-1) currentDepth++;
        
        String folderName = fileList[selectedIndex].substring(0, fileList[selectedIndex].length()-1);
        currentPath = constructPath(folderName);
        currentDir.close();
        currentDir = SD.open(currentPath.c_str());
        isAtRoot = (currentPath == "/");
        listFiles();
      } else {
        String fileName = fileList[selectedIndex];
        if(fileName.endsWith(".ir")) {
          // Save current position before opening IR file
          FileLoopBackIndex[currentDepth] = selectedIndex;
          
          File irFile = SD.open(currentPath + (currentPath == "/" ? "" : "/") + fileName);
          if(irFile) {
            parseIRFile(irFile);
            if(irCommandCount > 0) {
              currentState = STATE_COMMAND;
              selectedCommandIndex = 0;
              topCommandIndex = 0;
              displayCommandList();
              SerialBT.println("Loaded " + String(irCommandCount) + " commands");
            }
            irFile.close();
          }
        }
      }
    }
    if(buttonEqual.isReleased()){
      buttonEqualReady = true;
    }
    
    if(buttonBack.isPressed() && buttonBackReady == true) {
      buttonBackReady = false;
      goBackToParent();
            selectedIndex = FileLoopBackIndex[currentDepth];
    }
    if(buttonBack.isReleased()){
      buttonBackReady = true;
    }
  }

  // code by hackerman103020 on github 

  else if(currentState == STATE_COMMAND) {
    if(buttonPlus.isPressed() && buttonPlusReady == true) {
      buttonPlusReady = false;
      if(selectedCommandIndex > 0) selectedCommandIndex--;
      if(selectedCommandIndex < topCommandIndex)
        topCommandIndex = selectedCommandIndex;
      displayCommandList();
    }
    if(buttonPlus.isReleased()){
      buttonPlusReady = true;
    }
    
    if(buttonMinus.isPressed() && buttonMinusReady == true) {
      buttonMinusReady = false;
      if(selectedCommandIndex < irCommandCount-1) selectedCommandIndex++;
      if(selectedCommandIndex >= topCommandIndex + maxVisibleItems)
        topCommandIndex = selectedCommandIndex - maxVisibleItems + 1;
      displayCommandList();
    }
    if(buttonMinus.isReleased()){
      buttonMinusReady = true;
    }
    
    if(buttonEqual.isPressed() && buttonEqualReady == true) {
      buttonEqualReady = false;
      if(selectedCommandIndex < irCommandCount) {
        IRCommand cmd = commands[selectedCommandIndex];
        SerialBT.println("Command: " + cmd.name);
        SerialBT.println("Type: " + cmd.type);

        if(cmd.type == "raw") {
          //IrSender.sendRaw(cmd.rawData, cmd.rawDataLength, NEC_KHZ);
        } 
        else if(cmd.type == "parsed") {
          int protocol = parseProtocol(cmd.protocol);
          if(protocol != -1) {
            uint16_t formattedAddress, formattedCommand;
            formatForIRMP(cmd.address, cmd.command, formattedAddress, formattedCommand);
            irsnd_data.address = formattedAddress; 
            Serial.print("Address: ");
            Serial.print(irsnd_data.address);
            Serial.print(" and ");
            Serial.print(cmd.address);
            irsnd_data.command = formattedCommand;
            Serial.print("   commands: ");
            Serial.print(irsnd_data.command);
            Serial.print(" and ");
            Serial.print(cmd.command);
            Serial.print("    ");
            Serial.print("protocol: ");
            Serial.print(cmd.protocol);
            Serial.print("   ");
            irsnd_data.address = formattedAddress; 
            irsnd_data.command = formattedCommand;         
            irsnd_data.flags = 0;

            
            if(cmd.protocol == "NEC" || cmd.protocol == "NECext"){
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[1]); 
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }
              Serial.print("sent NEC  ");
            }
            else if(cmd.protocol == "NEC42" || cmd.protocol == "NEC42ext"){
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[23]); 
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }
              Serial.print("sent NEC42  ");
            }
            else if(cmd.protocol == "Samsung32"){ 
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[9]);
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }      
              Serial.print("sent Samsung32"); 
            }
            else if(cmd.protocol == "RC6") { 
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[8]);
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }
              Serial.print("sent RC6"); 
            }
            else if(cmd.protocol == "RC5" || cmd.protocol == "RC5X"){ 
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[6]);
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }      
              Serial.print("sent RC5"); 
            }
            else if(cmd.protocol == "SIRC" || cmd.protocol == "SIRC15" || cmd.protocol == "SIRC20" ){ 
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[0]);
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }      
              Serial.print("sent SIRCS"); 
            }
            else if(cmd.protocol == "Kaseikyo") {
              irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[4]); 
              if (!irsnd_send_data(&irsnd_data, true))
                {
                Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
                Serial.print(cmd.protocol);
                }
              Serial.print("sent Kaseikyo"); 
            }
          }
        }
      }
    }
    if(buttonEqual.isReleased()){
      buttonEqualReady = true;
    }
    
    if(buttonBack.isPressed() && buttonBackReady == true) {
      buttonBackReady = false;
      currentState = STATE_BROWSING;
      // Restore previous file list position
      selectedIndex = FileLoopBackIndex[currentDepth];
      displayFileList();
    }
    if(buttonBack.isReleased()){
      buttonBackReady = true;
    }
  }
}

void goBackToParent() {
  if(currentDepth > 0) {
    currentDepth--;
    selectedIndex = FileLoopBackIndex[currentDepth];
    //FileLoopBackIndex[currentDepth] = 0;
  }
  
  if(currentPath != "/") {
    int lastSlash = currentPath.lastIndexOf('/');
    currentPath = currentPath.substring(0, lastSlash > 0 ? lastSlash : 1);
  }
  currentDir.close();
  currentDir = SD.open(currentPath.c_str());
  isAtRoot = (currentPath == "/");
  listFiles();
}

void listFiles() {
  fileCount = 0;
  if(!isAtRoot) fileList[fileCount++] = "../";

  currentDir.rewindDirectory();
  while(true) {
    File entry = currentDir.openNextFile();
    if(!entry) break;
    
    if(entry.isDirectory()) {
      fileList[fileCount++] = String(entry.name()) + "/";
    } else {
      fileList[fileCount++] = String(entry.name());
    }
    entry.close();
    if(fileCount >= 1000) break;
  }
  
  // Restore saved position if available
  if(FileLoopBackIndex[currentDepth] < fileCount) {
    selectedIndex = FileLoopBackIndex[currentDepth];
    //FileLoopBackIndex[currentDepth] = 0;
  } else {
    selectedIndex = 0;
  }
  
  topIndex = (selectedIndex / maxVisibleItems) * maxVisibleItems;
  displayFileList();
}

void displayFileList() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int itemsToDisplay = min(fileCount - topIndex, maxVisibleItems);
  
  for(int i=0; i<itemsToDisplay; i++) {
    int index = topIndex + i;
    bool selected = (index == selectedIndex);
    
    display.drawRect(0, i*12, SCREEN_WIDTH, 12, SSD1306_WHITE);
    
    if(selected) {
      display.fillRect(0, i*12, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    
    String name = fileList[index];
    if(name.length() > maxDisplayLength)
      name = name.substring(0, maxDisplayLength-3) + "~";
      
    display.setCursor(2, i*12 + 2);
    display.print(name);
    
    display.setCursor(SCREEN_WIDTH-12, i*12 + 2);
    display.print(name.endsWith("/") ? 'F' : 'f');
  }
  display.display();
}

void displayCommandList() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int itemsToDisplay = min(irCommandCount - topCommandIndex, maxVisibleItems);
  
  for(int i=0; i<itemsToDisplay; i++) {
    int index = topCommandIndex + i;
    bool selected = (index == selectedCommandIndex);
    
    display.drawRect(0, i*12, SCREEN_WIDTH, 12, SSD1306_WHITE);
    
    if(selected) {
      display.fillRect(0, i*12, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    
    String name = commands[index].name;
    if(name.length() > maxDisplayLength-3)
      name = name.substring(0, maxDisplayLength-3) + "~";
      
    char typeChar = commands[index].type == "raw" ? 'R' : 'P';
    display.setCursor(2, i*12 + 2);
    display.print(typeChar + String(" ") + name);
    
    display.setCursor(SCREEN_WIDTH-24, i*12 + 2);
    if(commands[index].protocol == "NEC") display.print("N");
    else if(commands[index].protocol != "") display.print("P");
  }
  display.display();
}

String constructPath(String folderName) {
  if(folderName == "../") {
    if(currentPath != "/") {
      int lastSlash = currentPath.lastIndexOf('/');
      currentPath = currentPath.substring(0, lastSlash > 0 ? lastSlash : 1);
    }
  } else {
    currentPath += (currentPath == "/" ? "" : "/") + folderName;
  }
  return currentPath;
}

void parseIRFile(File &file) {
  irCommandCount = 0;
  String line = "";
  IRCommand currentCmd;
  bool commandInProgress = false;

  while(file.available()) {
    char c = file.read();
    
    if(c == '\n' || c == '\r') {
      if(line.length() > 0) {
        if(line.startsWith("#")) {
          if(commandInProgress) {
            if((currentCmd.type == "raw" && currentCmd.rawDataLength > 0) ||
               (currentCmd.type == "parsed" && currentCmd.protocol != "")) {
              if(irCommandCount < MAX_COMMANDS) {
                commands[irCommandCount++] = currentCmd;
              }
            }
            currentCmd = IRCommand();
            commandInProgress = false;
          }
        }
        else {
          commandInProgress = true;
          if(line.startsWith("name: ")) currentCmd.name = line.substring(6);
          else if(line.startsWith("type: ")) currentCmd.type = line.substring(6);
          else if(line.startsWith("protocol: ")) currentCmd.protocol = line.substring(10);
          else if(line.startsWith("address: ")) currentCmd.address = strtoul(line.substring(9).c_str(), NULL, 16);
          else if(line.startsWith("command: ")) currentCmd.command = strtoul(line.substring(9).c_str(), NULL, 16);
          else if(line.startsWith("frequency: ")) currentCmd.frequency = line.substring(11).toInt();
          else if(line.startsWith("duty_cycle: ")) currentCmd.dutyCycle = line.substring(12).toFloat();
          else if(line.startsWith("data: ")) {
            parseRawData(line.substring(6), currentCmd);
            currentCmd.data = line.substring(6);
          }
        }
        line = "";
      }
    } else {
      line += c;
    }
  }

  if(commandInProgress) {
    if((currentCmd.type == "raw" && currentCmd.rawDataLength > 0) ||
       (currentCmd.type == "parsed" && currentCmd.protocol != "")) {
      if(irCommandCount < MAX_COMMANDS) {
        commands[irCommandCount++] = currentCmd;
      }
    }
  }
}

void parseRawData(String dataStr, IRCommand &cmd) {
  int dataIndex = 0;
  int startPos = 0;
  
  while(startPos < dataStr.length() && dataIndex < MAX_RAW_DATA) {
    int endPos = dataStr.indexOf(' ', startPos);
    if(endPos == -1) endPos = dataStr.length();
    
    String numStr = dataStr.substring(startPos, endPos);
    cmd.rawData[dataIndex++] = numStr.toInt();
    startPos = endPos + 1;
  }
  cmd.rawDataLength = dataIndex;
}

void handleBluetoothInput() {
  if(SerialBT.available()) {
    char input = SerialBT.read();
    // Add Bluetooth navigation handling here if needed
  }
}