/*  I²C™ Bit-BANGER v0.01
     for Microchip® 64K I²C™ 24LC64 Serial EEPROM
     by Mitzpatrick Fitzsimmons
     CopyLeft April 19, 2019 All rights reserved.
     This code may be freely distributed for non-commercial use.
*/

/* The 24LC64 Serial EEPROM is a 2-wire Serial interface bus I²C™ device.
    This code utilizes the Wire.h library to communicate to the device
    using an Arduino (or compatible) micro-controller.
*/
#include "Wire.h"
//#include <SPI.h>
//#include <SD.h>

/* The 24LC64 Serial EEPROM I²C™ Address must be defined for any communication to
    the device to work properly. The 24LC64 Serial EEPROM address can be set using
    the first 3 pins A0, A1, A2 on the 24LC64 Serial EEPROM device. (see Datasheet)

    Up to 8 address locations can be set via the A0 A1 A2 pins, which means that up to
    8 24LC64 Serial EEPROMS can be cascaded on the same 2-wire I²C™ interfaced Serial bus.
    The HEX address location we define for our code will determine which 24LC64 Serial EEPROM
    device we will communicate to on the I²C™ Serial bus. (default is 0x50)
*/
//#define eeAdr 0x50

/* The 24LC64 Serial EEPROM is organized as eight blocks of 1K x 8-bit memory
    ie. 8 x 1024 bytes = 8192 bytes (65,536 bits)
    This will be defined as a variable eeMax used in this code
*/
//#define eeMax 8192

/* WRITE PROTECTION & safetyMode
    The 24LC64 Serial EEPROM has Write Protection capability via pin7 (see Datasheet for other models).
    To enable Write Protection, the WP pin is tied HIGH to Vcc. It can be tied LOW to Vss (ground) or
    left floating to disable Write Protection.

    As an added protection, this code has an optional backup "safetyMode" built-in to help prevent
    accidental overwrite data-loss in case the user decides not to use the WP pin of the 24LC64.

    safetyMode is OFF by default and can be turned on by changing the safetyMode variable value to true.
*/

//#define ctsPin D0 
//const int ledPin= D5;
//const int led2Pin= D6;
//const int buttonPin = D7;    // the number of the pushbutton pin

int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers


bool runlogo        = false;     /* false = INTRO OFF || true = INTRO ON */
bool sftyMd         = false;     /* false = SAFETY MODE OFF || true = SAFETY MODE ON */
bool EEPROMset      = false;     /* false = EEPROM not set || true = EEPROM set */
bool ledMd          = true;      /* false = LED MODE OFF || set true = LED MODE ON */
bool sndMd          = true;      /* false = SOUND MODE OFF || set true = SOUND MODE */
bool MainMenuMode   = true;      /* set true by default */
bool SettingsMode   = false;     /* set to false by default */
bool readDataMode   = false;     /* set to false by default */
bool setEEAdrMode   = false;     /* set to false by default */
bool setEETypeMode  = false;     /* set to false by default */
bool setReadMode    = false;     /* set to false by default */
bool wrSNMode       = false;     /* set to false by default */
bool rangeWriteMode = false;     /* set to false by default */
bool rangeReadMode  = false;     /* set to false by default */
bool byteScanMode   = false;     /* set to false by default */
bool EraseMode      = false;     /* set to false by default */
bool saveSetMode    = false;     /* set to false by default */
const PROGMEM String stuff[]  ={"ASCII", "BINARY", "DECIMAL", "HEXIDECIMAL", "OCTAL"};
const PROGMEM String menu[]   ={"SETTINGS", "00000000", "BYTE SCANNER", "RANGE READ", "READ EEPROM", "ERASE EEPROM", "11111111", "SAVE SETTINGS", "RANGE WRITE", "WRITE SERIAL NUMBER"};
//const PROGMEM String box[5]    ={"╔", "═", "╗", "╚", "╝"};


byte spkr = 3;             /* SPEAKER PIN */
byte eeAdr = 0x50;            /* EEPROM ADDRESS */
int eeType;                      /* EEPROM TYPE */
long eeMax;                      /* EEPROM MAXIMUM BYTES */
int startByte;                   /* ??? */
int endByte;                     /* ??? */
byte dataByte;                   /* ??? */
byte readMode=68;                /* ASCII=65 BIN=66 DEC=68 HEX =72 OCT 79 */
byte counter2=7;
int UAB;
int ctsCount=0;
int BeatBytes[8];

void setup()
{
  /* Speaker & LED Setup */    
  //pinMode(6, OUTPUT);       // Yellow LED
  //pinMode(7, OUTPUT);       // Green LED
  //pinMode(spkr, OUTPUT);    // Speaker
  
  //pinMode(ledPin, OUTPUT);
  //pinMode(led2Pin, OUTPUT);
  //pinMode(ctsPin, INPUT);
  //pinMode(buttonPin, INPUT);
  //digitalWrite(led2Pin, ledState);
  Wire.begin();
  Serial.begin(115200);
  randomSeed(analogRead(0));
  
  //BitBanger(128);
  //wrSN();
  //getBlocks(8176, 8192);
  //intro();
  //saveSettings();
  loadSettings();
  menuMGR();
  //initSDCard();
  //coolPrint();
  //getBlocks(0, 8191);
  //someOtherFunction();
}

void loop()
{
  //ctSensor();
  //btnPress();
  modeMGR();
  //delay(2000);
}
/*
void ctSensor(){
  int ctsVal = digitalRead(ctsPin);
  if(ctsVal == HIGH){ctsCount++;digitalWrite(ledPin, HIGH); Serial.println("Touched "+(String)ctsCount);rdAdrX(ctsCount);}
  else{digitalWrite(ledPin, LOW);} 
  delay(350);
}
*/
void rdAdrX(int val){
  Serial.print(F("Address "));
  Serial.print(val-1);
  Serial.print(F(": "));
  Serial.println(rdAdr(val-1));
}
void modeMGR() { /* Directive to manage modes. Only one mode can be active at any one time */

  if        (MainMenuMode)    { MainMenuOptions();        } 
    else if (SettingsMode)    { SettingsMenuOptions();    } 
    else if (readDataMode)    { readData();/*ReadDataMenuOptions(); */   } 
    else if (setEEAdrMode)    { setEEAdrMenuOptions();    } 
    else if (setEETypeMode)   { setEETypeMenuOptions();   } 
    else if (wrSNMode)        { setwrSNMenuOptions();     }
    else if (rangeWriteMode)  { rangeWriteMenuOptions();  }
    else if (rangeReadMode)   { rangeReadMenuOptions();   }
    else if (byteScanMode)    { NewbyteScanMenuOptions(); }
    else if (setReadMode)     { setReadModeMenuOptions(); }
    else if (EraseMode)       { EraseModeMenuOptions();   }
    else if (saveSetMode)     { saveSetMenuOptions();     }
  
}

