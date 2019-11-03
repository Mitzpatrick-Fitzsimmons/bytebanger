/*  
 *   MITZ I²C™ EEPROM BYTEBANGER v0.71
 *   for I²C™ Serial Interface EEPROM Devices
 *   Created by Mitzpatrick Fitzsimmons
 *   CopyLeft April 20, 2019 All writes reserved.
 *   
 */

/* The Serial EEPROM I²C™ Address must be defined for any communication to
    the device to work properly. The Serial EEPROM address can be set using
    the first 3 pins A0, A1, A2 on the Serial EEPROM device. (see Datasheet)

    Up to 8 address locations can be set via the A0 A1 A2 pins, which means that up to
    8 Serial EEPROMS can be cascaded on the same 2-wire I²C™ interfaced Serial bus.
    The HEX address location we define for our code will determine which Serial EEPROM
    device we will communicate to on the I²C™ Serial bus. (default is 0x50)
*/

/* WRITE PROTECTION & safetyMode
    The I²C™ Serial EEPROM has Write Protection capability via pin7 (see Datasheet for your model).
    To enable Write Protection, the WP pin is tied HIGH to Vcc. It can be tied LOW to Vss (ground) 
    or left floating (not recommended) to disable Write Protection.

    The MITZ I²C™ BYTEBANGER provides a SAFEMODE option to emulate Write Protection, so the WP pin
    of the EEPROM should be tied to ground to allow data to be written when SAFEMODE is turned off.
    SAFEMODE is ON by default and can be turned OFF by either changing the sftyMd variable value to 
    false, or from the Settings Menu in the serial monitor.
*/
#include "Wire.h"
#include <SPI.h>
#include "SdFat.h"
//#define CSV_DELIM ','
char CSV_DELIM=',';
//#define SD_CS_PIN 8
byte SD_CS_PIN=8;

bool sftyMd         = true;     /* false = SAFETY MODE OFF || true = SAFETY MODE ON */
bool EEPROMset      = false;     /* false = EEPROM not set || true = EEPROM set */
bool MainMenuMode   = true;      /* set true by default */
bool SettingsMode   = false;     /* set to false by default */
bool readDataMode   = false;     /* set to false by default */
bool setEEAdrMode   = false;     /* set to false by default */
bool setEETypeMode  = false;     /* set to false by default */
bool setReadMode    = false;     /* set to false by default */
bool setWriteMode   = false;     /* set to false by default */
bool wrSNMode       = false;     /* set to false by default */
bool rangeWriteMode = false;     /* set to false by default */
bool rangeReadMode  = false;     /* set to false by default */
bool byteScanMode   = false;     /* set to false by default */
bool EraseMode      = false;     /* set to false by default */
bool saveSetMode    = false;     /* set to false by default */
bool sdReady        = false;     /* set to false by default */
bool EE2SDMode      = false;     /* set to false by default */
bool SD2EEMode      = false;     /* set to false by default */
//const PROGMEM String stuff[]  ={"ASCII", "BINARY", "DECIMAL", "HEXIDECIMAL", "OCTAL", "K-bit"};
const String stuff[]  ={"ASCII", "BINARY", "DECIMAL", "HEXIDECIMAL", "OCTAL", "K-bit"};
//const PROGMEM String menu[]   ={"Settings", "SD FileList", "ByteScan", "RangeRead", "ReadEEprom", "EEprom2SD", "SD2EEprom", "SaveSettings", "RangeWrite", "Write SN"};
String menu[]   ={"Settings", "SD FileList", "ByteScan", "RangeRead", "ReadEEprom", "EEprom2SD", "SD2EEprom", "SaveSettings", "RangeWrite", "Write SN"};

byte spkr = 7;        /* SPEAKER PIN */
byte eeAdr = 0x50;    /* DEFAULT EEPROM ADDRESS */
int eeType;           /* EEPROM TYPE */
long eeMax;           /* EEPROM MAXIMUM BYTES */
byte rdMode=68;       /* ASCII=65 BIN=66 DEC=68 HEX =72 OCT 79 */
byte wrMode=68;       /* ASCII=65 BIN=66 DEC=68 HEX =72 OCT 79 */
byte counter2=7;
byte autoDt = 23;       /* Number used for AutoDetect function */
SdFat SD;


void setup(){
  Wire.begin();
  Serial.begin(115200);
  pinMode(spkr, OUTPUT);
  scanI2C();
}

void loop() { 
  modeMGR();  
}

