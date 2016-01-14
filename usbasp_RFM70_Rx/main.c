#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "RFM73.h"
#include "usbdrv.h"

#define RED_LED					PC0
#define RED_LED_DDR				DDRC
#define RED_LED_PORT			PORTC

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))

void timer0_init(void);
void power_on_delay(void);
void Receive_Packet(void);

volatile bool flag_1s;
static UINT8 rx_buf[MAX_PACKET_LEN];



usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	uint8_t len;
	
	if(data[1] == 1) { /* Send data in rx_buf */
		usbMsgPtr = rx_buf;
		len = sizeof(rx_buf);
	}
	
	return len;
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

/* Check if a packet is received and get the packet to rx buffer */
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
	if(chksum==rx_buf[16])
	{
		/* Packet received correctly */
		RED_LED_ON();
		_delay_ms(50);
		RED_LED_OFF();
		
		/* Send Interrupt packet to host, for synchronization */
		usbSetInterrupt((void *)rx_buf, 1);
	}
}




int main(void)
{	
	uint8_t 	i;
	
	power_on_delay();  
	RFM73_Initialize();
	RED_LED_OUT();
	RED_LED_OFF();
	
	usbInit();
	usbDeviceDisconnect();  /* Disconnect for some time to enforce reenumeration */
	i = 0;
	while(--i) {
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();   // enable interrupts globally
	
	while(1)
	{
		usbPoll();
		Receive_Packet();
	}
}

