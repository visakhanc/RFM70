/*
 *  RFM70 Tx example showing Low power usage
 *	
 * 		A sample packet is transmitted every 2 sec. When Transmission completes, AVR ATmega8 MCU goes to
 *		power-save mode, after setting RFM70 to power down mode. AVR wakes up every second, by overflow 
 *		interrupt of TIMER2 which runs in asynchronous mode by external 32.768kHz crystal.
 *		
 *		Auto-Acknowledgement feature is enabled in the transmitted packet. This means, the transmitting RFM70
 *		waits for acknowledgement from the receiver. If no acknowledgement is received, packet is retransmitted.
 * 		This is repeated for a number of times (5 times in this case), until acknowledgement is received.
 *		
 *		Approximate Current Consumption
 *		-------------------------------
 *		Low power state:
 *			RFM73	: 3 uA
 *			AVR		: 10 uA
 *		Active state:
 *			RFM73	: 17 mA (TX at 0dB)
 *			AVR		: 6 mA
 */


#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "rfm70.h"


/*************** DEFINES & MACROS *******************/
#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))


/******************** VARIABLES **********************/
volatile bool flag_2s = true;
uint8_t tx_buf[17]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;


/************** FUNCTION DECLARATIONS ***************/
void timer2_async_init(void);


int main(void)
{
	uint8_t result;
	uint8_t i;
	
	timer2_async_init();	/* Initialize Timer2 */
	rfm70_init(RFM70_MODE_PTX, addr);	/* Initialize RFM70 module in transmit mode */
	RED_LED_OUT();
	RED_LED_OFF();
	
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	sei();   /* Enable interrupts globally */
	sleep_mode();  /* Go to sleep */
	
	while(1)
	{
		if(flag_2s == true)
		{
			flag_2s = false;
			tx_buf[16] = 0;
			for(i = 0; i < 16; i++) {
				tx_buf[16] += tx_buf[i];
			}
			RED_LED_ON();
			rfm70_tx_mode();
			result = rfm70_transmit_packet(tx_buf,sizeof(tx_buf));  /* Transmit packet */
			rfm70_powerdown(); /* RFM70 to power down mode */
			if(result == 0) { /* ACK received */
				_delay_ms(50);
			}
			else {  /* ACK not received */
				_delay_ms(20);
				RED_LED_OFF();
				_delay_ms(100);
				RED_LED_ON();
				_delay_ms(20);
			}
			RED_LED_OFF();
		}
		sleep_mode(); /* AVR sleep mode */
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







	