byte scanI2C(){
    byte error, address;
    int nDevices=0;
    for (address = 80; address <= 87; address++){
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0){
      Serial.print(F("Found I²C device at: 0x"));
      Serial.println(address, HEX);
      funBeep(1);
      nDevices++;
    }
    else if (error == 4){
      Serial.print(F("\t\t\tUnknown error at address 0x"));
      if (address < 16) {
        Serial.print(F("0"));
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    errorMessage(6);
    return; 
  }
  else {
    //Serial.println("Done.\n");
    //delay(3000);
    loadSettings();
    prLn(8);
    menuMGR();
  }
}
void modeMGR() { /* Directive to manage modes. Only one mode can be active at any one time */

  if        (MainMenuMode)    { MainMenuOptions();        } 
    else if (SettingsMode)    { SettingsMenuOptions();    } 
    else if (readDataMode)    { readData();               } 
    else if (setEEAdrMode)    { setEEAdrMenuOptions();    } 
    else if (setEETypeMode)   { setEETypeMenuOptions();   } 
    else if (wrSNMode)        { wrSNMenuOptions();     }
    else if (rangeWriteMode)  { rangeWriteMenuOptions();  }
    else if (rangeReadMode)   { rangeReadMenuOptions();   }
    else if (byteScanMode)    { byteScanMenuOptions(); }
    else if (setReadMode)     { setReadModeMenuOptions(); }
    else if (setWriteMode)    { setWriteModeMenuOptions();}
    else if (EraseMode)       { EraseModeMenuOptions();   }
    else if (saveSetMode)     { saveSetMenuOptions();     }
    else if (EE2SDMode)       { EE2SDMenuOptions();       }
    else if (SD2EEMode)       { SD2EEMenuOptions();       }
  
}
void menuMGR() { /* Directive to manage menus. Only one menu can be active at any one time */
  if        (MainMenuMode)  { mainMenu();        } 
    else if (SettingsMode)  { settingsMenu();    } 
    else if (setEEAdrMode)  { setEEAdrMenu();    } 
    else if (setEETypeMode) { setEETypeMenu();   }
    else if (setReadMode)   { setReadModeMenu(); }
    else if (setWriteMode)  { setWriteModeMenu();}
    else if (wrSNMode)      { wrSNMenu();        }
    else if (rangeWriteMode){ rangeWriteMenu();  }
    else if (rangeReadMode) { rangeReadMenu();   }
    else if (byteScanMode)  { byteScanMenu();    }
    else if (EraseMode)     { EraseMenu();       }
    else if (saveSetMode)   { saveSetMenu();      }
    else if (EE2SDMode)     { EE2SDMenu();       }
    else if (SD2EEMode)     { SD2EEMenu();       }
}

/*                 MAIN MENU                  */
void mainMenu() {
  Serial.println();
  checkModes();
  Serial.println(F("\tMAIN MENU"));
  Serial.println();
  for(int i=0; i<=4;i++){ /* type int saves 4bytes over type byte... IDKy*/
    doTab(0); 
    if(EEPROMset){  Serial.print(i);Serial.print(F(". "));Serial.print(menu[i]);}
    if(!EEPROMset){ if(i>0){Serial.print(F("°"));} Serial.print(i);Serial.print(F(". "));Serial.print(menu[i]);}
    doTab(2); 
    if(!EEPROMset)            { Serial.print(F("°"));}
    if(sftyMd && EEPROMset)   { Serial.print(F("☺"));}
    if(!sftyMd && EEPROMset){ } Serial.print(i+5);Serial.print(F(". "));Serial.println(menu[i+5]);
  }
  //Serial.println();
  if(EEPROMset && !sftyMd){Serial.println();dashes();Serial.println(F("\t\t\tType E to Erase EEPROM"));dashes();}
  else{prLn(4);}
  Serial.println();
}
byte MainMenuOptions()        {
  if (Serial.available()) {
    switch (Serial.read()) {
/*0.Settings*/    case 48:  SettingsMode = true; mmFMGR();                                    break;
/*1.ListSDFiles*/ case 49: if(!sdReady){errorMessage(3);}else{ listSDfiles(); }               break;
/*2.ByteScan*/    case 50: if (eeMax < 1){errorMessage(2);}else{byteScanMode=true; mmFMGR();} break;
/*3.RangeRead*/   case 51: if (eeMax < 1){errorMessage(2);}else{rangeReadMode=true;mmFMGR();} break;
/*4.ReadEEPROM*/  case 52: if (eeMax < 1){errorMessage(2); }else{readDataMode=true;mmFMGR();} break; 
/*5.EE2SD*/       case 53: if(eeMax<1){errorMessage(2);}else if(!sdReady){errorMessage(3);}
                           else if (sftyMd) {errorMessage(1);}else{EE2SDMode=true; mmFMGR();} break;
/*6.SD2EE*/       case 54: if(eeMax<1){errorMessage(2);}else if(!sdReady){errorMessage(3);}
                           else if(sftyMd){errorMessage(1);}else{SD2EEMode=true; mmFMGR();}   break;
/*7.SaveSetting*/ case 55: if (eeMax < 1) {errorMessage(2);}else
                           if (sftyMd) {errorMessage(1);} else{saveSetMode = true; mmFMGR();} break;
/*8.RangeWrite*/  case 56: if(eeMax < 1){errorMessage(2);} else if(sftyMd){errorMessage(1);}
                           else{rangeWriteMode = true; mmFMGR();} break;
/*9.WriteSN*/     case 57: if(eeMax < 1){errorMessage(2);} else if(sftyMd){errorMessage(1);}
                           else{wrSNMode = true; mmFMGR();}   break;
/*E.ERASEEEPROM*/ case 69: case 101: if(eeMax < 1){errorMessage(2);} else if(sftyMd){errorMessage(1);}  
                           else{EraseMode = true; mmFMGR();} break;
    }
  }
}
byte listSDfiles(){
  if(sdReady){
  prLn(16);Serial.println(F("Files on SD Card:"));
  SD.ls(LS_R);prLn(8);delay(3000); 
  menuMGR();
  return;
  }else{errorMessage(3);} /* SD card not ready */
}
/*                 BYTE SCAN MENU             */
void byteScanMenu() {
  checkModes();
  Serial.println(menu[2]);
  Serial.println();
  //Serial.println(F("\t\tScan the EEPROM for specified data."));
  Serial.println(F("\t\tType a value to scan for (0-255)"));
  prLn(9);
  //Serial.println(F("\t\tNOTE: Set Serial Monitor to No line ending. "));
  //if(sndMd){Serial.println(F("\n\t\t♫ Sound Mode ON will slow the BYTE SCANNER\n\t\t\t(and can be annoying)"));}
  prLn(2);
}
byte byteScanMenuOptions() {
  int scanVal;
  int qty=0;
  byte counter;
  if (Serial.available() > 0) {   
    //while(Serial.available()==0) {          /* Wait for input  */ }
    scanVal=Serial.parseInt();  Serial.setTimeout(3000);
  if(scanVal <0 || scanVal >255){errorMessage(4);}
    else{
      prLn(16);
      Serial.print(F("\t░ SCAN FOR: "));
      Serial.println(scanVal);
      Serial.println();
      delay(2000);
      for (long i = 0; i <= eeMax-1 ; i++) {
        doCounterLine(i, 7);
        if (rdAdr(i) == scanVal) {
          qty++;
          Serial.print(F("\t"));
          print_i(rdAdr(i));
          beepbeep(400,50);
          //delay(25);
        }else {
          Serial.print(F("\t░"));
          }
          counter++;
        }
          prLn(8);
          Serial.print(F("Done\nFound "));
          Serial.print(qty);
          Serial.print(F(" of "));
          Serial.print(eeMax);
          Serial.print(F(" bytes= "));
          Serial.println(scanVal);
          delay(2000);
          byteScanMode=false; mmTMGR();
    } 
  }
}
/*                 RANGE READ MENU            */
/* FUNCTION NOTES
 *  The RangeRead function allows you to read any range of EEPROM data
 *  using (s)tart and (e)nd commands to identify the range.
 *  Example: type s0e511 to read data starting at address 0 and ending at address 511
 */
void rangeReadMenu() {
  checkModes();
  Serial.println(menu[3]);
  Serial.println();
  Serial.println(F("Type a range using the (s)tart & (e)nd commands"));
  Serial.println();
  prLn(8);
}
byte rangeReadMenuOptions()   {
  long START; long END;
  if(Serial.available() >0){
    if(Serial.peek() == 's'|| Serial.peek()=='S'){  /* CAPS or lowercase*/
      Serial.read();
      START =Serial.parseInt(); }
    if(Serial.peek() == 'e'|| Serial.peek()=='E'){  /* CAPS or lowercase*/
      Serial.read();
      END =Serial.parseInt(); 
      rangeRead(START, END);
      } 
      while(Serial.available() >0){ Serial.read();  }  
  }
}
void rangeRead(long START, long END){
  for(int i=START; i <= END; i++){
    doCounterLine(i, 7);
    print_i(rdAdr(i));
    Serial.print(F(" "));
  }
  Serial.println();
  /*Serial.print(F("Finished reading data range "));
  Serial.print(START);
  Serial.print(F(" to "));
  Serial.println(END);
  */
  delay(3000);
  prLn(10);
  rangeReadMode= false;
  mmTMGR();
}
/* READ DATA has no Menu || MenuOptions */
void readData() {
  int i;
  int block = 0;
  int freeBlocks = 0;
  for (i = 0; i <= eeMax-1; i++) {
    block = block + 1;
    doCounterLine(i, 7);
    print_i(rdAdr(i));
    Serial.print(F(" "));
    //beepbeep(400,25);
  }
  Serial.println();
  //msgMenu("Read EEPROM Complete", "\t\t" + (String)block + " bytes read.",2, 8000);
  Serial.println(F("Finished."));
  Serial.print(block);
  Serial.println(F(" bytes read"));
  readDataMode = false; mmTMGR(); 
}
/*                 EE2SD MENU                 */
void EE2SDMenu() {
  checkModes();
  Serial.println(menu[5]);
  Serial.println();
  Serial.println(F("\t\tType filename to save to SD Card"));
  Serial.println(F("\t\t(Existing filename will be overwritten)"));
  //Serial.println("\t\tType filename to save to SD Card");
  //Serial.println("\t\t(Existing filename will be overwritten)");
  
  prLn(10);
}
void EE2SDMenuOptions() {
  if (Serial.available()>0) {
    String FILENAME = Serial.readString();
    EEPROM2SD(FILENAME);
    EE2SDMode = false; mmTMGR();
  } 
}
void EEPROM2SD(String FILENAME){
  if(sdReady){
    char buf[12];
    FILENAME.toCharArray(buf,12);
    SD.remove(buf);
    File myFile = SD.open(FILENAME, FILE_WRITE);
    int counter=0;
    if (myFile) {
    Serial.print(F("Writing EEPROM data to SD file: "));
    Serial.print(FILENAME);
    for(long i=0; i <=eeMax-1; i++){
      if(counter>7){myFile.print(F("\n")); counter=0;}
      if(wrMode==65){ myFile.print(char(rdAdr(i))); myFile.print(",");counter++; }else{
      if(wrMode == 66){myFile.print(rdAdr(i),BIN); myFile.print(",");counter++; }else{
      if(wrMode == 68){myFile.print(rdAdr(i),DEC);myFile.print(",");counter++; }else{
      if(wrMode == 72){myFile.print(rdAdr(i),HEX);myFile.print(",");counter++; }else{
      if(wrMode == 79){myFile.print(rdAdr(i),OCT);myFile.print(",");counter++; }}
    }}}}
  } else {
    errorMessage(5); /* file didn't open */
  }
  // we're done close the file:
    myFile.close();
    Serial.println();
    Serial.println(F("done."));
  }else{ prLn(16);Serial.println(F("Check SD CARD")); delay(2400);menuMGR();}
}
/*                 SD2EE MENU                 */
void SD2EEMenu() {
  checkModes();
  Serial.println(menu[6]);
  Serial.println();
  Serial.println(F("\t\tType SD file to write to EEPROM"));
  Serial.println(F("SD files:"));
  SD.ls(LS_R);
  prLn(10);
}
void SD2EEMenuOptions() {
  if (Serial.available()>0) {
    String FILENAME = Serial.readString();
    SD2EEPROM(FILENAME);
    SD2EEMode = false; mmTMGR();
  }
}
void SD2EEPROM(String FILENAME) {
   if(sdReady){ /*open the file */
  File  file = SD.open(FILENAME, FILE_READ);
  long counter=0;
  if (!file) {
    errorMessage(5); /* open fail */
    return;
  }
  file.seek(0); /* rewind file & read */
  int16_t tcalc; 
  while (file.available()) {
    if (csvReadUint16(&file, &tcalc, CSV_DELIM) != CSV_DELIM)
      {
      Serial.println(F("read error"));
      int ch;
      int nr = 0;
      // print part of file after error.
      while ((ch = file.read()) > 0 && nr++ < 4) {
        Serial.write(ch);
      }
      break;            
    }
    wrAdr(counter, tcalc);
    /*Uncomment for output to Serial Monitor */
    //Serial.print(F("Writing Address ")); 
    //Serial.print(counter);
    //Serial.print(F(": "));
    //Serial.print(tcalc);
    //Serial.print(F("\n"));
    counter++;
  }
  file.close();
  Serial.println();
  Serial.println(F("Done"));
}else{errorMessage(3);} /* SD card not ready */
}
/*                 SAVE SETTINGS MENU */
void saveSetMenu() {
  checkModes();
  Serial.println(F("\tSAVE SETTINGS"));
  Serial.println();
  Serial.println(F("\t\tThe current settings will be saved to the EEPROM."));
  Serial.println(F("\t\tBy default, the save settings are allocated to EEPROM Memory Address 1-4."));
  Serial.println(F("\t\tType S to save settings now or X to exit"));
  prLn(2);
}
byte saveSetMenuOptions(){
  if (Serial.available()) {
    switch (Serial.read()) {
      case 83: case 115: 
                wrAdr(1,1);             // AUTODETECT
                if(sftyMd){wrAdr(2,1);} // SAFEMODE
                else{wrAdr(2,0);}
                wrAdr(3,rdMode);        // READMODE
                wrAdr(4,wrMode);        // WRITEMODE
                Serial.println(F("Settings Saved"));
                delay(2000);
                saveSetMode = false; mmTMGR(); break;
      /* X or x Exit & go back to Settings Menu */
      case 88: case 120: saveSetMode = false; mmTMGR(); break;
    }
  }
  
}
/*                 RANGE WRITE MENU           */
/* FUNCTION NOTES
 *  The RangeWrite function allows you to write data to any range of EEPROM adresses
 *  using (s)tart (e)nd and (d)ata commands to identify the range & data to write.
 *  Example: type s0e511d255 to write data 255 starting at address 0 and ending at address 511
 */
void rangeWriteMenu() {
  checkModes();
  Serial.println(menu[8]);
  Serial.println();
  Serial.println(F("Type a range using the (s)tart (e)nd & (d)ata commands"));
  Serial.println();
  prLn(3);
}
byte rangeWriteMenuOptions()  {
  long START; long END; long DATA;
  if(Serial.available() >0){
    if(Serial.peek() == 's'|| Serial.peek()=='S'){ 
      Serial.read();  
      START =Serial.parseInt();  
      }
    if(Serial.peek() == 'e'|| Serial.peek()=='E'){  
      Serial.read();
      END   =Serial.parseInt(); 
      }
    if(Serial.peek() == 'd'|| Serial.peek()=='D'){  
      Serial.read();
      DATA =Serial.parseInt(); 
      rangeWrite(START, END, DATA);                                       
      }
      while(Serial.available() >0){ Serial.read();  }  
   }
}
void rangeWrite(long START, long END, byte DATA){
  int wByte=0;
  int uByte=0;
  for(long i=START; i <= END; i++){
    if(rdAdr(i) == DATA){uByte++;} else{wrAdr(i, DATA); wByte++; }  
  }
  Serial.print(F("Finished writing data "));
  Serial.print(DATA); 
  Serial.print(F(" to EEPROM range "));
  Serial.print(START);
  Serial.print(F(" to "));
  Serial.println(END);
  Serial.print(F("Of "));Serial.print((END-START)+1);Serial.print(F(" bytes total, "));
  Serial.print(wByte);
  Serial.print(F(" bytes overwritten & "));
  Serial.print(uByte);
  Serial.println(F(" bytes unchanged."));
  Serial.println();
  rangeRead(START, END);
  delay(3000);
  rangeWriteMode= false;
  mmTMGR();
}
/*                 WRITE SN MENU              
   * By default, the Serial Number is written to the last 16 byte locations of the EEPROM
   * Type a 16 BYTE Serial Number into the Serial Monitor
   * EXAMPLE: 24LC64-1419x0001
   * this 16-Byte format example decodes as '24LC64 week 14 of 2019 x= erased 0001= id number'
   */
void wrSNMenu() {
  checkModes();
  Serial.println(menu[9]);
  Serial.println();
  Serial.println(F("Type a 16 BYTE Serial Number"));  
}
void wrSNMenuOptions() {
  if (Serial.available()>0) {
    byte   len =16;
    char  snBuf[len+1];
    byte  sn = Serial.readBytes(snBuf, len+1);
    int counter=0; /* must be int */
    String snTxt;
    Serial.print(F("\t\tWriting Serial Number: "));
    for (long i = (eeMax - len); i <= eeMax-1; i++) {
      if(rdAdr(i) == snBuf[counter]){/*do nothing*/}else{funBeep3(1);Serial.print(snBuf[counter]);wrAdr(i, snBuf[counter]);}
      snTxt+=snBuf[counter];
      counter ++;
    }
    Serial.println();
    Serial.print(F("Wrote SN:"));
    Serial.println(snTxt);
    wrSNMode = false; mmTMGR();
    }
}
/*                 ERASE EEPROM MENU          */
void EraseMenu() {
  checkModes();
  Serial.println(F("\tERASE EEPROM"));
  
  Serial.print(F("\tYou are about to Erase the EEPROM "));
  exitOption(4);
  Serial.println();
  prLn(3);
}
byte EraseModeMenuOptions() {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 69: case 101:  EraseEEprom();  EraseMode = false; mmTMGR();  break; /* E or e ERASE */
      case 67: case 99: EraseMode = false; mmTMGR();  break; /* C or c to Cancel */
    }
  }
}
void EraseEEprom(){
  Serial.println(F("ERASE EEPROM"));
    byte counter = 15;
    for (long i = 0; i <= eeMax-1; i++) {
      counter = counter + 1;
      if (counter > 15) {
        Serial.println();
        Serial.print(i);
        Serial.print(F(" \t►"));
        Serial.print(F("\t"));
        counter = 0;
      }
      if(rdAdr(i)>0){ wrAdr(i, 0);}
      Serial.print(F(" 0 "));
    }
    Serial.println();
    Serial.print(F("Done"));
}

