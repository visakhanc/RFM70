/*
 *  RFM73 Tx example showing Low power usage
 *	
 * 		A sample packet is transmitted every 2 sec. When Transmission completes, AVR ATmega8 MCU goes to
 *		power-save mode, after setting RFM73 to power down mode. AVR wakes up every second, by overflow 
 *		interrupt of TIMER2 which runs in asynchronous mode by external 32.768kHz crystal.
 *		
 *		Auto-Acknowledgement feature is enabled in the transmitted packet. This means, the transmitting RFM73
 *		waits for acknowledgement from the receiver. If no acknowledgement is received, packet is retransmitted.
 * 		This is repeated for a number of timers (5 times in this case), until acknowledgement is received.
 *
 *		Low power state:
			RFM73	: 3 uA
			AVR		: 10 uA
		Active state:
			RFM73	: 17 mA (TX at 0dB)
			AVR		: 6 mA
 */


#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "RFM73.h"

#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))


void timer2_async_init(void);

volatile bool flag_2s = true;
UINT8 tx_buf[17]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};


int main(void)
{
	uint8_t result, count = 0, i;
	
	//_delay_ms(1000); // power_on_delay  
	timer2_async_init();
	RFM73_Initialize();
	RED_LED_OUT();
	RED_LED_OFF();
	
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	sei();   // enable interrupts globally
	
	sleep_mode();  // go to sleep
	
	while(1)
	{
		if(flag_2s == true)
		{
			flag_2s = false;
			count++;
			tx_buf[0] = count;
			tx_buf[16] = 0;
			for(i = 0; i < 16; i++) {
				tx_buf[16] += tx_buf[i];
			}
			RED_LED_ON();
			//RFM73_Send_Packet(W_TX_PAYLOAD_NOACK_CMD,tx_buf,17);
			result = RFM73_Send_Packet(WR_TX_PLOAD,tx_buf,17);  // with ACK enabled
			SwitchToPowerDownMode(); // set RFM73 to power down mode
			if(result == 0) {
				_delay_ms(50);
			}
			else {  // ack not received
				_delay_ms(20);
				RED_LED_OFF();
				_delay_ms(100);
				RED_LED_ON();
				_delay_ms(20);
			}
			RED_LED_OFF();
		}
		
		sleep_mode();
	}
}



/* Starts Timer2 with asynchronous clock */
void timer2_async_init(void) 
{                                
    _delay_ms(2000);	//for crystal to become stable
	
    TIMSK &= ~((1<<TOIE2)|(1<<OCIE2));     //Disable TC0 interrupt
    ASSR |= (1<<AS2);           //set Timer/Counter2 to be asynchronous from the CPU clock 
                                //with a second external clock(32,768kHz)driving it.  
    TCNT2 = 0x00;
    TCCR2 = 0x05;                 //prescale the timer to be clock source / 128 to make it
                                //exactly 1 second for every overflow to occur
    while(ASSR&0x07);           //Wait until TC0 is updated
    TIMSK |= (1<<TOIE2);        //set 8-bit Timer/Counter0 Overflow Interrupt Enable                             
}



/*********************************************************
Function: Timer2 ISR                                      
                                                            
Description:                                                
 
*********************************************************/
ISR(TIMER2_OVF_vect)
{
 	static uint8_t count = 0;

	count++;
	if(count & 0x01)
		flag_2s = true;
}







	