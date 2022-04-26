# FT8QRP_AFP_FSK_TRX
The main purpose of this transceiver is the ability to use Digital modes and able to contact other station using FT8 or FT4.
The TRX is able to use RTTY mode.

PSK31 Mode is not available

all gerber and design are located  hereafter:

https://easyeda.com/F5NPV/qrp_ft8_arduino_trx_v2
 
https://easyeda.com/F5NPV/ft8qrp-top
 
https://easyeda.com/F5NPV/ft8qrp-lpf_copy_copy_copy
 
https://easyeda.com/F5NPV/ft8qrp-lpf

2 release are presented in this webpage:

An analog design using a cheap CD2003 chip
A SDR Design
CREDITS
FT8QRP credits and initial experimentation : http://elektronik-labor.de/HF/FT8QRP.html

First and initial PCB from UN7FGO and addon on the Sketch : https://easyeda.com/UN7FGO/QRP_FT8_ARDUINO_TRX (many thanks Gennady)
Sketch experimention from JE1RAV : https://github.com/je1rav/QP-7C
DL9OBU https://www.qrz.com/db/DL9OBU
DL9UPJ Aleksandr https://www.qrz.com/DB/DL9UPJ
F5NPV Experimentation , input ,addon and mods :

The Schematic , PCB and sketch are including : LPF switching , LCD display , relay control, i2c sharing the LCD and SI5351
Biasing for the IRF530 or/and BS170, Output Transformer, TX and RX management with a relay and sequencer
Temperature on LCD 1602 display
LPF low pass filter board
BPF Band pass filter board
Achievement

This TRX is part of a full homebrew combo dedicated for portable and Holidays:

ATX power supply modified for 13.8vdc
A tiny Homebrew about 40 up to 50w Mosfet Amplifier with embedded controler (Yeah i know but i am not a QRP guy !)
A Homebrew USDX for SSB and CW
A 3D printed CW Key

This tiny TRX is 4 Bands (80,40,30 and 20m) with about 3 up to 6 watts using an IRF530 (30ma and 3.70vdc for the biasing) or the design is able to provide you the option to drive an external amplifier and i will use this option since my 3w amplifier is more efficient with less current compare to the IRF530 (350ma vs 1amp during transmit). This will provide the capability and long run using a 12vdc battery or a phone powerbank + a tiny 12vdc booster.

The TRX is able to run digital modes and the capability to contact Amateur radio using FT8, FT4, JT65, WSPR and RTTY (Some other modes should be tested). This not a DSB transceiver but a sideband TRX (USB for instance).




