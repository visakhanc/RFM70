/*
 *  RFM70 Rx Low Power example
 *	
 * 		RFM70 is configured in Rx mode.  When a packet is received by RFM70, AVR MCU reads it
 *		and inspects the packet. If proper, blinks an LED. After this, AVR goes to power down mode, 
 *		only to be woken up by external IRQ, which happens when next packet arrives.
 */

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "rfm70.h"

/*************** DEFINES & MACROS *******************/
#define RED_LED					PC1
#define RED_LED_DDR				DDRC
#define RED_LED_PORT			PORTC

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))


/******************** VARIABLES **********************/
static uint8_t rx_buf[CONFIG_RFM70_STATIC_PL_LENGTH];
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;


/************** FUNCTION DECLARATIONS ***************/
static void Receive_Packet(void);


/******************************************************
 *					MAIN FUNTION
 ******************************************************/
int main(void)
{	
	_delay_ms(1000); /* Some power on delay */
	rfm70_init(RFM70_MODE_PRX, addr); /* Initialize RFM70 module in receive mode */
	RED_LED_OUT();
	RED_LED_OFF();
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
	sei();   /* Enable interrupts globally */
	sleep_mode();  /* Go to sleep until a packet comes */
	
	while(1)
	{
		Receive_Packet();	/* After waking up, get the received packet */
		sleep_mode();	/* Go to sleep again */
	}
}



static void Receive_Packet(void)
{
	uint8_t i;
	uint8_t len;
	uint8_t chksum = 0; 

	rfm70_receive_packet(rx_buf, &len);
	if(0 == len) { /* No packet received? */
		return;
	}
	/* Check received packet */
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
