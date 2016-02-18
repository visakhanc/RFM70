#include "rfm70.h"
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
static uint8_t rx_buf[CONFIG_RFM70_STATIC_PL_LENGTH];
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;
static uint8_t ack_pld[4] = {0x45, 0x46, 0x47, 0x48};
int main(void)
{	
	power_on_delay();  
 	timer0_init();
	rfm70_init(RFM70_MODE_PRX, addr);
	RED_LED_OUT();
	RED_LED_OFF();
	sei();   // enable interrupts globally
	
	rfm70_set_ack_payload(RFM70_PIPE0, ack_pld, sizeof(ack_pld));
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
	uint8_t i, len, chksum = 0; 
	
	rfm70_receive_packet(rx_buf, &len);
	if(len == 0) { /* No packet received */
		return;
	}
		
	for(i=0;i<16;i++)
	{
		chksum +=rx_buf[i]; 
	}
	if(chksum==rx_buf[16]&&rx_buf[0]==0x30)
	{	
		/* set ack payload */
		rfm70_set_ack_payload(RFM70_PIPE0, ack_pld, sizeof(ack_pld));
		
		/* Packet received correctly */
		RED_LED_ON();
		_delay_ms(50);
		RED_LED_OFF();
	}
}
