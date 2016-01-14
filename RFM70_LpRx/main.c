/*
 *  RFM73 Rx example
 *	
 * 		RFM73 is configured in Rx mode.  When a packet is received by RFM73, AVR MCU reads it
 *		and inspects the packet. If is proper, blinks an LED. Aftern this, AVR goes to power down mode, 
 *		only to be waoken up by external IRQ, which happens when a packet arrives.
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



void Receive_Packet(void);

UINT8 rx_buf[MAX_PACKET_LEN];


int main(void)
{	
	_delay_ms(1000); //power_on_delay
	RFM73_Initialize();
	RED_LED_OUT();
	RED_LED_OFF();
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();   // enable interrupts globally
	sleep_mode();  // go to sleep
	
	while(1)
	{
		Receive_Packet();
		sleep_mode();
	}
}



void Receive_Packet(void)
{
	UINT8 i, len, chksum = 0; 
	
	RFM73_Receive_Packet(rx_buf, &len);
	
	for(i=0;i<16;i++)
	{
		chksum +=rx_buf[i]; 
	}
	if(chksum==rx_buf[16]&&rx_buf[0]==0x30)
	{
		/* Packet received correctly */
		RED_LED_ON();
		_delay_ms(50);
		RED_LED_OFF();
	}
}
