/**
 * * Release V1.06
 * -adjusting PIN output according my configuration
 * -Timer1 enhancement for a better AF linearity during transmit
 * -Transmit cycle enhancement
 * -Temp monitoring is commented since i do not need this feature.
 * https://f5npv.wordpress.com/homebrew-ft8qrp-afp-fsk-trx/
 * https://www.youtube.com/watch?v=gx67r4AstVE
 ** ==============================================
 * F5NPV FT8QRP Transceiver
 * This TX is including FSK Capabilities compliant with FT8, WSPR , Rtty and many more
 * CAT is available for Digital mode 
 * No Rotary encoder since the TRX is controlled by CAT and dedicated for DIgital modes
 * With FLDIG and CAT you can change the frequency from FLDIGI and adjust any RTTY Frequency
 * Temperature reading with a LM35 during RX only 
 * =============================================
 * FT8QRP credits and initial experimentation  : http://elektronik-labor.de/HF/FT8QRP.html
 * First and initial PCB from UN7FGO and addon on the Sketch : https://easyeda.com/UN7FGO/QRP_FT8_ARDUINO_TRX
 * Sketch experimention from JE1RAV : https://github.com/je1rav/QP-7C 
 * DL9OBU https://www.qrz.com/db/DL9OBU
 * HA8HL for the initial CAT implementtion
 * F5NPV Experimentation , input ,addon and mods :
 * The Schematic , PCB and sketch are including : LPF switching , LCD display , opto relay control, i2c with multiple device
 * Biasing for the IRF530 or/and BS170, Output Transformer, TX and RX management with a relay and sequencer  
 * Temperature monitoring.
 * The sketch can be use without any CAT Connection and with or without LCD
 *  
 */
#include "si5351.h"
#include "Wire.h"
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // include i/o class header
// Use the I2C scanner in the ARDUINO to scan the SI5351 and LCD I2C adress to share them in the same bus.
// i use pullup resistor to allow the I2C bus a proper sharing of both devices but it works also without  
// you will need to allocate a different I2C adress to both devices (SI5351 and the LCD)
// On the LCD I2C interface if you solder both A0 pads the adress will be most of the time 0x26
// In case just use a I2C scanner sketch from the arduino IDE example
hd44780_I2Cexp lcd(0x26); // This is my LCD adress

Si5351 si5351;
//F5NPV LPF Pin Definition -----
// you can use also the spare PINs from the PCB and Arduino to drive the LPF separately
#define TX_LPF_A (9)//Band1 and LED 1  In addition LED will provide the LPF bank Status in line With UN7FGO addon
#define TX_LPF_B (10)//Band2 and LED 2
#define TX_LPF_C (11)//Band3 and LED 3
#define TX_LPF_D (12)//Band4 and LED 4

//F5NPV - Active Buzzer PIN for Out of Band
#define BUZZER_PIN  3

//F5NPV - Temperature reading with a LM35 during RX only - A0 is the connecting PIN for LM35----- 
const int TempPin = 0;

// Sorry. Comments from UN7FGO are in English only.
// ================================================
// Variable for storing the number of the current frequency.
int current_freq = 0;
// number of frequencies
int MAXFREQ = 4;
// Array of frequencies to switch.
unsigned long  freqs[4] = {3573000, 7074000, 10136000, 14074000};
// Array of contacts to display the selected frequency
unsigned long  pins[4] = {12,11,10,9};
// Contact for connecting the frequency change button.
#define FREQ_SWITCH_PIN 2 
// ================================================

long int freq = 7074000; // in Hz
   
int mode = 2;   
String received;
String receivedPart1;
String receivedPart2;    
String command;
String command2;  
String parameter;
String parameter2; 
String sent;
String sent2;
int TxStatus = 0; //0 =  RX 1=TX

