#ifndef _ADVERTISE_H_
#define _ADVERTISE_H_
#include "boards.h"

#define RXRU 				1
#define RXIDLE 			2
#define RX 					3
#define RXDISABLE 	4
#define TXRU 				9
#define TXIDLE 			10
#define TX 				  11
#define TXDISABLE 	12


#define BLE_TYPE_OFFSET			(0)
#define BLE_SIZE_OFFSET 		(1)
#define BLE_ADDR_OFFSET 		(3)
#define BLE_PAYLOAD_OFFSET 	(9)
#define BLE_TXADD_OFFSET		(1)

#define BLE_ADDR_TYPE_MASK	(0x40)
#define BLE_TYPE_MASK				(0x3F)
#define BLE_ADDR_LEN				(6)
#define BLE_PAYLOAD_MAXLEN	(31)

#define BLE_TX_MODE						2
#define BLE_RX_MODE						1


void adv_start(void);
/*tx/tx state*/

/**/
void radio_setTxTime(uint16_t len);
void radio_setStartTime(uint16_t value,uint8_t i);
void radio_anchorStartTime(uint16_t tx_start);
#endif //_ADVERTISE_H_
