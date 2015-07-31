#include "mesh_baseband.h"
#include "router.h"
#include <string.h>
#include <stdio.h>

extern volatile uint8_t node_enable;
extern uint8_t mesh_start_sign;
static uint8_t ble_disable_sign;
extern uint32_t counter;
uint8_t conn_state;
uint8_t connRole;
uint16_t radio_events;
uint16_t radio_times = 0;
static uint8_t tx_sign = 9;

static uint8_t ble_disable_sign;

static uint8_t ble_mode,mesh_mode;
static uint8_t channel;
static uint8_t radio_rx_data[39];
static uint16_t tx_count,rx_count;

static void periph_radio_config(void);
static void periph_timer_config(void);
static void periph_ppi_config(void);
static void periph_timer1_config(void);

/**
	*BLE
	*/
static uint8_t ble_adv_data[24] = {0x40, 0x15 ,0x00,0xE3 ,0xAD ,0x42, 0xD9, 0x2E, 0xE8,0x0B,0x09,0x4A,0x55,0x4D,0x41,0x5F, 0x42, 0x4F,0x41, 0x52, 0x44, 0x02, 0x01, 0x04};
//static uint8_t ble_adv_data[39] = {0x40,0x25,0x00,0x0E,0x22,0x31,0x60,0x9A,0xFA,0x02, 0x01, 0x04, 0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15, 0x8C, 0xF9, 0x95, 0xB6, 0xEE, 0x95, 0x22, 0xBC, 0xF8, 0x21, 0x32, 0x03, 0x51, 0x44, 0x58, 0x31, 0x05, 0x3E, 0xE3, 0xC7, 0xC3};
static uint8_t ble_scanReq_data[15] = {0x43,0x0C,0x00,0x02,0x03,0x04,0x05,0x06,0x9D,0x8C,0x49,0xE5,0xFF,0xD2};
static uint8_t ble_rx_data[37];


/**
 * @brief Function for INITIATING PERIPHERAL
 */
void baseband_init(void)
{
	periph_ppi_config();
	periph_radio_config();
	periph_timer_config();
	periph_timer1_config();
	ble_mode = 1;
	mesh_mode = 0;

}
/**
 * @brief Function for SETUP RADIO.
 */
static void periph_radio_config(void){
	///config radio
	periph_radio_setup();
	///config shortcut mode
	if(mesh_mode == 1){
		periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
															| RADIO_SHORTS_DISABLED_RXEN_Msk
		
															);//| RADIO_SHORTS_READY_START_Msk
	}else{
		periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
															| RADIO_SHORTS_READY_START_Msk
															| RADIO_SHORTS_DISABLED_RXEN_Msk
															);//
	}
		///config radio interrupt
		periph_radio_intenset(RADIO_INTENSET_DISABLED_Msk);
		periph_radio_intenset(RADIO_INTENSET_READY_Msk);
		periph_radio_intenset(RADIO_INTENSET_ADDRESS_Msk);
		periph_radio_intenset(RADIO_INTENSET_PAYLOAD_Msk);
		periph_radio_intenset(RADIO_INTENSET_END_Msk);
	
	if(mesh_mode == 1){
		tx_data();
		bb_setConnState(RADIO_TX_MODE);
		PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_TXEN);
	}else{
		bb_advGenericStart();
		periph_radio_packet_ptr_set(&ble_adv_data[0]);
		bb_setConnState(BLE_TX_MODE);
		PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_TXEN);
	}
	
}
/**
 * @brief Function for SETUP TIMER.
 */
static void periph_timer_config(void){
	NRF_TIMER0->TASKS_CLEAR = 1;
	///timer
	NRF_TIMER0->EVENTS_COMPARE[0] = 0;
	NRF_TIMER0->EVENTS_COMPARE[1] = 0;
	NRF_TIMER0->EVENTS_COMPARE[2] = 0;
	NRF_TIMER0->EVENTS_COMPARE[3] = 0;
	///timer interrupt
	NRF_TIMER0->INTENSET = (1 << (TIMER_INTENSET_COMPARE0_Pos));
	NRF_TIMER0->INTENSET = (1 << (TIMER_INTENSET_COMPARE1_Pos));
	NRF_TIMER0->INTENSET = (1 << (TIMER_INTENSET_COMPARE2_Pos));
	NRF_TIMER0->INTENSET = (1 << (TIMER_INTENSET_COMPARE3_Pos));
	NVIC_EnableIRQ(TIMER0_IRQn);
	NRF_TIMER0->TASKS_START = 1;
	
}

