#include "spi.h"
#include "RFM73.h"
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))


void timer2_async_init(void);
void power_on_delay(void);
void sub_program_1hz(void);

volatile bool flag_2s;
UINT8 tx_buf[17]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};
UINT8 rx_buf[MAX_PACKET_LEN];


int main(void)
{
	power_on_delay();  
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
			
			RED_LED_ON();
			RFM73_Send_Packet(W_TX_PAYLOAD_NOACK_CMD,tx_buf,17);
			SwitchToPowerDownMode(); // set RFM73 to power down mode
			_delay_ms(50);
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
Function: Timer0 ISR                                      
                                                            
Description:                                                
 
*********************************************************/
ISR(TIMER2_OVF_vect)
{
 	static uint8_t count = 0;

	count++;
	if(count & 0x01)
		flag_2s = true;
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





