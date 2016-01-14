#include "spi.h"
#include "RFM73.h"
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB
*/

#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))

void timer0_init(void);
void power_on_delay(void);
void Receive_Packet(void);

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
		Receive_Packet();
	}
}


/*********************************************************
Function: timer2_init();                                         
                                                            
Description:                                                
	initialize timer. 
*********************************************************/
void timer0_init(void)
{
	TCCR0 = 0x5; 		/* Start timer0 at frequecy of 8MHz/1024 = 128us */
	TIMSK |= (1 << 0); 	/* Enable Interrupt for timer0 overflow */ 
}

/*********************************************************
Function:  interrupt ISR_timer()                                        
                                                            
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


void Receive_Packet(void)
{
	UINT8 i, len, chksum = 0; 
	
	RFM73_Receive_Packet(rx_buf, &len);
	if(len == 0) { /* No packet received */
		return;
	}
	
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