void menuMGR() { /* Directive to manage menus. Only one menu can be active at any one time */
delay(1000);
  if        (MainMenuMode)  { mainMenu();         } 
    else if (SettingsMode)  { settingsMenu();     } 
    else if (readDataMode)  { readDataMenu();     } 
    else if (setEEAdrMode)  { setEEAdrMenu();     } 
    else if (setEETypeMode) { setEETypeMenu();    }
    else if (setReadMode)   { setReadModeMenu();  }
    else if (wrSNMode)      { wrSNMenu();         }
    else if (rangeWriteMode){ rangeWriteMenu();   }
    else if (rangeReadMode) { rangeReadMenu();    }
    else if (byteScanMode)  { byteScanMenu();     }
    else if (EraseMode)     { EraseMenu();        }
    else if (saveSetMode)   { saveSetMenu();      }
}

byte MainMenuOptions()        {
  delay(1000);
  if (Serial.available()) {
    switch (Serial.read()) {
/*0*/ case 48:  SettingsMode = true; MainMenuMode = false; menuMGR(); break;
/*1*/ case 49:  // 1 ???  break;
/*2*/ case 50:  // 2 Byte Scanner
          if (eeMax < 1) { eeTypeError();}else{byteScanMode = true; MainMenuMode = false; menuMGR();}
          break;
/*3*/ case 51:  // 3 Range Read
          if (eeMax < 1) { eeTypeError();}else{rangeReadMode = true; MainMenuMode = false; menuMGR();}
          break;
/*4*/ case 52:  // 4 read EEPROM
          if (eeMax < 1) { eeTypeError(); }else{readDataMode = true; MainMenuMode = false; menuMGR();}
          break; 
/*5*/ case 53:  // 5 ERASE EEPROM (zero-eeprom)
        if (eeMax < 1) {eeTypeError();}else
        if (sftyMd) {safetyModeError();} else{EraseMode = true; MainMenuMode = false; menuMGR();}
          break;
/*6*/ case 54:  // 6 ???
        //SetEEAdr();
        break;
/*7*/ case 55:  // 7 Save Settings
        if (eeMax < 1) {eeTypeError();}else
        if (sftyMd) {safetyModeError();} else{saveSetMode = true; MainMenuMode = false; menuMGR();}
          break;
/*8*/ case 56:  // 8 Range Write
        if (eeMax < 1) {eeTypeError();} else 
        if (sftyMd) {safetyModeError();}else{rangeWriteMode = true; MainMenuMode = false; menuMGR();}
        break;
/*9*/ case 57:  // 9 Write Serial Number
        if (eeMax < 1) {eeTypeError();} else 
        if (sftyMd) {safetyModeError();}else{wrSNMode = true; MainMenuMode = false; menuMGR();}
        break;
    }
  }
}
byte SettingsMenuOptions()    {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  switchSafetyMode();                         menuMGR();  break;
      case 50:  switchSoundMode();                          menuMGR();  break;
      case 51:  switchLEDMode();                            menuMGR();  break;
      case 52:  SettingsMode = false; setReadMode = true;   menuMGR();  break;
      case 53:  SettingsMode = false; setEETypeMode = true; menuMGR();  break;
      case 54:  SettingsMode = false; setEEAdrMode = true;  menuMGR();  break;
      /* X or x Exit & go back to Main Menu */
      case 88:  SettingsMode = false; MainMenuMode = true;  menuMGR();  break;
      case 120: SettingsMode = false; MainMenuMode = true;  menuMGR();  break;
    }
  }
}

/*
byte ReadDataMenuOptions()    {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  readData(); menuMGR();  break; 
      case 50:  readData(); menuMGR();  break; 
      case 51:  readData(); menuMGR();  break; 
      case 52:  readData(); menuMGR();  break; 
      case 53:  readData(); menuMGR();  break; 
      case 67:  readDataMode = false; MainMenuMode = true; menuMGR(); break;
      case 99:  readDataMode = false; MainMenuMode = true; menuMGR(); break;    
    }
  }
}
*/

byte setEEAdrMenuOptions()    {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 48:  eeAdr = 0x50; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 49:  eeAdr = 0x51; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 50:  eeAdr = 0x52; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 51:  eeAdr = 0x53; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 52:  eeAdr = 0x54; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 53:  eeAdr = 0x55; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 54:  eeAdr = 0x56; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      case 55:  eeAdr = 0x57; setEEAdrMode = false; SettingsMode = true; menuMGR();  break;
      /* X or x Exit & go back to Settings Menu */
      case 88:  setEEAdrMode = false; SettingsMode = true; menuMGR(); break;
      case 120: setEEAdrMode = false; SettingsMode = true; menuMGR(); break;
    }
  }
}

byte setReadModeMenuOptions() {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  readMode = 65;  setReadMode = false; SettingsMode = true; menuMGR();  break;
      case 50:  readMode = 66;  setReadMode = false; SettingsMode = true; menuMGR();  break;
      case 51:  readMode = 68;  setReadMode = false; SettingsMode = true; menuMGR();  break;
      case 52:  readMode = 72;  setReadMode = false; SettingsMode = true; menuMGR();  break;
      case 53:  readMode = 79;  setReadMode = false; SettingsMode = true; menuMGR();  break;
      /* c or C Cancel & go back to Settings Menu */
      case 67: setReadMode = false; SettingsMode = true; menuMGR();  break;
      case 99: setReadMode = false; SettingsMode = true; menuMGR();  break;
    }
  }
}

byte rangeReadMenuOptions()   {
  
  if(Serial.available() >0){
    if(Serial.peek() == 's'|| Serial.peek()=='S'){  Serial.read();
      startByte =Serial.parseInt(); Serial.println("startByte=" +(String) startByte); }
    if(Serial.peek() == 'e'|| Serial.peek()=='E'){  Serial.read();
      endByte =Serial.parseInt();   Serial.println("endByte=" +(String) endByte);   
      rangeRead(startByte, endByte);                                                  } 
    while(Serial.available() >0){ Serial.read();  } 
  }
}

byte rangeWriteMenuOptions()  {
  if(Serial.available() >0){
    if(Serial.peek() == 's'|| Serial.peek()=='S'){ Serial.read();  
      startByte =Serial.parseInt(); Serial.println("startByte=" +(String) startByte); }
    if(Serial.peek() == 'e'|| Serial.peek()=='E'){  Serial.read();
      endByte   =Serial.parseInt(); Serial.println("endByte="   +(String) endByte);   }
    if(Serial.peek() == 'd'|| Serial.peek()=='D'){  Serial.read();
      dataByte =Serial.parseInt();  Serial.println("dataByte="  +(String) dataByte);
      rangeWrite(startByte, endByte, dataByte);                                       }
    while(Serial.available() >0){ Serial.read();  }  
   }
}