/*                            SETTINGS MENU              */
void settingsMenu() {
  checkModes();
  doTab(0);
  Serial.println(menu[0]);  Serial.println();
  exitTitle();
  exitOption(1);
  Serial.println();
  Serial.print(F("\t1. AUTODETECT EEPROM "));
  Serial.println(F("\t\t5. SET READMODE"));
  
  Serial.print(F("\t2. SET EEPROM TYPE"));
  Serial.println(F("\t\t6. SET WRITEMODE"));

  Serial.print(F("\t3. SET EEPROM ADDRESS"));
  Serial.print(F("\t\t7. TURN SAFEMODE "));
  if (sftyMd)  { boolOption(0); }
  if (!sftyMd) { boolOption(1); }
  Serial.println();
  Serial.println(F("\t4. INIT SD CARD"));
  prLn(6);
}
byte SettingsMenuOptions()    {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 55:  switchSafetyMode();     menuMGR();  break;
      case 49:  EprAutoDet(autoDt); menuMGR();  break;
      case 52:  initSD();               menuMGR();  break;
      case 53:  SettingsMode = false; setReadMode = true;   menuMGR();  break;
      case 54:  SettingsMode = false; setWriteMode = true;  menuMGR();  break;
      case 50:  SettingsMode = false; setEETypeMode = true; menuMGR();  break;
      case 51:  SettingsMode = false; setEEAdrMode = true;  menuMGR();  break;
      case 88: case 120: SettingsMode = false; mmTMGR();  break; /* X or x Exit */
    }
  }
}
byte EprAutoDet(byte val){ /*AutoDetect EEprom has no seperate menu */
  long adr=128;
  if(rdAdr(0) != val){wrAdr(0, val); }
  for(int i=1; i <= 12; i++){
    if(rdAdr(adr) == val){
      setEprType((adr/128),adr);
      prLn(20);dashes();
      Serial.print(F("\t\t\tAutoDetect found "));
      Serial.print(adr/128);
      Serial.print(stuff[5]);
      Serial.println(F(" EEPROM"));
      dashes();prLn(8);
      catCall();
      delay(2000);
      return;
      } 
      adr=adr*2; 
  }
      prLn(20);dashes();
      Serial.print(F("\t\tEEPROM @ 0x"));
      Serial.print(eeAdr, HEX);
      Serial.println(F(" not found. Check wiring."));
      dashes();prLn(8);
      delay(2000);
}
/*                 SET EEPROM TYPE MENU       */
/* 1Kbit, 2Kbit, 4Kbit, 8Kbit, 16Kbit, 32Kbit, 64Kbit, 128Kbit, 256Kbit, 512Kbit, 1024Kbit, 2048Kbit*/
const int kbits[]={0,1,2,4,8,16,32,64,128,256,512,1024,2048};
void setEETypeMenu() {
  checkModes();
  Serial.println(F("\tSet EprType"));
  exitTitle();
  exitOption(1);
  Serial.println();
  for(int i=1;i<=4;i++){
    Serial.print(F("\t"));
    Serial.print(i);
    Serial.print(F(". "));
    Serial.print(kbits[i]);
    Serial.print(stuff[5]);
    doTab(1);
    Serial.print(i+4);
    Serial.print(F(". "));
    Serial.print(kbits[i+4]);
    Serial.print(stuff[5]);
    doTab(1);
    if(i+8>9){Serial.print(char (63+i));}else{Serial.print(i+8);}
    Serial.print(F(". "));
    Serial.print(kbits[i+8]);
    Serial.println(stuff[5]);
  }
  prLn(7);
}
byte setEETypeMenuOptions()   {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  setEprType(1,128);         break;
      case 50:  setEprType(2,256);         break;
      case 51:  setEprType(4,512);         break;
      case 52:  setEprType(8,1024);        break;
      case 53:  setEprType(16,2048);       break;
      case 54:  setEprType(32,4096);       break;
      case 55:  setEprType(64,8192);       break;
      case 56:  setEprType(128,16384);     break;
      case 57:  setEprType(256,32768);     break;
      case 65: case 97: setEprType(512, 65536);    break;
      case 66: case 98: setEprType(1024, 131072);  break;
      case 67: case 99: setEprType(2048, 262144);  break;
      
      /* X or x Exit & go back to Settings Menu */
      case 88: case 120: setEETypeMode = false; smTMGR();  break;
    }
  }
}
void setEprType(int TYPE, long MAX){
  eeType = TYPE; eeMax = MAX;  
  EEPROMset = true; setEETypeMode = false; smTMGR();
}
/*                 SET EEPROM ADDRESS MENU    */
void setEEAdrMenu() {
  checkModes();
  Serial.println(F("\tSet EprAddress"));
  exitTitle();
  exitOption(1);
  Serial.println();
  byte xAdr[] = {50, 51, 52, 53, 54, 55, 56, 57};
  int i;
  for (i = 0; i <= 3; i++) {
    doTab(1); Serial.print(i); Serial.print(F(". 0x")); Serial.print(xAdr[i]);
    doTab(2); Serial.print(i+4); Serial.print(F(". 0x")); Serial.println(xAdr[i+4]);
  }
  prLn(8);
}
byte setEEAdrMenuOptions()    {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 48:  eeAdr = 0x50; eeClear();  break;
      case 49:  eeAdr = 0x51; eeClear();  break;
      case 50:  eeAdr = 0x52; eeClear();  break;
      case 51:  eeAdr = 0x53; eeClear();  break;
      case 52:  eeAdr = 0x54; eeClear();  break;
      case 53:  eeAdr = 0x55; eeClear();  break;
      case 54:  eeAdr = 0x56; eeClear();  break;
      case 55:  eeAdr = 0x57; eeClear();  break;
      /* X or x Exit & go back to Settings Menu */
      case 88: case 120: setEEAdrMode = false; smTMGR(); break;
    }
  }
}
void eeClear(){
  eeType=0; eeMax=0; EEPROMset= false; setEEAdrMode = false; smTMGR();
  funBeep2(1);
  loadSettings();
}
void initSD(){
  if (!SD.begin(SD_CS_PIN)) {
    sdReady = false;
    return;
  }
  sdReady = true;
}
/*                 SET READ MODE MENU          */
void setReadModeMenu() {
  checkModes();
  Serial.println(F("\tSet ReadMode"));
  exitTitle();
  exitOption(2);
  Serial.println();
  for (int i = 0; i <= 2; i++) {
    Serial.print(F("\t"));
    Serial.print((i+1));
    Serial.print(F(". "));
    Serial.print(stuff[i] );

    if((i+4)>5){Serial.println();}
    else{
      Serial.print(F("\t\t"));
    Serial.print((i+4));
    Serial.print(F(". "));
    Serial.println(stuff[i+3] );
  }}
  prLn(8);
}
byte setReadModeMenuOptions() {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  rdMode = 65;  setReadMode = false; smTMGR();  break;
      case 50:  rdMode = 66;  setReadMode = false; smTMGR();  break;
      case 51:  rdMode = 68;  setReadMode = false; smTMGR();  break;
      case 52:  rdMode = 72;  setReadMode = false; smTMGR();  break;
      case 53:  rdMode = 79;  setReadMode = false; smTMGR();  break;
      /* c or C Cancel & go back to Settings Menu */
      case 67: case 99: setReadMode = false; smTMGR();  break;
      
    }
  }
}
/*                 SET WRITE MODE MENU         */
void setWriteModeMenu() {
  checkModes();
  Serial.println(F("\tSet WriteMode"));
  exitTitle();
  exitOption(2);
  Serial.println();
  for (int i = 0; i <= 2; i++) {
    Serial.print(F("\t"));
    Serial.print((i+1));
    Serial.print(F(". "));
    Serial.print(stuff[i] );

    if((i+4)>5){Serial.println();}
    else{
      doTab(1);
    Serial.print((i+4));
    Serial.print(F(". "));
    Serial.println(stuff[i+3] );
  }}
  prLn(8);
}
byte setWriteModeMenuOptions() {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  wrMode = 65;  setWriteMode = false; smTMGR();  break;
      case 50:  wrMode = 66;  setWriteMode = false; smTMGR();  break;
      case 51:  wrMode = 68;  setWriteMode = false; smTMGR();  break;
      case 52:  wrMode = 72;  setWriteMode = false; smTMGR();  break;
      case 53:  wrMode = 79;  setWriteMode = false; smTMGR();  break;
      /* c or C Cancel & go back to Settings Menu */
      case 67: case 99: setWriteMode = false; smTMGR();  break;
    }
  }
}

