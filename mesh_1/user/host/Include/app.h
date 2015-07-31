#ifndef _APP_H_
#define _APP_H_
#include "baseband.h"
void led_blink(uint8_t *attValue, uint8_t dataLen);
void app_tx_data(uint8_t attOpcode, uint16_t attHandle, uint8_t pdu_type);

#endif //_APP_H_