byte setEETypeMenuOptions()   {
  if (Serial.available()) {
    switch (Serial.read()) {
      case 49:  SETEETYPE(1,128);         break;
      case 50:  SETEETYPE(2,256);         break;
      case 51:  SETEETYPE(4,512);         break;
      case 52:  SETEETYPE(8,1024);        break;
      case 53:  SETEETYPE(16,2048);       break;
      case 54:  SETEETYPE(32,4096);       break;
      case 55:  SETEETYPE(64,8192);       break;
      case 56:  SETEETYPE(128,16384);     break;
      case 57:  SETEETYPE(256,32768);     break;
      case 65:  SETEETYPE(512, 65536);    break;
      case 97:  SETEETYPE(512, 65536);    break;
      case 66:  SETEETYPE(1024, 131072);  break;
      case 98:  SETEETYPE(1024, 131072);  break;
      case 67:  SETEETYPE(2048, 262144);  break;
      case 99:  SETEETYPE(2048, 262144);  break;
      /* X or x Exit & go back to Settings Menu */
      case 88:  setEETypeMode = false; SettingsMode = true; menuMGR();  break;
      case 120: setEETypeMode = false; SettingsMode = true; menuMGR();  break;
    }
  }
}
void SETEETYPE(int type, long max){
  eeType = type; eeMax = max;  
  EEPROMset = true; setEETypeMode = false; SettingsMode = true; menuMGR();
}

byte EraseModeMenuOptions() {
  if (Serial.available()) {
    switch (Serial.read()) {
      /* E or e ERASE */
      case 69:   ERASE();  EraseMode = false; MainMenuMode = true; menuMGR();  break;
      case 101:  ERASE();  EraseMode = false; MainMenuMode = true; menuMGR();  break;
      
      /* C or c to Cancel & go back to Main Menu */
      case 67: EraseMode = false; MainMenuMode = true; menuMGR();  break;
      case 99: EraseMode = false; MainMenuMode = true; menuMGR();  break;
    }
  }
}

byte saveSetMenuOptions(){
  // we need to get the vars into a string to write?
  // Safety Sound and LED Modes are either a 1 or a zero  (3Bytes)
  // eeprom address is 50-57                              (1Byte)
  // eeprom type is 1-12                                  (1Byte)

  
  byte sfty;
  byte snd;
  byte led;
  
  if(sftyMd){sfty=1;}else{sfty=0;}
  if(sndMd){snd=1;}else{snd=0;}
  if(ledMd){led=1;}else{led=0;}
  wrAdr(0,eeAdr); // DEFAULT EEPROM ADRESS 80  */stored as HEX#- needs load convert to DEC (50to80) then back to HEX (80to0x50)
  int myArray[12]={1,2,4,8,16,32,64,128,256,512,1024,2048};
  int myOtherArray[12]={129,130,132,136,128,64,32,16,8,4,2,1};
  for(int i=0; i<=12;i++){
    if(eeType == myArray[i]){wrAdr(1,myOtherArray[i]); }
  }
  
  //wrAdr(1,(eeType/2)); // EEPROM TYPE =(64K)        : (129=1K)(130=2K)(132=4K)(136=8K)   :(128=16K)(64=32K)(32=64K)(16=128K)(8=256K)(4=512K)(2=1024K)(1=2048K)
  wrAdr(2,sfty);  // SAFETY MODE               : 0 OFF 1 ON
  wrAdr(3,snd);  // SOUND MODE                : 0 OFF 1 ON
  wrAdr(4,led);  // LED MODE                  : 0 OFF 1 ON
  wrAdr(5,readMode); // READ DISPLAY METHOD ASC=65 BIN=66 DEC=68 HEX=72 OCT=79
  //wrAdr(6,0); // UNUSED
  //wrAdr(7,0); // UNUSED
  Serial.println(F("Settings Saved"));
  delay(3000);
  saveSetMode = false; MainMenuMode = true; menuMGR();
}

byte loadSettings(){
  /*LOAD EEPROM ADRESS */
  if(rdAdr(0)>49&&rdAdr(0)<58){eeAdr = rdAdr(0)+30;}
  Serial.print(F("eeAdr= "));
  Serial.println(rdAdr(0), DEC);

  /*LOAD EETYPE */
  //wrAdr(1,32); // testing
  if(rdAdr(1)>0 && rdAdr(1) <=128){ eeType = 2048/rdAdr(1);}
  else{ if (rdAdr(1)>128 && rdAdr(1)<137){eeType = rdAdr(1)-128;}}

  /*ASSIGN EEMAX & EESET*/
  eeMax= (eeType*1024L)/8;
  if(eeMax>0){ EEPROMset = true;Serial.print(F("eeMax=")); Serial.println(eeMax);Serial.print(eeType);}
  
  //getFreeBytes();
}
/*
void thisArrayThing(){
  int myArray[12]={1,2,4,8,16,32,64,128,256,512,1024,2048};
  int myOtherArray[12]={129,130,132,136,128,64,32,16,8,4,2,1};
  for(int i=0; i<=12;i++){
    if(eeType == myArray[i]){wrAdr(1,myOtherArray[i]); }
  }
}
*/
void coolPrint(String txt){
  int len = txt.length();
  char txtBuf[len];
  txt.toCharArray(txtBuf, len+1);
  //delay(1000);
  //Serial.print("String length =");
  //Serial.println((String)len);
  for(int i=0; i<len;i++){
    Serial.print(txtBuf[i]);
    Serial.print(F(" "));
    if(sndMd){beepbeep(1100,10);}
  }
  Serial.println();
  txt="";
}

void setwrSNMenuOptions() {
  if (Serial.available()>0) {
    byte   len =16;
    char  snBuf[len+1];
    byte  sn = Serial.readBytes(snBuf, len+1);
    int counter=0; /* must be int */
    String snTxt;
    Serial.print(F("\t\tWriting Serial Number: "));
    for (int i = (eeMax - len); i <= eeMax-1; i++) {
      if(sndMd){funBeep3(1);}
      Serial.print(snBuf[counter]);
      wrAdr(i, snBuf[counter]);
      snTxt+=snBuf[counter];
      counter ++;
    }
    Serial.println();
    getFreeBytes();
    //delay(3000);
    msgMenu("☺ FINISHED ☺" , "\tWrote Serial Number: " + snTxt, 2, 3000);
    wrSNMode = false; MainMenuMode = true; menuMGR();
    }
}



