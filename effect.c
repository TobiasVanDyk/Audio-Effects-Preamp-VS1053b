/* For free support for VSIDE, please visit www.vsdsp-forum.com */
// See line 49 and 56 59 and 195 198 221 224 244 245 for changes 20/12/2017

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Audio effect example for VS1053/VS1063
//
// Flanger, chorus and delay with adjustable parameters
// (see "effect.h" for the parameters)
//
// press button on board to bypass effect when program is running
// There's really nothing "hidden" from you in the VS1053 audio effects template. If you look at the loop in effect.c, you 
// can see that it only calls StreamBufferReadData to get the input samples and AudioOutputSamples to push samples to the DAC. 
// There are no other calls to VS1000 ROM, except the AD interrupt handler calls auto gain functions and the startup calls 
// InitHardware and SetHardware to make sure the DACs, ADCs etc are switched on. All the audio processing (effects) is done
// in the main() function of effect.c.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define USE_SIMPLE_DSP_BOARD
//#define USE_PROTOTYPING_BOARD
// (define *one* of the PCB types before including board.h)
#include "board.h"

#define USE_MIC 0 // define 1 for mic-in, 0 for line-in

#include <string.h>
#include <stdlib.h>
#include "effect.h"

void InitAudioExample(u_int16 srInput,int useMicIn,u_int16 coreClock); // (see "init.c")
// Remember to never allocate buffers from stack space. So, if you
// allocate the space inside your function, never forget "static"!
static s_int16 __y delayBuffer[DELAY_BUF_SZ]; // delay line (buffer in Y memory)

// These are at addr 0x0800, 0x0801,...0x0805
// Can be accessed from Arduino with:
// addr1 = 0x0800;
// musicPlayer.sciWrite(VS1053_REG_WRAMADDR, addr1);
struct Effect {
	u_int16 depth;
	u_int16 speed;
	u_int16 delay;
	u_int16 decay;
	u_int16 mix;
	u_int16 mixOrg;
};
struct Effect __x effect = {DEPTH, SPEED, DELAY, DECAY, MIX, MIX};

// Probably at address 0x086
s_int16 __x loopVar0 = 0;

// Probably at address 0x087
// Indicate button GPIO3 being pressed
s_int16 __x NoEffect = 0;

// Probably at address 0x088
s_int16 __x loopVar1 = 0;

// Probably at address 0x089
s_int16 __x loopVar2 = 0;

// avoid being called again (when executed as a plug-in)
//extern s_int16 (*applAddr)(s_int16 register __i0 **d, s_int16 register __a1 mode, s_int16 register __a0 n);

