#include "router.h"
#include "mesh_baseband.h"
#include "string.h"

static uint16_t radio_times_record;
uint8_t radio_tx_data[39] = {0x40,0x13,0x01,0x08,0x09,0x09,0x09,0x09,0x09,0x0A,0x03,0x01};
static uint8_t ble_adv_data[39] = {0x40,0x13,0x00,0x01,0x02,0x03,0x04,0x05,0x06,2, 1, 4, 0x09, 8, 'J','U','M','A','_','M','E','S','H'};
static beacon_pkt_t beaconPkt;
static router_responce_t routerRsp;
volatile uint8_t node_enable = 0;
volatile uint8_t root_role = 0;
extern  uint16_t radio_events;
extern uint8_t radio_app_rx_data[39];
extern uint16_t radio_times;
extern uint16_t interval;
uint8_t mesh_start_sign;
uint16_t mesh_start_timer_record;


/**
	*select and set root
	*/	
void root_set(uint8_t level)
{
	level_set(level);
	node_enable = 1;
	root_role = 1;
}
/**
	*set mac address
	*/
void mac_addr_set(uint8_t* mac_addr)
{
	memcpy(radio_tx_data+3, mac_addr, 6);
}
/**
	*set mac address
	*/
void level_set(uint8_t level)
{
	radio_tx_data[9] = level;
}
/**
	*node send data to root
	*/
void node_send_data(uint8_t* node_data, uint8_t data_len)
{
	memcpy(radio_tx_data+11, node_data, data_len);
}

void rx_data(uint8_t * pdu)
{
	/*
	 * type:pdu[0]
	 * len:pdu[1]
	 * group address:pdu[2]
	 * address:pdu[3]~pdu[8]
	 * level:pdu[9]
 	 * level_target:pdu[10]
	 * data: pdu+11
	 */
	/// on receiving first  beacon to change level
	if((pdu[9] < radio_tx_data[9])&&(pdu[3] == radio_tx_data[3])&&(pdu[4] == radio_tx_data[4])){
		if(node_enable == 0){
			radio_tx_data[9] = pdu[9] + 1;
			node_enable = 1;
			//radio_events = 7;
		}
		
		/*if(pdu[2] == radio_tx_data[2]){
			//memcpy(app_rx_data, pdu, pdu[1]);
			//node_on_receive_cb();//data
		}else*/
		if(pdu[10] == radio_tx_data[9]){
			if((radio_tx_data[3] == pdu[3])&&(radio_tx_data[4] == pdu[4])&&(radio_tx_data[5] == pdu[5])&&
						 (radio_tx_data[6] == pdu[6]) && (radio_tx_data[7] == pdu[7])&&(radio_tx_data[8] == pdu[8])){
				memcpy(radio_app_rx_data, pdu, pdu[1]);
				node_on_receive_cb();//data
			}
						 
		}else{
				pdu[9] = radio_tx_data[9];
				memcpy(radio_tx_data, pdu, 12);
		}
		
	}else if((pdu[10] == 0)&&(pdu[3] == radio_tx_data[3])&&(pdu[4] == radio_tx_data[4])){
		if(root_role == 1){
			memcpy(radio_app_rx_data, pdu, pdu[1]);
			root_on_receive_cb();//group address + mac address + level + data
		}else{
			pdu[9] = radio_tx_data[9];
			memcpy(radio_tx_data, pdu, 12);
		}
	}
		
}

void tx_data(void)
{
	//periph_radio_packet_ptr_set(&radio_tx_data[0]);
	periph_radio_packet_ptr_set(ble_adv_data);
}

void radio_set_slot(void)
{
	if(radio_events < 8){
		nrf_gpio_pin_toggle(16);
		if(radio_events == 0){
			nrf_gpio_pin_toggle(17);
			radio_times = (mesh_start_timer_record+3000)%65536;
			mesh_start_timer_record = (mesh_start_timer_record + interval*1250)%65536;
			NRF_TIMER1->CC[0] = mesh_start_timer_record;
		}else{
			radio_times = (radio_times + 2500)%65536;
		}
		
		NRF_TIMER1->CC[1]=radio_times -44;
		NRF_TIMER1->CC[2]=radio_times -500;
		NRF_TIMER1->CC[3]=radio_times +2000;
			
	}
	/*else{
		NRF_TIMER0->CC[0] =	ble_start_record%65536;
		NRF_TIMER0->CC[2] = (ble_start_record - 2000)%65536 ;
		NRF_TIMER0->CC[3] = (ble_start_record + 2000)%65536;	
		
	}*/
}

void radio_set_ch(void)
{
	NRF_RADIO->FREQUENCY 	 = 84;////2;					// Frequency bin 80, 2480MHz, channel 39.
  NRF_RADIO->DATAWHITEIV = 42;			
}