void wrAdr(int address, byte val)
{
  Wire.beginTransmission(eeAdr);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.write(val); Wire.endTransmission(); delay(5);
}

byte rdAdr(int address)
{
  byte rData = 0xFF;
  Wire.beginTransmission(eeAdr);
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(eeAdr, 1);
  rData =  Wire.read();
  return rData;
}

/*-------------------------------- GET STUFF --------------------------------------------------*/

/* READ DATA HAS NO Seperate MenuOptions */
void readData() {
  int i;
  int block = 0;
  int freeBlocks = 0;
  for (i = 0; i <= eeMax-1; i++) {
    block = block + 1;
    doCounterLine(i);
    print_i(rdAdr(i));
    Serial.print(" ");
  }
  Serial.println();
  msgMenu("Read EEPROM Complete", "\t\t" + (String)block + " bytes read.",2, 8000);
  readDataMode = false; MainMenuMode = true; menuMGR(); 
}

void getFreeBytes() {
  UAB=0;
  int i;
  int freeBytes = 0;
  int unfreeBytes = 0;
  //Serial.println();
  Serial.println("Analyzing EEPROM");
  for (i = 0; i < eeMax ; i++) {
    if (rdAdr(i) < 1) {
      freeBytes += 1;
      UAB+=1;
    } else {
      unfreeBytes += 1;
    }
  }
  /*Serial.print(unfreeBytes);
  Serial.println(" BYTES allocated to EEPROM memory");
  Serial.print(freeBytes);
  Serial.print(" of " );
  Serial.print(eeMax);
  Serial.println(" BYTES avaialable.");*/
}


void getRange(int stBlk, int enBlk) { //omit??
  //rdSN();
  msgMenu("Read EEPROM" , "\tAnalyzing EEPROM memory locations " + (String) stBlk + "-" + (String) enBlk,2, 3000);
  int i;
  int block = 0;
  int freeBlocks = 0;
  //int counter = 7;
  delay(2500);
  for (i = stBlk; i <= enBlk; i++) {
    block = block + 1;
    doCounterLine(i);
    print_i(rdAdr(i));

    if (rdAdr(i) < 1) {
      if (sndMd) {
        beepbeep(30, 30);
      }
      freeBlocks = freeBlocks + 1;
      //Serial.print(" . \t");
      Serial.print(rdAdr(i), HEX);
      Serial.print("\t");
    } else {
      if (sndMd) {
        beepbeep(20, 10);
      }
      //Serial.print("▒ \t");
      Serial.print(rdAdr(i), HEX);
      Serial.print("\t");
    }
  }
  Serial.println();
  msgMenu("Read EEPROM Complete", "\t\t" + (String)block + " bytes read.",2, 8000);
  /*Serial.println();
    Serial.println("-----------------");
    Serial.print("\t");
    Serial.print(block);
    Serial.print(" bytes read →");
    Serial.print(" ");
    Serial.print(((float)block/(float)eeMax)*100 +(String) "%");
    Serial.print(" of " + (String)eeMax + " bytes total.");
    Serial.println();
    Serial.print("\t");
    Serial.print(freeBlocks);
    Serial.print(" bytes available.");
    Serial.print(" ");
    float a= ((float) freeBlocks/ (float) block);
    float b= a*100;
    Serial.print(b, 2);
    Serial.print("%");
    Serial.println();
    Serial.print("\t");
    Serial.print(block-freeBlocks);
    Serial.print(" bytes used.");
    Serial.println();
    delay(8000);
  */
  //eMenu();
  
}


//========================================================= DO STUFF ===============================
//==================================================================================================
void rangeWrite(word startCell, word endCell, int data){
  int i;
  int wByte=0;
  int uByte=0;
  for(i=startCell; i <= endCell; i++){
    if(rdAdr(i) == data){if(sndMd){beepbeep(200,50);uByte++;} }else{wrAdr(i, data); wByte++; if(sndMd){beepbeep(1200,100);}}
    
  }
  rangeWriteMode = false;
  MainMenuMode=true;
  Serial.println("RangeWrite finished writing data " + (String) data +" to EEPROM range "+(String)startCell+" to "+(String)endCell);
  Serial.print("Of ");Serial.print((endCell-startCell)+1);Serial.print(" bytes total, ");
  Serial.print(wByte);
  Serial.print(F(" bytes were overwritten & "));
  Serial.print(uByte);
  Serial.println(F(" bytes were unchanged."));
  Serial.println();
  getFreeBytes();
  rangeRead(startCell, endCell);
  delay(3000);
  rangeWriteMode= false;MainMenuMode=true;
  startByte=0;endByte=0;dataByte=0;
  menuMGR();
}

void rangeRead(word startCell, word endCell){
  int i;
  for(i=startCell; i <= endCell; i++){
    if(sndMd){beepbeep(1200,100);}
    doCounterLine(i);
    print_i(rdAdr(i));
    Serial.print(" ");
  }
  rangeReadMode = false;
  MainMenuMode=true;
  Serial.println();
  Serial.println("RangeRead finished reading data from EEPROM range "+(String)startCell+" to "+(String)endCell);
  delay(3000);
  prLn(10);
  rangeReadMode= false;MainMenuMode=true;
  startByte=0;endByte=0;dataByte=0;
  menuMGR();
}



void ERASE(){
  msgMenu("ERASE EEPROM", "\tERASE EEPROM " + (String) eeMax + " bytes",2, 3000);
    word i;
    byte counter = 15;
    for (i = 0; i <= eeMax-1; i++) {
      counter = counter + 1;
      if (counter > 15) {
        Serial.println();
        Serial.print(i);
        Serial.print(F(" \t►"));
        Serial.print(F("\t"));
        counter = 0;
      }
      if(rdAdr(i)<1){ /* do nothing || if (sndMd) {beepbeep(1200, 2); }*/ }
      else{
      wrAdr(i, 0);
      if (sndMd) {beepbeep(120, 2); beepbeep(720, 1);}
      }
      Serial.print(" 0 ");
    }
    Serial.println();
    getFreeBytes();
    msgMenu("ERASE EEPROM", "\t\tFINISHED!",2, 1500);
    //menuMGR();
}

void RndWrite(unsigned int startCell, unsigned int endCell){
  int i;
  int block;
  int randNumber;
  Serial.print(F("Randomizing..."));
  for(i=startCell; i <= endCell; i++){
    block=block+1;
    randNumber = random(0, 255);
    beepbeep(600, 10);
    //flashYellow(5);
    wrAdr(i, randNumber);
    //Serial.print("Randomizing...");
    //Serial.println(i);
  }
  Serial.println(F(" done!"));
}

