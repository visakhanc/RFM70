#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr_spi.h"
#include "rfm70.h"

#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))


void timer0_init(void);
void power_on_delay(void);
void sub_program_1hz(void);

volatile bool flag_1s;
static uint8_t tx_buf[8];
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;

int main(void)
{
	power_on_delay();
	timer0_init();
	rfm70_init(RFM70_MODE_PTX, addr);
	RED_LED_OUT();
	RED_LED_OFF();	
	sei();   // enable interrupts globally
	
	while(1)
	{
		sub_program_1hz();
	}
}


/*********************************************************
Function: timer0_init                                       
                                                            
Description:                                                
	initialize timer. 
*********************************************************/
void timer0_init(void)
{
	TCCR0 = 0x5; 		/* Start timer0 at frequecy of 8MHz/1024 = 128us */
	TIMSK |= (1 << 0); 	/* Enable Interrupt for timer0 overflow */ 
}


/*********************************************************
Function: Timer0 ISR                                        
                                                            
Description:                                                
 
*********************************************************/
ISR(TIMER0_OVF_vect)
{
 	static uint8_t count = 0;
	
	count++;
	if(count == 15)
	{
		count = 0;
       	flag_1s = true;
	}
}


/*********************************************************
Function:      power_on_delay()                                    
                                                            
Description:                                                
 
*********************************************************/
void power_on_delay(void)
{
	unsigned int i;
	for(i = 0; i<1000; i++)
	{
		_delay_ms(1);
	}
}



/*********************************************************
Function:  sub_program_1hz()                                        
                                                            
Description:                                                
 
*********************************************************/
void sub_program_1hz(void)
{
	uint8_t i, len;

	if(flag_1s == true)
	{
		flag_1s = false;
		
		tx_buf[0]++;
		tx_buf[7] = 0;
		for(i=0;i<7;i++)
		{
			tx_buf[7] += tx_buf[i];
		}
		
		//RED_LED_ON();
		if(0 == rfm70_transmit_packet(tx_buf,sizeof(tx_buf))) {
			_delay_ms(5);
			RED_LED_TOGGLE();
			_delay_ms(5);
		}
		//_delay_ms(5);
		//RED_LED_OFF();
	}
}



