
#include <string.h>
#include <stdio.h>
#include "nrf_gpio.h"

#include "advertise.h"
#include "baseband.h"
#include "simple_uart.h"
#include "ts_peripheral.h"

 uint8_t connState;
 uint8_t connRole ;


////
static uint16_t tx_len = 350;
static uint16_t rx_timeout = 0;
uint16_t start_time;
static uint16_t bleEvents ;
 uint16_t tx_count;
 uint16_t rx_count;
static uint8_t ble_events;
/*Tx/Rx state*/
static uint8_t sm_state;
static uint8_t pdu_type;
uint8_t time = 50;
//static uint8_t ble_adv_data[BLE_ADDR_OFFSET + BLE_ADDR_LEN + BLE_PAYLOAD_MAXLEN];
//static uint8_t ble_adv_data[39] = {0x40,0x24,0x00,0x01,0x02,0x03,0x04,0x05,0x06,2, 1, 4, 0x1a, 0xFF, 0x4c,0x00,0x02,0x15,0x8c,0xF9,0x97,0xA6,0xEE,0x94,0xE3,0xBC,0xF8,0x21,0xB2,0x60,0x51,0x44,0x58,0x31,'J','U','M','A',0xC3};
static uint8_t ble_adv_data[24] = {0x40, 0x15 ,0x00,0xE3 ,0xAD ,0x42, 0xD9, 0x2E, 0xE8,0x0B,0x09,0x4A,0x55,0x4D,0x41,0x5F, 0x42, 0x4F,0x41, 0x52, 0x44, 0x02, 0x01, 0x04};
//static uint8_t ble_adv_data[39] = {0x40,0x25,0x00,0x0E,0x22,0x31,0x60,0x9A,0xFA,0x02, 0x01, 0x04, 0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15, 0x8C, 0xF9, 0x95, 0xB6, 0xEE, 0x95, 0x22, 0xBC, 0xF8, 0x21, 0x32, 0x03, 0x51, 0x44, 0x58, 0x31, 0x05, 0x3E, 0xE3, 0xC7, 0xC3};
static uint8_t ble_scanReq_data[15] = {0x43,0x0C,0x00,0x02,0x03,0x04,0x05,0x06,0x9D,0x8C,0x49,0xE5,0xFF,0xD2};
static uint8_t ble_rx_data[37];
static void ble_evt_setup(void);
static void sm_exit_adv_send(void);
static void sm_enter_tx_send(void);
static void sm_enter_wait_for_idle(bool req_rx_accepted);

static void radio_setTime(uint16_t value);
	

void adv_start(void){
	
	ble_evt_setup();
	sm_enter_tx_send();
}

static void ble_evt_setup(void)
{
	/*set radio*/
	periph_radio_setup();
	
	periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
														| RADIO_SHORTS_READY_START_Msk
														| RADIO_SHORTS_DISABLED_RXEN_Msk
														);//
	//ppi
	//periph_ppi_set(1, &(NRF_TIMER0->TASKS_CAPTURE[0]), &(NRF_RADIO->EVENTS_DISABLED));
	periph_ppi_set(26, &(NRF_TIMER0->TASKS_CAPTURE[1]), &(NRF_RADIO->EVENTS_ADDRESS));
	periph_ppi_set(2, &(NRF_RADIO->TASKS_START), &(NRF_TIMER0->EVENTS_COMPARE[0]));
	//periph_ppi_set(3, &(NRF_RADIO->TASKS_DISABLE), &(NRF_TIMER0->EVENTS_COMPARE[1]));
	///timer0
	NRF_TIMER0->TASKS_CLEAR = 1;
	NRF_TIMER0->EVENTS_COMPARE[0] = 0;
	NRF_TIMER0->EVENTS_COMPARE[1] = 0;
	NRF_TIMER0->EVENTS_COMPARE[2] = 0;
	NRF_TIMER0->EVENTS_COMPARE[3] = 0;
	NRF_TIMER0->INTENSET = (1 << (TIMER_INTENSET_COMPARE0_Pos + 2));
	NRF_TIMER0->INTENSET = (1 << (TIMER_INTENSET_COMPARE0_Pos + 3));
	NVIC_EnableIRQ(TIMER0_IRQn);
	NRF_TIMER0->TASKS_START = 1;
	//| RADIO_SHORTS_DISABLED_TXEN_Msk
	/*set disable irq*/
	periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk);
	periph_radio_intenset(RADIO_INTENSET_READY_Msk);
	periph_radio_intenset(RADIO_INTENSET_ADDRESS_Msk);
	periph_radio_intenset(RADIO_INTENSET_PAYLOAD_Msk);
	periph_radio_intenset(RADIO_INTENSET_END_Msk);
	
}	