void rdSN()
{
  word i;
  String sn;
  for (i = 0; i < 16; i++) {
    sn += (char) rdAdr(i + (eeMax - 16));
  }
  Serial.print(F("SERIAL NUMBER: "));
  Serial.println(sn);
}


void getBlocks(unsigned int stBlk, word enBlk) {
  Serial.println(F("Analyzing EEPROM memory locations"));
  int i;
  
  for (int base = stBlk; base <= enBlk; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = rdAdr(base + offset);
    }
    char buf[80];
    sprintf(buf, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10],
            data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }
  delay(500);
}



/* CHECKS & SWITCHES*/
byte switchSafetyMode() {
  if (sftyMd) { sftyMd = false; if(sndMd){funBeep2(1);} return sftyMd;} 
  else{ sftyMd = true; if(sndMd){funBeep(1);}
  return sftyMd;
  }
}

byte switchLEDMode() {
  if (ledMd) { ledMd = false; if(sndMd){funBeep2(1);} return ledMd;} 
  else{ ledMd = true; if(sndMd){funBeep(1);} 
  return ledMd;
  }
}

byte switchSoundMode() {
  if (sndMd) { sndMd = false; funBeep2(1); return sndMd; } 
  else { sndMd = true; funBeep(1); 
  return sndMd;
  }
}

void safetyModeError() {
  prLn(16);
  msgMenu("☺ OOPS ☺", "It looks like you forgot to turn off Safety Mode!",1, 5000);
  menuMGR();
}
void eeTypeError(){
  prLn(16);
  msgMenu("☺ OOPS ☺", "It looks like you forgot to set the EEPROM Type!",1, 5000);
  menuMGR();
}
void rangeError() {
  prLn(16);
  msgMenu("☺ RANGE ERROR ☺", "Please enter a valid number (0-255)",1, 5000);
  menuMGR();
}

void checkEEPROMAddress() {
  Serial.print(F("EEPROM Address set to: 0x"));
  Serial.println(eeAdr, HEX);
}

byte print_i(byte i){
  
  if(readMode==65){ Serial.print(char(i)); return i;}else
  if(readMode==66){ Serial.print(i, BIN); return i;}else
  if(readMode==68){ Serial.print(i, DEC); return i;}else
  if(readMode==72){ Serial.print(i, HEX); return i;}else
  if(readMode==79){ Serial.print(i, OCT); return i;}
}

void doCounterLine(int i){
  counter2 = counter2 + 1;
    if (counter2 > 7) {
      Serial.println();
      Serial.print(i);
      Serial.print(F(" \t:"));
      Serial.print(F(" "));
      counter2 = 0;
      }
}

void checkEEPROMType()
{
  if (eeType < 1) {
    Serial.print(F("≡≡≡NOT SET≡≡≡"));
  } else {
    Serial.print((String)eeType + "K-bit (");
    Serial.print((String)eeMax + " bytes)");
    //return eeType;
  }
}
void getReadMode(){
  if(readMode==65){Serial.print(F("ASCII"));}
  if(readMode==66){Serial.print(F("BINARY"));}
  if(readMode==68){Serial.print(F("DECIMAL"));}
  if(readMode==72){Serial.print(F("HEXIDECIMAL"));}
  if(readMode==79){Serial.print(F("OCTAL"));}
}
void checkModes() {
  Serial.println(F("\t\t\tMITZ I²C™ EEPROM BYTEBANGER v0.75"));
  
  const String box[5]    ={"╔", "═", "╗", "╚", "╝"};
  Serial.print("   ");
  Serial.print(box[0]);
  for (byte i=0;i<41;i++){Serial.print(box[1]);}
  Serial.println(box[2]);
  
  
  //Serial.println(F("   ╔═════════════════════════════════════════╗"));
  Serial.println(F("\t    MODE STATUS\t\t\t\tEEPROM STATUS"));
  if (!sftyMd) {
    Serial.print(F("\tSAFETY MODE: OFF ×"));
  } else {
    Serial.print(F("\tSAFETY MODE: ON ☺"));
  }

  Serial.print(F("\t\tEEPROM Type: "));
  checkEEPROMType();
  Serial.println();

  if (!sndMd) {
    Serial.print(F("\tSOUND  MODE: OFF ×"));
  } else {
    Serial.print(F("\tSOUND  MODE: ON ♫"));
  }
  Serial.print(F("\t\tEEPROM Address: 0x"));
  Serial.println(eeAdr, HEX);

  if (!ledMd) {
    Serial.print(F("\tLED    MODE: OFF ×"));
  } else {
    Serial.print(F("\tLED    MODE: ON ☼"));
  }

  Serial.print("\t\t");
  rdSN();
  Serial.print(F("\tREAD   MODE: "));
  getReadMode();
  Serial.print(F("\t\tUNALLOCATED BYTES: "));
  Serial.println(UAB);
  
  Serial.print("   ");
  Serial.print(box[3]);
  for (byte i=0;i<41;i++){Serial.print(box[1]);}
  Serial.println(box[4]);
  //Serial.println(F("   ╚═════════════════════════════════════════╝"));
  prLn(2);

}
/*
                              MENUS
  
*/

/*                 MAIN MENU */
void mainMenu() {
  //if(sndMd){ hurrah();}
  Serial.println();
  checkModes();
  Serial.println(F("\t« MAIN MENU »"));
  Serial.println();
  for(int i=0; i<=4;i++){ /* type int saves 4bytes over type byte... IDKy*/
    doTab(0); 
    if(EEPROMset){  Serial.print(i);Serial.print(F(". "));Serial.print(menu[i]);}
    if(!EEPROMset){ if(i>0){Serial.print(F("≡"));} Serial.print(i);Serial.print(F(". "));Serial.print(menu[i]);}
    doTab(2); 
    if(!EEPROMset)            { Serial.print(F("≡"));}
    if(sftyMd && EEPROMset)   { Serial.print(F("☺"));}
    if(!sftyMd && EEPROMset){ } Serial.print(i+5);Serial.print(F(". "));Serial.println(menu[i+5]);
  }
  prLn(2);
  prDots();
  Serial.println();
}

/*                 SETTINGS MENU */
void settingsMenu() {
  checkModes();
  Serial.println(F("\tSETTINGS ○→"));
  Serial.println();
  exitTitle();
  exitOption(1);
  Serial.println();
  Serial.print(F("\t\t1. Turn Safety Mode "));
  if (sftyMd)  { boolOption(0); }
  if (!sftyMd) { boolOption(1); }
  Serial.println(F("\t4. Set Read Mode"));
  
  Serial.print(F("\t\t2. Turn Sound Mode "));
  if (sndMd)  { boolOption(0); }
  if (!sndMd) { boolOption(1); }
  Serial.println(F("\t5. Set EEPROM Type"));
  
  Serial.print(F("\t\t3. Turn LED Mode "));
  if (ledMd)  { boolOption(0); }
  if (!ledMd) { boolOption(1); }
  Serial.println(F("\t6. Set EEPROM Adress"));
   
  prLn(3);
  prDots();
}