/*◙◙◙◙◙◙◙◙                 ERRORS, CHECKS & SWITCHES      */
void errorMessage(byte val){
  prLn(16); dashes();
  if(val == 1){Serial.println(F("\t\t\tError: SAFEMODE is ON"));}
  if(val == 2){Serial.println(F("\t\t\tError: EEpromType not set"));}
  if(val == 3){Serial.println(F("\t\t\tError: SD Card Not Ready"));}
  if(val == 4){Serial.println(F("\t\t\tError: Must enter a valid number (0-255)"));}
  if(val == 5){Serial.println(F("\t\t\tError: Couldn't open file"));}
  if(val == 6){Serial.println(F("\t\t\tNo I²C devices found"));}
  dashes(); prLn(8);
  beepbeep(75,100);
  delay(3000);  menuMGR();
}
void checkEEPROMAddress(){
  //Serial.print(F("EEpromAddress set to: 0x"));
  Serial.println(eeAdr, HEX);
}
void checkEEPROMType(){
  if (eeType < 1) {
    Serial.print(F("°NOT SET"));
  } else {
    Serial.print(eeType);
    Serial.print(stuff[5]);
    Serial.print(F(" ("));
    Serial.print(eeMax);
    Serial.print(F(" bytes)"));
  }
}
void getReadMode(){
  if(rdMode==65){Serial.print(F("ASC"));}
  if(rdMode==66){Serial.print(F("BIN"));}
  if(rdMode==68){Serial.print(F("DEC"));}
  if(rdMode==72){Serial.print(F("HEX"));}
  if(rdMode==79){Serial.print(F("OCT"));}
}
void getWriteMode(){
  if(wrMode==65){Serial.print(F("ASC"));}
  if(wrMode==66){Serial.print(F("BIN"));}
  if(wrMode==68){Serial.print(F("DEC"));}
  if(wrMode==72){Serial.print(F("HEX"));}
  if(wrMode==79){Serial.print(F("OCT"));}
}
void rdSN(){
  String sn;
  for (word i = 0; i < 16; i++) {
     sn += (char) rdAdr(i + (eeMax - 16));
  }
  Serial.print(F("SN: "));
  Serial.print(sn);
}
void checkModes(){
  initSD();
  //Serial.println("\t\t\tMITZ I²C™ EEPROM BYTEBANGER v0.75");
  Serial.println(F("\t\t\tMITZ I²C™ EEPROM BYTEBANGER v0.75"));
  //Serial.println(F("   ╔══════════════════════╗"));
  Serial.println(F("\tEEPROM"));
  Serial.print(F("\tADDRESS: 0x"));
  Serial.print(eeAdr, HEX);
  Serial.print(F("\tTYPE: "));  checkEEPROMType();
  Serial.print(F("\t"));  rdSN();
  Serial.println();
  if (!sftyMd) { Serial.print(F("\tSAFEMODE: OFF"));} 
  else { Serial.print(F("\tSAFEMODE:ON ☺"));}
  Serial.print(F("\tREADMODE: ")); getReadMode();
  Serial.print(F("\tWRITEMODE: ")); getWriteMode();
  Serial.print(F("\tSD CARD: "));
  if(sdReady){Serial.println(F("READY"));}else{Serial.println(F("NOT READY"));}
  //Serial.println(F("   ╚══════════════════════╝"));
  prLn(2);
}
byte switchSafetyMode(){
  if (sftyMd) { sftyMd = false;  return sftyMd;} 
  else        { sftyMd = true;   return sftyMd;}
}

