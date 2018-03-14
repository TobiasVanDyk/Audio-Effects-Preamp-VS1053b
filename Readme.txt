Audio Effects Preamp With VS1053b

Introduction: 

This is a fully functional audio effects preamp using the VLSI VS1053b Audio DSP IC. It has a potentiometer to adjust the volume and the five effect parameters. It has nine fixed effects and one customizable effect, where each effect has five effects settings namely delay, decay-repeat, modulation speed and depth, and the mix-ratio of the processed and direct audio. It includes adjustments for bass and treble boost, bass and treble centre frequency, a selection of six input gain values, an option for saving or retrieving the current/saved parameters to/from the Arduino Eeprom, and a basic/normal/advanced/edit menu option that determines the number of functions that are cycled through. Adjustments are made using three pushbuttons, namely a function select button and two buttons to increase and decrease values for the selected function .
Although it is a fully functioning effects box, it has not yet been built for use in a live environment.
?Add TipVAsk Question|CommentDownload

Step 1: Construction and Parts List

The preamp has three pushbuttons - a function select button and two buttons to increase and decrease the selected function value. It also uses a potentiometer as a volume control, or it can be used to set values for the five effects parameters. These effects parameters are modulation speed and depth (used in chorus, phaser, and flanging effects), or delay time and repetition (used in echo and reverb effects). The fifth parameter is used to set the ratio of the direct to processed audio path. The function select button cycles through: (1) Effects select (0 to 9), (2) Volume select (adjusted with the potentiometer), (3) a bass boost adjustment, (4) a treble booost adjustment, (5) bass and (6) treble centre frequency selection (from 20Hz to 150 Hz in 10 Hz steps and from 1 kHz to 15 kHz in 1 kHz steps), (7) an input gain selection adjustable from 1/2x to 1x, up to 5x gain, (8) Saving or reading parameters to the ATmega328 eeprom, (9) function select cycle detail (from all 14 cycles to the 6 cycle edit mode that only cycles through the five effects paramers), and (10) to (14), adjustment of the five effect parameters using the potentiometer .
The Adafruit VS1053 Breakout board is recommended but the Sparkfun Board can also be used provided two jumper wires are soldered to pins 1 and 48 of the IC package. These will then be used as Line In2 and Line In1. Despite my best effort I could not get a Geeetech board (red variant) to work with the effects code - it is possible that it may be a special Shenzhen variant of the VS1053 design...

Parts List:

ATmega328 Arduino Uno R3 
Wemos 64x48 I2C OLED Display or similar 
Adafruit VS1053b Codec breakout board (or a Sparkfun VS1053 Breakout Board - soldering needed) 
3 x miniature pushbuttons 
100k potentiometer linear 
2 x Stereo audio socket to connect to an amplifier and input 
Resistors: 5 x 10k, 3 x 470 ohm 
Capacitors: 1uf 25v electrolytic 
Yellow and red LED 1 x Foot Switch

Step 2: Software

The Arduino sketch attached (Effect34.ino), is based on the Adafruit VS1053 library, and the VLSI effects processing code is loaded as a plugin within the Arduino sketch.

Further details of the VLSI effects processing can be obtained by installing their development tool - VSIDE - obtainable from their website, and then opening the folder VSIDE\templates\project\VS10X3_Audio_Effects. I used their Coff2All tool to convert the executable file into a C code type plugin which was then copied into the Arduino sketch and which loads before the loop function of the sketch starts.
The software monitors three pushbuttons. The first button cycles through 9 functions and the 5 effects parameters. Function 1 offers 10 effects such as a Wet Echo, Phaser, Flanger, Chorus, Reverb and a Dry Echo as effects 0 to 6. Effects 7 and 8 are zeroed - i.e. there is no processing of the audio input - this can be changed in the Arduino code by giving values for the five effects parameters. The up and down pushbuttons are then used to select the effects function 0 to 9, or they are used to set values for the other functions such as bass boost.
This function button is also used to select the bass and treble boost vales (as 16 steps), and the centre frequency for the treble boost (1 to 15 kHz in 1 Khz steps) and the bass boost frequency (From 20 Hz to 150 Hz in 10 Hz steps. It is also used to select an input gain which can be adjusted to 0.5x, 1x, 2x, 3x, 4, or 5x gain. There is an option to save the current parameters (Volume, Bass and Treble Boost, Bass and Treble Frequency, and the five effects parameters for the customisable effect), and also to retrieve these paramers at a later stage.
Because the function select pushbutton cycles through a large number of options (15), it has an option to set a basic mode where the number of cycles are reduced to Effects Select (0 to 9), Volume Select, Bass Boost Select, Treble Boost Select, or a normal mode which adds the 5 Effects parameters to that of the basic mode, as well as its default full mode. There is also an edit mode that only cycles through the five effects parameters.

A potentiometer is used to control the volume and it is also used to set the five effects parameters for effect number 9, i.e. the effects can be adjusted by turning the potentiometer.

In addition the code running on the VS1053 caters for the installation of a footswitch connected to the VS1053 GPIO3 pin to enable or disable the currently selected audio effect. NB: This must be galvanically connected to 3.3 volt and not 5 volt (as used by the Arduino Uno). An LED is on when the effects are processed and off when it is a direct audio loopthrough. An activity LED is used to confirm important operations such as reading or writing from the Eeprom.

A slightly modified version of the Adafruit Graphics library had been used to cater for the 64x48 pixel resolution of the OLED Display - please refer to the links given at the end for Mr Mcauser. A list of the required libraries is given in the sketch code.
Credit is given to all the persons and entities mentioned for their code and libraries.


Step 3: Links

VLSI: http://www.vlsi.fi/en/home.html
Adafruit:  https://www.adafruit.com/product/1381
Github VS1053b:  https://www.adafruit.com/product/1381
Github Graphics:  https://www.adafruit.com/product/1381
Oled:  https://www.adafruit.com/product/1381
Sparkfun:  https://www.sparkfun.com/products/12660