/*                 READ DATA MENU */
void readDataMenu() {
  checkModes();
  Serial.println(F("○→ READ EEPROM"));
  prDots();
  //Serial.print(F("\tSelect data type format to be read or "));
  exitOption(2);
  Serial.println();
  
  prLn(3);
}


/*                 SET EEPROM ADDRESS MENU */
void setEEAdrMenu() {
  checkModes();
  Serial.println(F("\t○→ SET EEPROM ADDRESS"));
  exitTitle();
  exitOption(1);
  Serial.println();
  byte xAdr[] = {50, 51, 52, 53, 54, 55, 56, 57};
  int i;
  for (i = 0; i <= 3; i++) {
    doTab(1); Serial.print(i); Serial.print(F(". 0x")); Serial.print(xAdr[i]);
    doTab(2); Serial.print(i+4); Serial.print(F(". 0x")); Serial.println(xAdr[i+4]);
  }
  prLn(3);
  prDots();
}

/*                 SET EEPROM TYPE MENU */
void setEETypeMenu() {
  checkModes();
  Serial.println(F("\t○→ SET EEPROM TYPE"));
  exitTitle();
  exitOption(1);
  Serial.println();
  /* 1Kbit, 2Kbit, 4Kbit, 8Kbit, 16Kbit, 32Kbit, 64Kbit, 128Kbit, 256Kbit, 512Kbit, 1Mbit, 2Mbit*/
  int stuff[]={0,1,2,4,8,16,32,64,128,256,512,1024,2048};
  for(int i=1;i<=4;i++){
    Serial.print(F("\t"));
    Serial.print(i);
    Serial.print(F(". "));
    Serial.print(stuff[i]);
    Serial.print(F("K-bit"));
    
    doTab(1);//Serial.print(F("\t"));
    Serial.print(i+4);
    Serial.print(F(". "));
    Serial.print(stuff[i+4]);
    Serial.print(F("K-bit"));

    doTab(1);//Serial.print(F("\t"));
    if(i+8>9){Serial.print(char (63+i));}else{Serial.print(i+8);}
    //Serial.print(i+8);
    Serial.print(F(". "));
    Serial.print(stuff[i+8]);
    Serial.println(F("K-bit"));
  }
  prLn(2);
  prDots();
}

/*                 SET READ MODE MENU */
void setReadModeMenu() {
  checkModes();
  Serial.println(F("\t○→ SET READ MODE"));
  exitTitle();
  exitOption(2);
  Serial.println();
  for (int i = 0; i <= 2; i++) {
    doTab(1);
    Serial.print((i+1));
    Serial.print(F(". "));
    Serial.print(stuff[i] );

    if((i+4)>5){Serial.println();}else{
    doTab(1);//Serial.print(F("\t\t"));
    Serial.print((i+4));
    Serial.print(F(". "));
    Serial.println(stuff[i+3] );
  }}
  prLn(2);
  prDots();
  //prLn(3);
}

/*                 ERASE EEPROM MENU */
void EraseMenu() {
  checkModes();
  Serial.println(F("ERASE EEPROM"));
  //prDots();
  Serial.print(F("\tYou are about to Erase the EEPROM "));
  exitOption(4);
  Serial.println();
  
  prLn(3);
}


/*                 WRITE SN MENU */
void wrSNMenu() {
  checkModes();
  Serial.println(F("\t« WRITE SERIAL NUMBER »\t"));
  Serial.println();
  //prDots();
  
  Serial.println(F("\tBy default,the Serial Number is written to the last 16 byte locations."));
  Serial.println(F("\tType a 16 BYTE Serial Number into the Serial Monitor."));
  Serial.println(F("\tEXAMPLE: 24LC64-1419x0001")); /* this 16-Byte format I use decodes as "the 1st 24LC64 of week 14 2019. x= erased" */
  
  msgMenu("\t\t\t☺ IMPORTANT ☺", "Make sure the Serial Monitor is set to No line ending", 3,1000);
  //prLn(1);
}

/*                 RANGE WRITE MENU */
void rangeWriteMenu() {
  checkModes();
  Serial.println(F("○ RANGE WRITE"));
  prDots();
  Serial.println();
  Serial.println(F("To write data to a range of EEPROM address bytes use the format 's e d' \nwith no quotes or whitespaces."));
  Serial.println(F("Example: \nby typing s0e8d255 into the Serial Monitor, \nthe data 255 will be written to a byte adress range \nstarting at address 0 and ending at address 8"));
  Serial.println();
  prLn(3);
}

/*                 RANGE READ MENU */
void rangeReadMenu() {
  checkModes();
  Serial.println(F("○ RANGE READ"));
  prDots();
  Serial.println();
  Serial.println(F("To read data from a range of EEPROM address bytes use the format 's e' \nwith no quotes or whitespaces."));
  Serial.println(F("Example: \nby typing s0e511 into the Serial Monitor, \nEEPROM data will be read starting at address 0 and ending at address 511"));
  Serial.println();
  prLn(3);
}

/*                 BYTE SCANNER MENU */
void byteScanMenu() {
  checkModes();
  Serial.println(F("\t« BYTE SCANNER »"));
  Serial.println();
  Serial.println(F("\t\tThe Byte Scanner scans the EEPROM for specified data."));
  Serial.println(F("\t\tType a specific value to scan for (0-255)"));
  prLn(2);
  prDots();
  Serial.println();
  Serial.println(F("\t\t○ NOTE: Set Serial Monitor to No line ending. "));
  if(sndMd){Serial.println(F("\n\t\t♫ Sound Mode ON will slow the BYTE SCANNER\n\t\t\t(and can be annoying)"));}
  Serial.println();
  prDots();
  //prLn(2);
}

/*                 SAVE SETTINGS MENU */
void saveSetMenu() {
  checkModes();
  Serial.println(F("\t« SAVE SETTINGS »"));
  Serial.println();
  Serial.println(F("\t\tThe current settings displayed in the status window"));
  Serial.println(F("\t\tcan be saved to the EEPROM."));
  Serial.println(F("\t\tBy default, the save settings are allocated to EEPROM Memory"));
  Serial.println(F("\t\tadress 0-7. Save current settings now?"));
  prLn(2);
  prDots();
  /*Serial.println();
  Serial.println(F("\t\t○ NOTE: Set Serial Monitor to No line ending. "));
  if(sndMd){Serial.println(F("\n\t\t♫ Sound Mode ON will slow the BYTE SCANNER\n\t\t\t(and can be annoying)"));}
  Serial.println();
  prDots();
  //prLn(2);*/
}