void setup(void)
{
   // ================================================
  // Operating mode of the input contact with pull-up to power supply.
  pinMode (FREQ_SWITCH_PIN,INPUT_PULLUP);
  // The default frequency is always the first in the array.  
  freq = freqs[current_freq];
  // We set the mode of operation of contacts to display the selected frequency
  // and turn off all LEDs
  for (int i=0; i<MAXFREQ; i++) {
    pinMode (pins[i],OUTPUT);  
    digitalWrite(pins[i],LOW);
  }
  // Turn on the LED for the first frequency memory cell
  digitalWrite(pins[0],HIGH);
  // ================================================

Serial.begin(115200); 
Serial.setTimeout(4);

//lcd initialization----- 
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("===FT8QRP TRX===");
  lcd.setCursor(0,1);
  lcd.print("--UN7FGO-F5NPV--");
  delay (3000);  
  lcd.clear(); 

//freq= 1840000;
//freq= 3573000;
//freq= 5357000;    
//freq= 7038600;
freq= 7074000;// the frequency when power up or reset the transceiver
//freq= 7445000;
//freq= 10136000; //FT8 
//freq= 10138700; //WSPR
//freq= 14074000;
//freq= 18100000;
//freq= 21074000;
//freq= 24915000;
//freq= 28074000;
//freq= 26048000;
//freq= 50313000;

//F5NPV - My calibration setup
  word cal_factor = 42392;
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 25001100, 0); 
// End of calibration setup
  si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
 
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(freq*100ULL, SI5351_CLK1);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA);
//si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK1, 0);
 
  si5351.set_freq(freq*100ULL, SI5351_CLK0);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.output_enable(SI5351_CLK0, 0);

//TIMER1 SETUP for highly accurate timed measurements 
  cli();//stop all interrupts
// turn on CTC mode and set registor to 0
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
// turn on CTC mode
  TCCR1B |= (1 << WGM12);
//initialize counter value to 0;
  TCNT1  = 0;
  si5351.freq_calc_fast(df);
// set timer count for 31.25Hz increments
  OCR1A = 65000;//= (16*10^6) / (31.25*8) - 1 The OCR1A value must be less than 65536 (2^16 )= 65000 for prescaler 8
//-bits 0,1 and 2 : Clock source selection= 101-->Prescale output to1024 for 64 µs
//-bits 3 et 4 : WGM12 et WGM13 wave form generation
//-bit 6 : ICES1 (Input Capture Edge Select), ICES1==0 falling edge /ICES1==1 rising edge
//-bit 7: ICNC1: Input Captur Noise Canceler: 1-->activated
//TCCR1B = 0b01000001// 1024 prescaler
  TCCR1B = 0b00000001;//8 prescaler with rising edge detection and noise canceller
  ACSR |= (1<<ACIC);  // Analog Comparator Capture Input
  sei();//allow interrupts

//F5NPV PIN management for RX and TX
  pinMode(7, INPUT); //PD7 = AN1 = HiZ, PD6 = AN0 = 0
//To trigger the TX LED and Sequencer relay
  pinMode(13, OUTPUT);
  pinMode(4, OUTPUT);
}

void loop(void)
{
  if(Serial.available() > 0) cat();
  setTXFilters(freq);
  lcd.setCursor(0,1);
  lcd.print("VFO:");
  String freqString =  String(freq, DEC);
  lcd.setCursor(4,1-freqString.length());
  lcd.print(freqString);
  // Added UN7FGO.
  // ================================================
  // We process the press of the frequency change button.
  if (digitalRead(FREQ_SWITCH_PIN) == 0) {
    // If the button is pressed, then go to the next frequency.
    // Increase the frequency number by 1.
    current_freq++;
    // If we reach the last frequency in the list, go to the beginning of the list.
    if (current_freq == MAXFREQ) {
      current_freq = 0;
    }
    // We read a new frequency from the array.
    freq = freqs[current_freq];
    // We set the frequency of the receiver local oscillator.
    si5351.set_freq(freq*100ULL, SI5351_CLK1);
    // Turn off all LEDs
    for (int i=0; i<MAXFREQ; i++) {
      digitalWrite(pins[i],LOW);
    }
    // Turn on the LED for the selected frequency
    digitalWrite(pins[current_freq],HIGH);
    // We are waiting for half a second to avoid reprocessing of pressing the frequency change button.
    delay(500);
  }  
  // ================================================
  // End of addition from UN7FGO.
  
 // F5NPV Measure modulation frequency via Analog Comparator Pin7 = AN1
 // Cycle enhancement for an accurate modulation
 int FSK = 10;
 int FSKtx = 0;
 while (FSK>0){
 int Nsignal = 10;
 int Ncycle01 = 0;
 int Ncycle12 = 0;
 int Ncycle23 = 0;
 int Ncycle34 = 0;
 unsigned int d1=1,d2=2,d3=3,d4=4;
  TCNT1 = 0;  
  while (ACSR &(1<<ACO)){
    if (TIFR1&(1<<TOV1)) {
      Nsignal--;
      TIFR1 = _BV(TOV1);
      if (Nsignal <= 0) {break;};
  }
  }  while ((ACSR &(1<<ACO))==0){
    if (TIFR1&(1<<TOV1)) {
      Nsignal--;
      TIFR1 = _BV(TOV1);
      if (Nsignal <= 0) {break;}
   }
  }
  
  if (Nsignal <= 0) {break;}
   TCNT1 = 0;
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle01++;
      TIFR1 = _BV(TOV1);
      if (Ncycle01 >= 2) {break;}
    }
  }
 d1 = ICR1;  
  while ((ACSR &(1<<ACO))==0){
      if (TIFR1&(1<<TOV1)) {
      Ncycle12++;
      TIFR1 = _BV(TOV1);
      if (Ncycle12 >= 3) {break;}      
    }
  } 
 while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle12++;
      TIFR1 = _BV(TOV1);
      if (Ncycle12 >= 6) {break;}
    }
  }
