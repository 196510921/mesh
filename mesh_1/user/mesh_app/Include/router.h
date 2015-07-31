#ifndef _ROUTER_H_
#define _ROUTER_H_
#include "boards.h"

enum{
	LEVEL_0  = 0,
	LEVEL_1,
	LEVEL_2,
	LEVEL_3,
	LEVEL_4,
	LEVEL_5,
	LEVEL_6,
	LEVEL_7,
	LEVEL_8,
	LEVEL_9,
	LEVEL_10,
};

/**
	*beacon packet
	*/
typedef uint8_t  beacon_pkt_t;
/**
	*router responce to node for which request of joining in network
	*/
typedef uint16_t router_responce_t;

void tx_data(void);
void rx_data(uint8_t * pdu);
void mac_addr_set(uint8_t* mac_addr);
void root_set(uint8_t level);
void level_set(uint8_t level);
/**
	*node received root's packet callback
	*/
void node_on_receive_cb(void);
void node_send_data(uint8_t* node_data, uint8_t data_len);
/**
	*root received node's packet callback
	*/
void root_on_receive_cb(void);
#endif //_ROUTER_H_