/*                 MESSAGE MENU */
void msgMenu(String title, String msg, int alert, int dt) {
  int alert1;
  int alert2;
  if (sndMd && alert == 1) { beepbeep(50, 100);}
  if (sndMd && alert == 2) { beepbeep(1200, 100);}
  if (sndMd && alert == 3) { funBeep(2);}
  Serial.println();
  Serial.println("\t" + title);
  //Serial.print("\t"); coolPrint(title);
  prDots();
  Serial.println("\t\t" + msg);
  prDots();
  delay(dt);
  //prLn(2);
}



/*████****************************************************************** MENU HELPERS */
void prLn(int val) {
  for (int i = 1; i < val; i++) { Serial.println(); }
}

void prDots() {
  //Serial.println(F("\t"));
  Serial.println(F("            "));
}

void doTab(byte val) {
  for (byte i = 0; i <= val; i++) { Serial.print(F("\t")); }
}

void exitTitle(){
  Serial.print(F("\tChoose an option below or "));
}
void exitOption(int val) {
  if (val == 1) {Serial.println(F("Type X to Exit"));}
  if (val == 2) {Serial.println(F("Type C to Cancel"));}
  if (val == 3) {Serial.println(F("Type W to Write or C to Cancel"));}
  if (val == 4) {Serial.println(F("Type E to ERASE or C to Cancel"));}
}

void boolOption(byte val){
  if(val ==0){Serial.print(F("OFF"));}
  if(val ==1){Serial.print(F("ON")); }
}



/*████████████████████████████████████████ THE WALL*/
void countNumbers() {
  word i;
  for (i = 62000; i <= 65540; i++) {
    Serial.print(F("Number: "));
    Serial.print(i);
    Serial.print(F("\t Hex: "));
    Serial.print(i, HEX);
    Serial.println();
  }
}

void beep(int KHz, int td) {
  tone(spkr, KHz);
  delay(td);
  noTone(spkr);
  delay(td);
}

void beepbeep(int KHz, int td) {
  tone(spkr, KHz);
  delay(td);
  noTone(spkr);
  delay(td);
  tone(spkr, KHz);
  delay(td);
  noTone(spkr);
}

void funBeep(int td) {
  int i;
  for (i = 500; i < 2500; i = i + 5) {
    tone(spkr, i);
    delay(td);
  }
  noTone(spkr);
}
void funBeep2(int td) {
  int i;
  for (i = 2500; i > 500; i = i - 5) {
    tone(spkr, i);
    delay(td);
  }
  noTone(spkr);
}

void funBeep3(int td) {
  int i;
  for (i = 500; i > 100; i = i - 5) {
    tone(spkr, i);
    delay(td);
  }
  noTone(spkr);
}

void hurrah() {
  beepbeep(444, 100);
  beepbeep(555, 150);
  beep(666, 200);
}


/*████████████████████████████████████████████████████ THE BIG WALL
██████████████████████████████████████████████████████*/
/*/------------------------------------------------------------------------------------/*

void intro(){
  if (runlogo) {
    Serial.println(F("Initializing, please wait..."));
    beepbeep(100, 50); beepbeep(200, 50); beepbeep(50, 100);
    delay(2500);
    int i;
    int counter = -1;
    int x = 0;
    for (i = 0; i < 170; i++) {
      counter++;
      if (counter > 24) {
        Serial.print(" √");
        funBeep2(1);
        Serial.println();
        delay(5);
        //doTab(x);
        counter = 0;
        x++;
      }
      Serial.print(".");
      beepbeep(30, 8);
    }
    delay(1500);
    beepbeep(50, 100);
    Serial.print(F("?"));
    delay(1500);
    beepbeep(50, 100);
    Serial.print(F("?"));
    delay(1500);
    beepbeep(50, 100);
    Serial.print(F("?"));
    delay(1500);
    beepbeep(50, 100);
    Serial.print(F("?"));
    delay(1500);
    Serial.print(F("☺ √"));
    beepbeep(1200, 100);
    delay(1000);
    Serial.println();
    for (i = 0; i < 29; i++) {
      
      Serial.print(".");
      beepbeep(30, 8);
    }
    //doTab(3);
    Serial.println(F(" Mitzpatrick Fitzsimmons"));
    doTab(3);
    Serial.println(F("      PRESENTS"));
    funBeep(2);
    funBeep(1);
    funBeep2(2);
    delay(1250);
    
    doTab(2);
    Serial.println(F("  ↑       ↑   ↑     ↑       ↑   ↑   ")); doTab(2);
    Serial.println(F("╔◙◙◙◙◙◙◙◙╦╦╦◙◙◙◙◙◙◙◙╗ "));  doTab(2);
    Serial.println(F("╠◙  I²C EEPROM M★F BYTEBANGER ◙╣ ")); doTab(2);
    Serial.println(F("╚◙◙◙◙◙◙◙◙╩╩╩◙◙◙◙◙◙◙◙╝ "));  doTab(2);
    Serial.println(F("  ↓   ↓   ↓   ↓     ↓   ↓   ↓   ↓   "));
    hurrah();
  }
  runlogo = false;
  delay(3000);
  prLn(20);
}
//------------------------------------------------------------------------------------*/

