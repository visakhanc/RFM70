#include "spi.h"
#include "RFM73.h"
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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
const UINT8 tx_buf[17]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};
UINT8 rx_buf[MAX_PACKET_LEN];


int main(void)
{	
	power_on_delay();  
	timer0_init();
	RFM73_Initialize();
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
	if(count > 30)
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
	UINT8 i;
	UINT8 temp_buf[32];

	if(flag_1s == true)
	{
		flag_1s = false;
		
		for(i=0;i<17;i++)
		{
			temp_buf[i]=tx_buf[i];
		}
		
		RED_LED_ON();
		RFM73_Send_Packet(W_TX_PAYLOAD_NOACK_CMD,temp_buf,17);	
		_delay_ms(50);
		RED_LED_OFF();
	}
}