static void periph_timer1_config(void){
	NRF_TIMER0->TASKS_CLEAR = 1;
	///timer
	NRF_TIMER1->EVENTS_COMPARE[0] = 0;
	NRF_TIMER1->EVENTS_COMPARE[1] = 0;
	NRF_TIMER1->EVENTS_COMPARE[2] = 0;
	NRF_TIMER1->EVENTS_COMPARE[3] = 0;
	///timer interrupt
	NRF_TIMER1->INTENSET = (1 << (TIMER_INTENSET_COMPARE0_Pos));
	NRF_TIMER1->INTENSET = (1 << (TIMER_INTENSET_COMPARE1_Pos));
	NRF_TIMER1->INTENSET = (1 << (TIMER_INTENSET_COMPARE2_Pos));
	NRF_TIMER1->INTENSET = (1 << (TIMER_INTENSET_COMPARE3_Pos));
	NVIC_EnableIRQ(TIMER1_IRQn);
	
		capture_timer(0);
		capture_timer(1);
		set_cc_value(0,500);
		//radio_times = 2500+50;
		//set_cc_value(1,4600);
	
}
/**
 * @brief Function for setup PPI.
 */
static void periph_ppi_config(void)
{
	periph_ppi_set(26, &(NRF_TIMER0->TASKS_CAPTURE[1]), &(NRF_RADIO->EVENTS_ADDRESS));
	periph_ppi_set(2, &(NRF_RADIO->TASKS_START), &(NRF_TIMER0->EVENTS_COMPARE[0]));
	periph_ppi_set(1, &(NRF_RADIO->TASKS_START), &(NRF_TIMER1->EVENTS_COMPARE[1]));
}
/**
 * @brief Function for RADIO IRQ.
 */