// __y indicates that Y RAM is used
// For more information see the VSDSP tools manual
/************************************************************************************************************
We have two large ring buffers in VS1053. One of them is the stream buffer, the other the audio buffer. 
The stream buffer is used for unprocessed data, be it WAV, MP3, AAC, or something else. The audio buffer is 
used for decoded and processed, 16-bit PCM stereo audio samples that are going to the DAC. For details, see V
S1053b Datasheet Chapter 9.2, Data Flow of VS1053b.

Reading the status of the stream buffer, which lies at address y:0x1a7d, hence in BufferFillWords() there is a write of value 0x5a7d to SCI_WRAMADDR.

The fill status of the audio buffer. The address for that is y:0x1a80. The pointer works similarly as the stream buffer pointer, except that the VS1053 audio buffer size is 4 KiWords (or, 2 stereo kilosamples). You can get the audio buffer status by modifying (or duplicating) the following:
1) BufferFillWords(): replace the 0x5a7d write to SCI_WRAMADDR by 0x5a80.
2) BufferFreeWords(): replace 1023 with 4095.
3) BufferFreeWords(), replace 1024 with 4096.
http://www.vsdsp-forum.com/phpbb/viewtopic.php?t=1451
http://www.vsdsp-forum.com/phpbb/viewtopic.php?t=1463
*************************************************************************************************************/
main(void) {

	s_int16 auxBuffer[2*BLOCKSIZE];     // auxiliary audio buffer
	s_int16 __y lineInBuf[2*BLOCKSIZE]; // line-in audio buffer
	s_int16 __y *bufptr = delayBuffer;
	s_int16 __y *flgptr = delayBuffer;
	s_int16 phase = 0;                  // this is the (accumulator) phase for the LFO
	s_int16 flangerSample = 0;
	u_int16 flangerPhase = 0;
	// parameters for the currently selected effect:
		
	// need to enable interrupts when executed as a plug-in
	Enable();
	// avoid being called again (when executed as a plug-in)	
	applAddr = NULL;
	
	effect.delay += (effect.depth/2)+1;       // depth/2 added to delay
	// initialize delay line with silence
	memsetY(delayBuffer, 0, DELAY_BUF_SZ);    // delayBuffer all = 0
	// disable interrupts during initialization
	Disable();
	// basic initialization phase in int.c
	InitAudioExample(MY_SAMPLERATE,USE_MIC,CORE_CLOCK_3X); // Check if can do CORE_CLOCK_4X 3.5X 4.5X 5X
	// adjust output samplerate
	// VS1053 has 18-bit stereo DAC. DAC samplerate can be adjusted by ROM function:
	// SetHardware(2, 48000U); Stereo, 48 kHz
	// Parameters for the function are number of output channels (1 or 2) and output sample
	// rate in Hz. Fine tuning for the DAC sample rate can be done with a 32-bit number.
	// The lower 16 bits are written to the register FREQCTLL and higher 4 bits to the register FREQCTLH.
	SetHardware(2/*stereo output*/,MY_SAMPLERATE/*DA sample rate*/);
	//
	USEX(INT_ENABLE)|=(1<<INT_EN_MODU)/*AD*/|(1<<INT_EN_DAC);
	// initialize audio_buffer read and write pointers
        //extern s_int16 __y audio buffer[]; 
        //volatile extern s_int16 __y * audio_wr_pointer;
        //volatile extern s_int16 __y * audio_rd_pointer;
        // To stop playing, just set the audio_rd_pointer to audio_wr_pointer so the audio buffer seems empty.
	audio_rd_pointer = audio_buffer;               // ROM defined output buffer
	audio_wr_pointer = audio_buffer + 2*BLOCKSIZE; // Two blocksize samples ahead

	// clear audio buffer (avoid unwanted noise in the beginning)
	memsetY(audio_buffer,0,AUDIO_BUFFER_SZ);
	
	// set up GPIO in board.h
        // LED_1 (1 << 4)  Led 1 is at GPIO4
        // CONFIGURE_LED_1 {GPIO_CONFIGURE_AS_OUTPUT(LED_1);} 
        // SET_LED_1_ON GPIO_SET_HIGH(LED_1)
        // SET_LED_1_OFF GPIO_SET_LOW(LED_1)
	CONFIGURE_LED_1; 
        // BUTTON_1 (1 << 3) /* Button 1 is at GPIO3 */
        // BUTTON_1_PRESSED GPIO_IS_HIGH(BUTTON_1)
        // CONFIGURE_BUTTON_1 {GPIO_CONFIGURE_AS_INPUT(BUTTON_1);}
	CONFIGURE_BUTTON_1;
	
	// set almost max volume
	USEX(SCI_VOL) = 0x0101;
	
	//USEX(SCI_BASS) = 0xfa7a; // a=> 10khz a => 100 hz f = full +15db 7 => +10db
	//USEX(SCI_BASS) = 0x0f0f; // off

	// enable interrupts
	Enable();	
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main loop
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
	while (1) {

	// for testing...
        // if (effect.delay == 8000) SET_LED_1_OFF;
        // if (effect.delay == 40) SET_LED_1_ON;
       
		// wait until there is enough audio data to process
		// _StreamDiff=18491:NULL in rom1053b.txt/.o
		// The audio input (microphone or line in) in asm.s (interrupt handler mymodu_int) pushes audio 
                // samples to the stream buffer
		// The effect.c code gets the audio samples from the stream buffer, does some DSP on them and then 
                // pushes the data to AudioOutputSamples function.
		// StreamDiff() tells how many 16-bit words (samples) there are in the stream buffer.
		// The ROM function StreamDiff() returns the number of 16-bit samples in stream buffer
		// If the samples are not read fast enough, the buffer overflows and returns to empty state. 
                // Highest possible value is 1023 (buffer full)
		if (StreamDiff() > BLOCKSIZE*2) { // x 2 because stereo => enough to process
			u_int16 i;
			s_int16 temp = 0;
			u_int32 fractDelay;
			__y s_int16 *lp = lineInBuf;
			s_int16 *sp = auxBuffer;
			//
			// read input samples (stereo, hence 2 x block size)
			//
			StreamBufferReadData(lineInBuf, 2*BLOCKSIZE);
		
			// Check Button on GPIO3 pressed
			if (BUTTON_1_PRESSED) 
                           { effect.mix = 0;
                             NoEffect = 0; 
			     SET_LED_1_OFF;
			   } else { effect.mix = effect.mixOrg;
                                    NoEffect = 1;
				    SET_LED_1_ON;
			          }
			
                 	// process BLOCKSIZE samples at one go
			for (i = 0; i < BLOCKSIZE; i++) {
				// fractional delay part
				s_int16 flangerOut = (s_int16)(((s_int32)*flgptr * (65535U-flangerPhase)) >> 16);
				//
				// advance flgptr (and wrap the cyclic buffer)
				//
				if (flgptr >= delayBuffer+DELAY_BUF_SZ-1) {
					flgptr = delayBuffer;
                                        loopVar1 = 1;
				} else {
					flgptr++;
                                        loopVar1 = 0;
				}
				//
				flangerOut += (s_int16)(((s_int32)*flgptr * flangerPhase) >> 16);
			        ///////////////////////////////////////////////////////////
				// mix dry and effect and send to output (mix) 
                                ///////////////////////////////////////////////////////////
				*sp = (s_int16)(((s_int32)lp[0] * (32000U-effect.mix) + 
						(s_int32)flangerOut * effect.mix) >> 15);
                                ///////////////////////////////////////////////////////////
                                // add input sample to buffer with feedback (decay)
                                // 
                                ///////////////////////////////////////////////////////////
				*bufptr = (s_int16)(((s_int32)*sp++ * effect.decay + 
						(s_int32)lp[0] * (32000U-effect.decay)) >> 15);
				lp += 2;
				sp[0] = sp[-1]; // duplicate output to both channels
				sp++;
                                ///////////////////////////////////////////////////////////
				// advance bufptr (and wrap the cyclic buffer)
                                ///////////////////////////////////////////////////////////
				if (bufptr	>= delayBuffer+DELAY_BUF_SZ-1) {
					bufptr = delayBuffer;
                                        loopVar2 = 1;
				} else {
					bufptr++;
                                        loopVar2 = 0;
				}
			}
			//
			// LFO (speed & depth)
			//
			phase += effect.speed; 
			//
			// LFO value varies from 0 to 1024 (depth = max 2048)
			{
			u_int32 tt = (u_int32)((u_int16)abs(phase)) * effect.depth;
			flangerSample = (u_int16)(tt >> 16);
			flangerPhase = (u_int16)tt;
			}
			//
			// flanger delay
			//
			flgptr = bufptr + flangerSample - effect.delay;
			if (flgptr < delayBuffer) {
				flgptr += DELAY_BUF_SZ;
                                loopVar0 = 1;
			} else loopVar0 = 0;
			//
			// ouput sample pairs
			//
			AudioOutputSamples(auxBuffer, BLOCKSIZE);

		} else { // StreamDiff() <= BLOCKSIZE*2 do nothing wait for input
                        ///////////////////////////////////////////////////////////
			// limit values even if adjusted in real-time
                        ///////////////////////////////////////////////////////////
			if (effect.depth > 2048)  { effect.depth = 2048; }
			if (effect.speed > 1024)  { effect.speed = 1024; }
			if (effect.mix > 32000)   { effect.mix = 32000;	 }
			if (effect.decay > 32000) { effect.decay = 32000;}
		       } 
	} // while(1)
	return 0;
   
}
/*
void ChangeEffect()
{  
   effect.depth  = p1;
   effect.speed  = p2;
   effect.delay  = p3;
   effect.decay  = p4;
   effect.mix    = p5;
   effect.mixOrg = p6;
   if (EffectType == 0) effect.mix = 0;
   if (EffectType < 4) EffectType++; else EffectType = 0;
 
}
*/