d2 = ICR1;
  while ((ACSR &(1<<ACO))==0){
      if (TIFR1&(1<<TOV1)) {
      Ncycle23++;
      TIFR1 = _BV(TOV1);
      if (Ncycle23 >= 3) {break;}
    }
  } 
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle23++;
      TIFR1 = _BV(TOV1);
      if (Ncycle23 >= 6) {break;}
      }
  } 
  d3 = ICR1;
  while ((ACSR &(1<<ACO))==0){
      if (TIFR1&(1<<TOV1)) {
      Ncycle34++;
      TIFR1 = _BV(TOV1);
      if (Ncycle34 >= 3) {break;}
    }
  } 
  while (ACSR &(1<<ACO)){
      if (TIFR1&(1<<TOV1)) {
      Ncycle34++;
      TIFR1 = _BV(TOV1);
      if (Ncycle34 >= 6) {break;}
      }
  } 
  d4 = ICR1;
  unsigned long codefreq1 = 1600000000/(65536*Ncycle12+d2-d1);
  unsigned long codefreq2 = 1600000000/(65536*Ncycle23+d3-d2);
  unsigned long codefreq3 = 1600000000/(65536*Ncycle34+d4-d3);
  unsigned long codefreq = (codefreq1 + codefreq2 + codefreq3)/3;
  if (d3==d4) codefreq = 5000;
  if ((codefreq < 310000) and  (codefreq >= 10000)) {
      if (FSKtx == 0){
          digitalWrite(13,1);
          digitalWrite(4,1);
          si5351.output_enable(SI5351_CLK1, 0);   //RX off
          si5351.output_enable(SI5351_CLK0, 1);   // TX on
          delay(20);
          lcd.setCursor(0,0);
          lcd.print("TX");
          //digitalWrite(13,1);
      }
      si5351.set_freq((freq * 100 + codefreq), SI5351_CLK0);  
      FSKtx = 1;
    }
  
  else{
    FSK--;
  }
 }
  digitalWrite(13,0);
  digitalWrite(4,0);
  si5351.output_enable(SI5351_CLK0, 0);   //TX off
  si5351.output_enable(SI5351_CLK1, 1);   //RX on
  /*
//F5NPV  - LM 35 Temperature sensor , will need to be calibrated , for myself i adjust to 800 the parameter . 
//You will need to adjust according your LM35 and move bit by bit up or down the 800 parameter eg : 900 or 700
  int value = analogRead(TempPin); // read the value from the sensor
  float millivolts = value * (5.0 / 1023.0 * 800); 
  float T = millivolts / 10;
  lcd.setCursor(10,0);
  lcd.print(T);
  lcd.print("C");
  delay(20);
*/
  lcd.setCursor(13,0);
  lcd.print("FT8");
  lcd.setCursor(0,0);
  lcd.print("RX");
  FSKtx = 0;
  
}
//remote contol (simulating TS-2000)
//"ft8qrp_cat11.ico" from https://www.elektronik-labor.de/HF/FT8QRP.html from HA8HL
void cat(void)
{     
    received = Serial.readString();  
    received.toUpperCase();  
    received.replace("\n","");  

           String data = "";
           int bufferIndex = 0;

          for (int i = 0; i < received.length(); ++i)
               {
               char c = received[i];
    
                 if (c != ';')
                    {
                    data += c;
                    }
                 else
                    {
                        if (bufferIndex == 0)
                          {  
                              data += '\0';
                              receivedPart1 = data;
                              bufferIndex++;
                              data = "";
                          }
                         else
                          {  
                              data += '\0';
                              receivedPart2 = data;
                              bufferIndex++;
                              data = "";
                          }
                    }

               }
    
    command = receivedPart1.substring(0,2);
    command2 = receivedPart2.substring(0,2);    
    parameter = receivedPart1.substring(2,receivedPart1.length());
    parameter2 = receivedPart2.substring(2,receivedPart2.length());

    if (command == "FA")  
    {  
        
          if (parameter != "")  
              {  
              freq = parameter.toInt();
              }
          
          sent = "FA" // Return 11 digit frequency in Hz.  
          + String("00000000000").substring(0,11-(String(freq).length()))   
          + String(freq) + ";";     
    }

    else if (command == "PS")  
        {  
        sent = "PS1;";
        }

    else if (command == "TX")  
        {   
        sent = "TX0;";
        TxStatus = 1;
        delay(50);
       
        }

    else if (command == "RX")  
        {  
        sent = "RX0;";
        TxStatus = 0;
        delay(50);
        
        }

    else  if (command == "ID")  
        {  
        sent = "ID019;";
        }

    else if (command == "AI")  
        {
        sent = "AI0;"; 
        }

    else if (command == "IF")  
        {
          if (TxStatus == 1)
            {  
            sent = "IF" // Return 11 digit frequency in Hz.  
            + String("00000000000").substring(0,11-(String(freq).length()))   
            //+ String(freq) + String("     ") + "+" + "0000" + "0" + "0" + "0" + "00" + "0" + String(mode) + "0" + "0" + "0" + "0" + "00" + String(" ") + ";"; 
            + String(freq) + "00000" + "+" + "0000" + "0" + "0" + "0" + "00" + "1" + String(mode) + "0" + "0" + "0" + "0" + "000" + ";"; 
            } 
             else
            {  
            sent = "IF" // Return 11 digit frequency in Hz.  
            + String("00000000000").substring(0,11-(String(freq).length()))   
            //+ String(freq) + String("     ") + "+" + "0000" + "0" + "0" + "0" + "00" + "0" + String(mode) + "0" + "0" + "0" + "0" + "00" + String(" ") + ";"; 
            + String(freq) + "00000" + "+" + "0000" + "0" + "0" + "0" + "00" + "0" + String(mode) + "0" + "0" + "0" + "0" + "000" + ";"; 
            } 
       }
  
    else if (command == "MD")  
      {  
      sent = "MD2;";
      }

//------------------------------------------------------------------------------      

    if (command2 == "ID")  
        {  
        sent2 = "ID019;";
        }
            
    if (bufferIndex == 2)
        {
        Serial.print(sent2);
        }
        
    else
    {
        Serial.print(sent);
    }  

   if ((command == "RX") or (command = "TX")) delay(50);

    sent = String("");
    sent2 = String("");  
}
//F5NPV LPF and LED Switching matrix -----  
//You can adjust the matrix according the frequencies you are targetting if you are using LPF 
void setTXFilters(unsigned long freq){
  
  if (freq > 3500000 && freq < 3800000){  // 
    digitalWrite(TX_LPF_A, 1);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
    digitalWrite(TX_LPF_D, 0);
    lcd.setCursor(15,1);
    lcd.print("1");  
    digitalWrite(BUZZER_PIN, LOW);
   
  }
  else if (freq > 7000000 && freq < 7200000){ //
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 0);
    digitalWrite(TX_LPF_D, 0);
    lcd.setCursor(15,1);
    lcd.print("2"); 
    digitalWrite(BUZZER_PIN, LOW);

  }
  else if (freq > 10100000 && freq < 10200000){
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 1);
    digitalWrite(TX_LPF_C, 0);
    digitalWrite(TX_LPF_D, 0);
    lcd.setCursor(15,1);
    lcd.print("2"); 
    digitalWrite(BUZZER_PIN, LOW);
  
  }
    else if (freq > 14000000 && freq < 14300000){
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0);
    digitalWrite(TX_LPF_D, 1);
    lcd.setCursor(15,1);
    lcd.print("4"); 
    digitalWrite(BUZZER_PIN, LOW);
 
  }
  else {
    digitalWrite(TX_LPF_A, 0);
    digitalWrite(TX_LPF_B, 0);
    digitalWrite(TX_LPF_C, 0); 
    digitalWrite(TX_LPF_D, 0);
    digitalWrite(13,0);
    digitalWrite(4,0);
//F5NPV - It will trigger a buzzer if out of band
    digitalWrite(BUZZER_PIN, HIGH);
    //delay (1500);   
  }
}