/*◙◙◙◙◙◙◙◙                 MENU HELPERS      */
void prLn(int val) {
  for (int i = 1; i < val; i++) { Serial.println(); }
}
void doTab(byte val) {
  for (byte i = 0; i <= val; i++) { Serial.print(F("\t")); }
}
void dashes(){
   byte count=0;
    Serial.print(F("    "));
    for(byte i=0;i<64;i++){
      if(count>7){Serial.print(F(" ")); count=0;}
      Serial.print(F(""));
      count++;
    }
    Serial.println();
}
void exitTitle(){
  //Serial.print(F("\tChoose an option below or Type "));
  Serial.print("\tChoose an option below or Type ");
}
void exitOption(int val) {
  if (val == 1) {Serial.println(F("X to Exit"));}
  if (val == 2) {Serial.println(F("C to Cancel"));}
  if (val == 3) {Serial.println(F("W to Write or ")); exitOption(2);}
  if (val == 4) {Serial.println(F("E to ERASE or ")); exitOption(2);}
}
void boolOption(byte val){
  if(val ==0){Serial.print(F("OFF"));}
  if(val ==1){Serial.print(F("ON")); }
}

/*◙◙◙◙◙◙◙◙                 FUNCTION HELPERS   */
byte print_i(byte i){
  if(rdMode==65){ Serial.print(char(i)); return i;}else
  if(rdMode==66){ Serial.print(i, BIN); return i;}else
  if(rdMode==68){ Serial.print(i, DEC); return i;}else
  if(rdMode==72){ if(i<10){Serial.print(F("0x0"));}else{Serial.print(F("0x"));}Serial.print(i, HEX); return i;}else
  if(rdMode==79){ Serial.print(i, OCT); return i;}
}
void doCounterLine(long i, byte line){
  counter2 = counter2 + 1;
    if (counter2 > line) {
      Serial.println();
      Serial.print(i);
      Serial.print(F(" \t:"));
      Serial.print(F(" "));
      counter2 = 0;
      }
}
int csvReadText(File* file, char* str, size_t size, char delim) {
  char ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1) {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR.
    if (ch == '\r'|| ch == '\n') {
      continue;
    }
    if (ch == delim /*|| ch == '\n'*/) {
      rtn = ch;
      break;
    }
    if ((n + 1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}
int csvReadUint16(File* file, uint16_t* num, char delim) {
  uint32_t tmp;
  int rtn = csvReadUint32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp > UINT_MAX) return -5;
  *num = tmp;
  return rtn;
}
int csvReadUint32(File* file, uint32_t* num, char delim) {
  char buf[10];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtoul(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
/*MGR Helpers*/
void mmFMGR(){  MainMenuMode = false; menuMGR();}
void mmTMGR(){  MainMenuMode = true; menuMGR(); }
void smTMGR(){  SettingsMode = true; menuMGR(); }

/*◙◙◙◙◙◙◙◙                 SOUNDS             */
/*void beep(int KHz, int td) {
  tone(spkr, KHz);  delay(td);
  noTone(spkr); 
}*/
void beepbeep(int KHz, int td) {
  tone(spkr, KHz);  delay(td);
  noTone(spkr);     delay(td);
  tone(spkr, KHz);  delay(td);
  noTone(spkr);
}
void funBeep(byte td) {
  for (int i = 500; i < 2500; i = i + 5) {
  tone(spkr, i);    delay(td);
  }
  noTone(spkr);
}
void funBeep2(byte td) {
  for (int i = 2500; i > 500; i = i - 5) {
    tone(spkr, i);   delay(td);
  }
  noTone(spkr);
}
void funBeep3(byte td) {
  for (int i = 500; i > 100; i = i - 5) {
    tone(spkr, i);    delay(td);
  }
  noTone(spkr);
}
void catCall(){
  funBeep(1);
  funBeep(1);
  funBeep2(1);
}

byte loadSettings(){
  if(rdAdr(1)==1){EprAutoDet(autoDt);}
  if(rdAdr(2)==0){sftyMd = false;} else{sftyMd = true;}
  if(rdAdr(3)== 65 || rdAdr(3)==66 || rdAdr(3)==68 || rdAdr(3)==72 || rdAdr(3)==79){rdMode=rdAdr(3);}
  if(rdAdr(4)== 65 || rdAdr(4)==66 || rdAdr(4)==68 || rdAdr(4)==72 || rdAdr(4)==79){wrMode=rdAdr(4);}
  menuMGR();
}
void wrAdr(long adr, byte val){
  Wire.beginTransmission(eeAdr);  /* CONTROL BYTE*/
  Wire.write((adr >> 8));         /* HIGH BYTE*/
  Wire.write((adr & 0xFF));       /* LOW BYTE*/
  Wire.write(val);                /* DATA VALUE */
  Wire.endTransmission(); 
  delay(5);
}
byte rdAdr(long adr){
  byte rData = 0xFF;
  Wire.beginTransmission(eeAdr);  /* CONTROL BYTE*/
  Wire.write((adr >> 8));         /*HIGH BYTE*/
  Wire.write((adr & 0xFF));       /*LOW BYTE*/
  Wire.endTransmission();
  Wire.requestFrom(eeAdr, 1);
  rData =  Wire.read();
  return rData;
}
