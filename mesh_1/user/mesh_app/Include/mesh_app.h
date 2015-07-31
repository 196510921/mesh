#ifndef _MESH_APP_H_
#define _MESH_APP_H_

#include "mesh_baseband.h"
void led_blink(uint8_t *attValue, uint8_t dataLen);
void app_tx_data(uint8_t attOpcode, uint16_t attHandle, uint8_t pdu_type);

void app_test(void);
#endif //_MESH_APP_H_