/**
	Tx data
*/
static void sm_enter_tx_send(void)
{
	bb_advGenericStart();
	periph_radio_packet_ptr_set(&ble_adv_data[0]);
	radio_setMode(RADIO_MODE_ADV_TX);
	
}

void RADIO_IRQHandler(void)
{
	if(NRF_RADIO->EVENTS_DISABLED == 1){
		NRF_RADIO->EVENTS_DISABLED = 0;
		if(bb_getConnState() == BLE_TX_MODE){
			bb_fsm->onSleep();
			tx_count++;
			bb_setConnState(BLE_RX_MODE);	
		}else {
			rx_count++;
			if((bb_getConnRole() == ADV_STATE) || (bb_getConnRole() == SLAVE_STATE)){
				bb_fsm->onTxStarted();
				bb_setConnState(BLE_TX_MODE);
			}
			if((bb_getConnRole() == CONN_REQ)){
				bb_setConnState(BLE_RX_MODE);
				bb_setConnRole(SLAVE_STATE);
			}
		}
	}
	
	if(NRF_RADIO->EVENTS_READY == 1){
			NRF_RADIO->EVENTS_READY = 0;
			if(bb_getConnState() == BLE_TX_MODE){
	
				if(bb_getConnRole()== ADV_STATE){
					periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
																	| RADIO_SHORTS_DISABLED_RXEN_Msk
																	| RADIO_SHORTS_READY_START_Msk
																	);////
				}
				if(bb_getConnRole()==SLAVE_STATE){
					periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
																	
																	);
				}
					
			}else{
					if(bb_getConnRole()==SLAVE_STATE){
						periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
																		| RADIO_SHORTS_DISABLED_TXEN_Msk
																		| RADIO_SHORTS_READY_START_Msk
																		);////
						//NRF_TIMER0->CC[3] = 0;
					}
					if(bb_getConnRole()== ADV_STATE){
						periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk
																			| RADIO_SHORTS_DISABLED_TXEN_Msk
																			//| RADIO_SHORTS_READY_START_Msk
																		);
					}
				
		}
	}
	
	if(NRF_RADIO->EVENTS_ADDRESS == 1){
			NRF_RADIO->EVENTS_ADDRESS = 0;
			if(bb_getConnState() == BLE_RX_MODE){
				bb_fsm->onRxAA();
			}
	}
	
	if(NRF_RADIO->EVENTS_PAYLOAD == 1){
			NRF_RADIO->EVENTS_PAYLOAD = 0;
			if(bb_getConnState() == BLE_RX_MODE){
				
				if((bb_getConnRole() == CONN_REQ)){
					bb_fsm->onWakeup();
				}
				if(bb_getConnRole() == SLAVE_STATE){
					bb_fsm->onWakeup();
					bb_fsm->onRxPDU();
				}
				
			}else{
				if(bb_getConnRole()==ADV_STATE){
					bb_fsm->onWakeup();
					
				}
			}
				
	}
	
	if(NRF_RADIO->EVENTS_END == 1){
		NRF_RADIO->EVENTS_END = 0;		
	}
}

void TIMER0_IRQHandler(void){
	/*if(NRF_TIMER0->EVENTS_COMPARE[0] == 1){
		NRF_TIMER0->EVENTS_COMPARE[0] = 0;
		if(bb_getConnRole() == SLAVE_STATE){
			set_interval();
		}
	}*/
	if(NRF_TIMER0->EVENTS_COMPARE[2] == 1){
		NRF_TIMER0->EVENTS_COMPARE[2] = 0;
		if(bb_getConnRole() == SLAVE_STATE){
			slave_set_ch();
		}
		if(bb_getConnRole() == ADV_STATE){
			adv_set_ch();
		}
	}
	if(NRF_TIMER0->EVENTS_COMPARE[3] == 1){
		NRF_TIMER0->EVENTS_COMPARE[3] = 0;
		if(bb_getConnRole() == SLAVE_STATE){
			set_interval();
			bb_fsm->onWakeup();
			
			}
		if(bb_getConnRole() == ADV_STATE){
			NRF_RADIO->TASKS_DISABLE = 1;
			
			//set_interval();
			//bb_fsm->onWakeup();
		}
	}
}

/* get current BLE state*/
uint8_t bb_getConnState(void)
{
    return connState;
}
/*set next BLE state*/
void bb_setConnState(uint8_t state)
{
    connState = state;
}

/* get current BLE state*/
uint8_t bb_getConnRole(void)
{
    return connRole;
}
/*set next BLE state*/
void bb_setConnRole(uint8_t Role)
{
    connRole = Role;
}

