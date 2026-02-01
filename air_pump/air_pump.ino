#include <avr/interrupt.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_PC3, /* data=*/ PIN_PC2, /* reset=*/ U8X8_PIN_NONE);


int i;
unsigned long timer = 300000;
unsigned long lastA3;
unsigned long lastA4;
bool currA5;
bool prevA5 = true;
unsigned long lastA5=101;
bool currA6;
bool prevA6 = true;
unsigned long lastA6=101;
int opCode;
int minutes;
int seconds;
unsigned long startTime;
unsigned long startTimeTimer=100;
int lasti;
int addr = 0;
String timerString;
float percent;
unsigned long timeRemaining= 0;
unsigned long runTime;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(); //Initiailize wire we might not need this
  
  u8g2.begin(); //Start the screen
  u8g2.setFont(u8g2_font_ncenB14_tr);

  PORTA.DIRCLR = PIN3_bm;                    // Input PA3
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;         // Enable pull-up
  PORTA.DIRCLR = PIN4_bm;                    // Input PA4
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm;         // Enable pull-up
  PORTA.DIRCLR = PIN5_bm;                    // Input PA5
  PORTA.PIN5CTRL = PORT_PULLUPEN_bm;         // Enable pull-up
  PORTA.DIRCLR = PIN6_bm;                    // Input PA6
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;         // Enable pull-up
  PORTC.DIRSET = PIN1_bm;                     //Output PC1
  PORTB.DIRSET = PIN2_bm;                     //Output PB2
  PORTB.DIRSET = PIN1_bm;                     //Output PB1
  
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;  // Use clk/64 // Enable the TCA0 timer (use the system clock, default 3.33 MHz from 20 MHz / 6 prescaler)
  TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;       // Enable timer
  TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP1EN_bm; // Set waveform generation mode to single-slope PWM
  // Set PWM period (TOP value) — higher = slower PWM
  // e.g. 0xFF for 8-bit PWM
  TCA0.SINGLE.PER = 255;
  // Set duty cycle: CMP1 = compare match for WO1 (PC1)
  // e.g. 50% duty
  
  i = EEPROM.read(addr);
  i = lasti;
  opCode = 0;
  PORTB.OUTSET = PIN1_bm;                     //Set PB1 high

}

void loop() {
  u8g2.clearBuffer();

    
  //button 1
  if (!(PORTA.IN & PIN3_bm) && (millis()-lastA3 > 100)){
      i++; 
      if (i >= 200) {i = 200;} 
      lastA3=millis();
  }
             
  //button 2
    if (!(PORTA.IN & PIN4_bm) && (millis()-lastA4 > 100)){
      i--; 
      if (i <= 0) {i = 0;} 
      lastA4=millis();
  }
  
  //button 3
currA5 = PORTA.IN & PIN5_bm; 
if (prevA5 == false && currA5 == true && (millis() - lastA5 > 100)) {
//timer
if (opCode > 0) {opCode = 0;} 
else {opCode = 1;} 
  
  startTimeTimer = millis(); 
  lastA5 = millis();
  }
  prevA5 = currA5;
  
  //button 4
currA6 = PORTA.IN & PIN6_bm; 
if (prevA6 == false && currA6 == true && (millis() - lastA6 > 100)) {
//continous
if (opCode > 0) {opCode = 0;} 
else {opCode = 2;}

  lastA6=millis();
  startTime = millis(); 
}

prevA6 = currA6;

  //Memory loop
  if (i != lasti) {  
    EEPROM.write(addr, i);
    lasti= i;
    } 

 percent = i/200.0;

 
 u8g2.drawStr(10, 35, String(millis()).c_str()); //i as  a percent
 u8g2.drawStr(80, 35, "%"); //i as  a percent
 
  
  //opCode logic
  switch (opCode){

  case 0:
  PORTB.OUTCLR = PIN2_bm;                     //Set PB2 low
  TCA0.SINGLE.CMP1 = 0;
  u8g2.drawStr(30, 40, "--:--");
  break;
  
  case 1:
  timeRemaining = timer - millis() + startTimeTimer;
  
  if (timeRemaining <= 0) {
  timeRemaining = 0;
  PORTB.OUTCLR = PIN2_bm;                     //Set PB2 low
  TCA0.SINGLE.CMP1 = 0;
  opCode = 0;
  }

  else {
  PORTB.OUTSET = PIN2_bm;                     //Set PB2 high
  TCA0.SINGLE.CMP1 = i;
  minutes = floor(timeRemaining/60000);
  seconds = ((timeRemaining%60000)/1000) ; 
  timerString.concat(String(minutes)); 
  timerString.concat(":");
  if (seconds < 10) {timerString.concat("0");} 
  timerString.concat(String(seconds));
  u8g2.drawStr(30,40,timerString.c_str()); 
  timerString = "";
  }
  
  break;

  case 2:
  PORTB.OUTSET = PIN2_bm;                     //Set PB2 high
  runTime =  millis() - startTime;
  TCA0.SINGLE.CMP1 = i;
  minutes = floor(runTime/60000);
  seconds = ((runTime%60000)/1000) ; 
  timerString.concat(String(minutes)); 
  timerString.concat(":");
  if (seconds < 10) {timerString.concat("0");} 
  timerString.concat(String(seconds));
  u8g2.drawStr(30,40,timerString.c_str()); 
  timerString = "";
  break;
  
  default:
  
  PORTB.OUTCLR = PIN2_bm;                     //Set PB2 low
  TCA0.SINGLE.CMP1 = 0;
  u8g2.drawStr(30, 40, "--:--");
  
  break;
}
  u8g2.sendBuffer();


 
}