//------------------------------------
/* Bit-BANGER GAME 
void BitBanger(int bits){
  int bangs=(bits/16);
  int score;
  int i;
  int counter;
  
  Serial.println(F("\t\t\t\t  BIT-BEATER"));
  /*
  Serial.println(F("   ╔═════════════════════════════════════════╗"));
  //Serial.print("\tBangs remaining: "+ (String)bangs);
  Serial.println("\t\t\t\t\tScore: "+ (String)score);
  Serial.println(F("   ╚═════════════════════════════════════════╝"));
  */
  /*int b;
  for(b=0; b<8; b++){
    int randNumber = random(0, 127);
    BeatBytes[8] += {randNumber};
  }
  //int randNumber = random(0, 127);
  //BeatBytes[8]+= randNumber;*/
  /*undo
  int a = random(0, 255);int b = random(0, 255);int c = random(0, 255);int d = random(0, 255);
  int e = random(0, 255);int f = random(0, 255);int g = random(0, 255);int h = random(0, 255);
  int q = a+b+c+d+e+f+g+h;
  Serial.println(F("   ╔═════════════════════════════════════════╗"));
  Serial.print(F("\tBITS to BEAT: ")); 
  Serial.print((String)a+" "+(String)b+" "+(String)c+" "+(String)d+" ");
  Serial.print((String)e+" "+(String)f+" "+(String)g+" "+(String)h+" ");
  Serial.print("\t");
  Serial.println((String)q);
  Serial.println(F("   ╚═════════════════════════════════════════╝"));
  for(i=0; i < bits; i++){
    counter = counter + 1;
    //Serial.print(i);
    //Serial.print(" ░ ");
    //Serial.print(i+"░");
    if (counter > 15) {  
      Serial.println();
      Serial.print(i);
      Serial.print("\t");
      counter = 0;
      }
    Serial.print(" ░ ");
  }
  prLn(3);  
  Serial.println(F("   ╚═════════════════════════════════════════╝"));
  //prLn(2);
  Serial.println(F("   ╔═════════════════════════════════════════╗"));
  Serial.print("\tScore: "+ (String)score);
  Serial.print("\tBit-Byters remaining: "+ (String)bangs);
  Serial.print("  ");
  byterLine(8);
  Serial.println();
  //Serial.println("\t\t\t\tScore: "+ (String)score);
  Serial.println(F("   ╚═════════════════════════════════════════╝"));
  delay(2000);
  //RndWrite(0,127);delay(2000);
  Serial.println("Choose a BYTE from 0 to 127");
  /*
  if (Serial.available()) {
    Serial.print("\tBit-Byters remaining: "+ (String)bangs);
    if(bangs >0){
      int fire = Serial.read();
      bangs+-1;
      Serial.println("You typed: " + (String) fire);
      Serial.print("BYTE TAKEN:" + (String)rdAdr(fire));
    }
  }*/       /*<undo
}
*/
/*
void byterLine(int val){
  int i;
  for(i=1; i <= val; i++){
    Serial.print(" ←Σ");
  } 
}

byte randByte(){
  int randNumber;
  int i;
  for(i=0; i<8;i++){
    randNumber = random(0, 127);
    BeatBytes[8]+= randNumber;
  }
  return randNumber;
  
}
*/
/*
void someFunction(){
  String temp;
  for(int i=0; i < 16; i++){
    temp = myBools[i];
    Serial.print(F("Checking array variable "));
    //Serial.print(i);
    delay(1000);
    if(myBools[i]){
      Serial.print( temp );
      Serial.println(F(" is TRUE"));
    }else{
      Serial.print( temp );
      Serial.println(F(" is FALSE"));
    }
  }
  Serial.println(F("Finished Checking."));
}
*/
/*
byte someOtherFunction(){
  bool myOtherBools[8] = {0,1,0,0,1,0,1,1};
  for(int i=0; i < 8; i++){
    
    Serial.print(F("Checking array variable "));
    Serial.print(i);
    delay(1000);
    if(!myOtherBools[i]){
      
      Serial.println(F(" is TRUE"));
    }else{
      
      Serial.println(F(" is FALSE"));
    }
  }
  Serial.println(F("Finished Checking."));
}
*/
/*
void readStringData(){
  String data;
  byte otherData[10];
  if (Serial.available()) {
    data = Serial.readString();
    beepbeep(1200, 80);
    Serial.println("The STRING is: " + data);  
  }
}
*/
// INCOMING BYTE vars- can these be local instead of global??
//unsigned int integerValue=0;  // Max value is 65535
//char incomingByte;

void IMCOMINGBYTE() {
  unsigned int integerValue=0;  // Max value is 65535
  char incomingByte;
  if (Serial.available() > 0) {   // something came across serial
    integerValue = 0;         // throw away previous integerValue
    while(1) {            // force into a loop until 'n' is received
      incomingByte = Serial.read();
      if (incomingByte == '\n') break;   // exit the while(1), we're done receiving
      if (incomingByte == -1) continue;  // if no characters are in the buffer read() returns -1
      integerValue *= 10;  // shift left 1 decimal place
      // convert ASCII to integer, add, and shift left 1 decimal place
      integerValue = ((incomingByte - 48) + integerValue);
    }
    if(integerValue <0 || integerValue >255){Serial.println(F("Number out of range (0-255)"));}
    else{
    Serial.println(integerValue);   // Do something with the value
    Serial.println(integerValue, HEX);   // Do something with the value
    Serial.println(integerValue, BIN);   // Do something with the value
    Serial.println();
    }
  }
}

byte NewbyteScanMenuOptions() {
  int scanVal;
  int qty=0;
  byte counter;
  if (Serial.available() > 0) {   
    while(Serial.available()==0) {          /* Wait for input  */ }
    scanVal=Serial.parseInt();  Serial.setTimeout(3000);
    /*if(!isdigit(scanVal)){                  Serial.println("scanVal is NOT a digit"); 
      byteScanMode = false; MainMenuMode = true; menuMGR();}
    else{                                   /*Serial.println("scanVal is a digit"); */
    
    Serial.println("scanVal="+(String)scanVal);
  if(scanVal <0 || scanVal >255){rangeError(); menuMGR();}
    else{
      prLn(16);
      Serial.print(F("\t░ SCAN FOR: "));
      Serial.print(scanVal);
      Serial.print(F("\t\t\tDISPLAY METHOD: ")); getReadMode(); Serial.println();
      prDots();
      Serial.print(F("\tDEC VAL:"));  Serial.print(scanVal, DEC);
      Serial.print(F("\tHEX VAL:"));  Serial.print(scanVal, HEX);
      Serial.print(F("\tOCT VAL:"));  Serial.print(scanVal, OCT);
      Serial.print(F("\tBIN VAL:"));  Serial.println(scanVal, BIN);
      prDots();
      Serial.println();
      delay(5000);

      for (long i = 0; i <= eeMax-1 ; i++) {
        doCounterLine(i);
        if (rdAdr(i) == scanVal) {
          qty++;
          if(sndMd){funBeep3(1);}
          Serial.print(F("\t"));
      print_i(rdAdr(i));
        }else {
          Serial.print(F("\t░"));
        }
    counter++;
    }
    prLn(8);
    msgMenu("☺ FINISHED ☺", "\n\t\tBYTE SCANNER found "+(String)qty+" of "+(String)eeMax+" bytes == "+(String)scanVal+"\n",2, 5000);
    byteScanMode=false; MainMenuMode=true;
    menuMGR();
    } 
    }
}
/*
void btnPress() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  //Serial.print("buttonPin reading: ");
  //Serial.println(reading);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}
*/
/*
void initSDCard(){
Serial.print(F("Initializing SD card..."));

  if (!SD.begin(8)) {
    Serial.println(F("initialization failed!"));
    while (1);
  }
  Serial.println(F("initialization done."));
}
*/
