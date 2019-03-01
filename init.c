#include <string.h>
#include <vs1053.h>

// InitAudioExample
//		- a basic initialization sequence for VS1063/VS1053 audio processing applications
//		- sets core clock frequency, UART and A/D parameters
//
void InitAudioExample(u_int16 srInput,int useMicIn,u_int16 coreClock) {
	// avoid being called again (when executed as a plug-in)
	applAddr = NULL;
	// set core clock
	USEX(SCI_CLOCKF) = coreClock; // CORE_CLOCK_*
	//
	// perform essential audio setup
	InitHardware();
	//		
	// disable analog powerdown
	USEX(SCI_STATUS) &= ~((1<<SCIST_APDOWN1) | (1<<SCIST_APDOWN2));
	//
	if (useMicIn) {
		// enable microphone amplification
		// (left channel = mono mic-in)
		USEX(SCI_MODE) &= ~(1<<SCIMB_LINE);
	} else {
		// disable microphone amplification
		USEX(SCI_MODE) |= (1<<SCIMB_LINE);
	}
	// set UART speed for serial port communication
	uartByteSpeed = 11520U;                                            // Try lowering this for effect on noise
	USEX(UART_DIV) = UartDiv();
	//
	// set A/D parameters
	//
	memcpyXY(agcConsts, agcConstsInit, sizeof(agcConsts));
	memset(&adcControl, 0, sizeof(adcControl));
	USEX(SCI_AICTRL1) = 1024; // Autogain == 0, 1X == 1024, 0.5X == 512 etc.
	USEX(SCI_AICTRL3) = (1<<2); // set linear pcm mode, joint-stereo

	if (useMicIn) {
		USEX(SCI_AICTRL3) |= 2; /* channel selector: left channel only (mic-in) */
	}
	/* autogain max volume default 0 = 64x */
	if (USEX(SCI_AICTRL2) == 0) {
		USEX(SCI_AICTRL2) = 65535U;
	}
	adcControl.agc[0].gain = -32767;
	adcControl.agc[1].gain = -32767;
	adcControl.adcMode = USEX(SCI_AICTRL3) & 3;
	if (srInput == 48000U) {
		USEX(DECIM_CONTROL) = DECIM_ENABLE | DECIM_FACTOR48K;
	} else if (srInput == 24000U) {
		USEX(DECIM_CONTROL) = DECIM_ENABLE | DECIM_FACTOR24K;
	} else if (srInput == 12000U) {
		USEX(DECIM_CONTROL) = DECIM_ENABLE | DECIM_FACTOR24K;
		USEX(SCI_STATUS) |= (1<<SCIST_AD_CLOCK); // halve A/D clock from 6 MHz to 3 MHz
	}
	//
	// initialize stream (A/D) buffer
	// __x indicates X RAM used
	stream_rd_pointer = (u_int16 __x *)stream_buffer;
	stream_wr_pointer = stream_rd_pointer;
	stream_rd_odd = 0; // word alignment
}