void RADIO_IRQHandler(void)
{
	///disable event
	if(NRF_RADIO->EVENTS_DISABLED == 1){
		NRF_RADIO->EVENTS_DISABLED = 0;
		
		if(mesh_mode == 1){
			
			/*if(radio_events == 100){
				bb_setConnState(RADIO_TX_MODE);
				tx_data();
				tx_count++;
				radio_events = 0;
				//random_num();
			}else{
				bb_setConnState(RADIO_RX_MODE);
				periph_radio_packet_ptr_set(&radio_rx_data[0]);
				rx_count++;
			}*/
			if(radio_events == 8){
				ble_mesh_mode_switch(BLE_MODE);
				bb_fsm->onSleep();
				tx_count++;
				bb_setConnState(BLE_RX_MODE);
				radio_events = 0;
			}
		}else{
			if(bb_getConnState() == BLE_TX_MODE){
				if(bb_getConnRole() == SLAVE_STATE){
					if(counter == 5){
						NRF_TIMER1->TASKS_START = 1;
					}
				}
				if(ble_mode){
					bb_fsm->onSleep();
					tx_count++;
					bb_setConnState(BLE_RX_MODE);
				}
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
		
	}
	/// ready event
	if(NRF_RADIO->EVENTS_READY == 1){
			NRF_RADIO->EVENTS_READY = 0;
		if(mesh_mode == 1){
		
			if(radio_events < 8){
				
				periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
																/*| RADIO_SHORTS_DISABLED_TXEN_Msk*/
																		
															 );//| RADIO_SHORTS_READY_START_Msk
			}else{
				periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
																/*| RADIO_SHORTS_DISABLED_RXEN_Msk*/
																	
															 );//| RADIO_SHORTS_READY_START_Msk
			}
		}else{
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
		
	}
	/// address event
	if(NRF_RADIO->EVENTS_ADDRESS == 1){
			NRF_RADIO->EVENTS_ADDRESS = 0;
			if(mesh_mode == 1){//#if MESH_MODE
				nrf_gpio_pin_toggle(15);
				radio_set_slot();	
			}else{//#else
				if(bb_getConnState() == BLE_RX_MODE){
					bb_fsm->onRxAA();
					
				}else{
					if(bb_getConnRole() == SLAVE_STATE){

					}
				}
		
			}//#endif
			
	}
	///payload event
	if(NRF_RADIO->EVENTS_PAYLOAD == 1){
			NRF_RADIO->EVENTS_PAYLOAD = 0;
			if(mesh_mode == 1){//#if MESH_MODE
				rx_data((uint8_t*)radio_rx_data);
				//radio_set_slot();
		
			}else{//#else
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
		}//#endif
				
	}
	/// end event
	if(NRF_RADIO->EVENTS_END == 1){
		NRF_RADIO->EVENTS_END = 0;
		
	}
}
/**
 * @brief Function for TIMER IRQ.
 */
void TIMER0_IRQHandler(void){
	///timer0_0
	if(NRF_TIMER0->EVENTS_COMPARE[0] == 1){
		NRF_TIMER0->EVENTS_COMPARE[0] = 0;
			nrf_gpio_pin_toggle(13);
	}
	///timer0_1
	if(NRF_TIMER0->EVENTS_COMPARE[1] == 1){
		NRF_TIMER0->EVENTS_COMPARE[1] = 0;
	
	}
	///timer0_2
	if(NRF_TIMER0->EVENTS_COMPARE[2] == 1){
		NRF_TIMER0->EVENTS_COMPARE[2] = 0;
	
			if(bb_getConnRole() == SLAVE_STATE){
				slave_set_ch();
			}
			if(bb_getConnRole() == ADV_STATE){
				adv_set_ch();
			}
	
	}
	///timer0_3
	if(NRF_TIMER0->EVENTS_COMPARE[3] == 1){
		NRF_TIMER0->EVENTS_COMPARE[3] = 0;
			
			if(bb_getConnRole() == SLAVE_STATE){
				//ble_mesh_mode_switch(MESH_MODE);
				//NRF_RADIO->TASKS_DISABLE = 1;
				set_interval();
				bb_fsm->onWakeup();
				ble_disable_sign = 1;
				
			}
			if(bb_getConnRole() == ADV_STATE){
				NRF_RADIO->TASKS_DISABLE = 1;

			}
			
	}
}
/*timer1*/
void TIMER1_IRQHandler(void){
	///timer1_0
	if(NRF_TIMER1->EVENTS_COMPARE[0] == 1){
		NRF_TIMER1->EVENTS_COMPARE[0] = 0;
		NRF_TIMER1->TASKS_CAPTURE[0] = 1;
			nrf_gpio_pin_toggle(14);
			if(node_enable == 1){
			//	radio_events++;
			}
			ble_mesh_mode_switch(MESH_MODE);
		
			radio_set_slot();
	}
	///timer1_1
	if(NRF_TIMER1->EVENTS_COMPARE[1] == 1){
		NRF_TIMER1->EVENTS_COMPARE[1] = 0;
		radio_events++;
		nrf_gpio_pin_toggle(12);
	}
	///timer1_2
	if(NRF_TIMER1->EVENTS_COMPARE[2] == 1){
		NRF_TIMER1->EVENTS_COMPARE[2] = 0;
			radio_set_ch();
			if(node_enable == 1){
				tx_data();
				//bb_setConnState(RADIO_TX_MODE);
				PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_TXEN);
			}else{
				//bb_setConnState(RADIO_RX_MODE);
				PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_RXEN);
				periph_radio_packet_ptr_set(&radio_rx_data[0]);
			}
	}
	///timer1_3
	if(NRF_TIMER1->EVENTS_COMPARE[3] == 1){
		NRF_TIMER1->EVENTS_COMPARE[3] = 0;
		if(mesh_mode == 1){//#if MESH_MODE
			NRF_RADIO->TASKS_DISABLE = 1;
			//set_cc_value(1,2500);
			radio_set_slot();
			
		}
			
	}
}
/**
 *@brief get current BLE STATE
 */
uint8_t bb_getConnState(void)
{
    return conn_state;
}
/**
 *@brief set next BLE STATE
 */
void bb_setConnState(uint8_t state)
{
    conn_state = state;
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

/**
 *@brief capture current timer 
 */
void capture_timer(uint8_t timer)
{
	switch (timer){
		case 0:
			NRF_TIMER1->TASKS_CAPTURE[0] = 1;
			break;
		case 1:
			NRF_TIMER1->TASKS_CAPTURE[1] = 1;
			break;
	}
	
}
/**
 *@brief set capture register value
 */
void set_cc_value(uint8_t timer,uint16_t time)
{
	switch (timer){
		case 0:
			NRF_TIMER1->CC[0] = (NRF_TIMER1->CC[0]+time)%65536;
			break;
		case 1:
			NRF_TIMER1->CC[1] = (NRF_TIMER1->CC[1]+time)%65536;
			break;
	}
	
}

void ble_mesh_mode_switch(uint8_t mode)
{
	if(mode == BLE_MODE){
		ble_mode = 1;
		mesh_mode = 0;
	}else{
		mesh_mode = 1;
		ble_mode = 0;
	}
}

