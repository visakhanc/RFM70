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


static void avr_init(void);

volatile bool flag_1s;
static uint8_t tx_buf[17]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x78};
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;

int main(void)
{	
	uint8_t len;
	uint8_t ack[10];
	
	avr_init();	
	while(1)
	{
		if(flag_1s == true)
		{
			flag_1s = false;		
			if(0 == rfm70_transmit_packet(tx_buf, sizeof(tx_buf))) {
				rfm70_receive_packet(ack, &len);
				if(len && (ack[0] == 0x45)) {
					RED_LED_ON();
					_delay_ms(50);
					RED_LED_OFF();
				}
			}
			//_delay_ms(200);
			//RED_LED_ON();
			//_delay_ms(50);
			//RED_LED_OFF();
		}
	}
}

static void avr_init(void)
{
	RED_LED_OUT();
	RED_LED_ON();
	_delay_ms(500);  /* startup delay */
	RED_LED_OFF();
	
	TCCR0 = 0x5; 		/* Start timer0 at frequecy = 8MHz/1024 = 128us */
	TIMSK |= (1 << 0); 	/* Enable Interrupt for timer0 overflow */ 
	
	rfm70_init(RFM70_MODE_PTX, addr);	
	sei();   // enable interrupts globally	
}



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






