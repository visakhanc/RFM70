/* 
 *  rfm70_config.h
 *
 *	This file defines the configurations to be used for RFM70 module 
 *	Modify these definitions according to your project
 */

/******** I/O PIN DEFINITIONS FOR AVR *********/

/* Define which AVR pin is connected to RFM70 Chip Enable (CE) */
#define CE_DDR		DDRB
#define CE_PORT		PORTB
#define CE_PIN   	1

/****************** POLLED/INTERRUPT MODE *******************/
/* Define to 0 if interrupt mode is used. In this case AVR INT1 
   pin should be connected to RFM70 IRQ pin
   Define to 1 if polled mode is used. In this case RFM70 IRQ pin
   is not connected to AVR
*/
#define RFM70_POLLED_MODE 	0
