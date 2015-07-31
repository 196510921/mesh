#include "mesh_baseband.h"
#include "ts_peripheral.h"
#include "simple_uart.h"
/* Store the radio channel */

const bb_fsm_t* bb_fsm;
static uint8_t channel;
static uint16_t tx_start;
uint8_t count_scan = 1;
static uint8_t scan_sign,adv_count;
static uint8_t adv_interval = 10;
//static uint8_t ble_adv_data[24] = {0x40, 0x15 ,0x00,0xE3, 0xAD, 0x42, 0xD9, 0x2E, 0xE8,0x0B,0x09,0x4A,0x55,0x4D,0x41,0x5F, 0x42, 0x4F,0x41, 0x52, 0x44, 0x02, 0x01, 0x04};
static uint8_t ble_adv_data[39] = {0x40,0x13,0x00,0x01,0x02,0x03,0x04,0x05,0x06,2, 1, 4, 0x09, 8, 'J','U','M','A','_','B','L','E'};
	//static uint8_t ble_adv_data[39] = {0x40,0x25,0x00,0x0E,0x22,0x31,0x60,0x9A,0xFA,0x02, 0x01, 0x04, 0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15, 0x8C, 0xF9, 0x95, 0xB6, 0xEE, 0x95, 0x22, 0xBC, 0xF8, 0x21, 0x32, 0x03, 0x51, 0x44, 0x58, 0x31, 0x05, 0x3E, 0xE3, 0xC7, 0xC3};
static uint8_t _ble_adv_data[23] = {0x40,0x13,0x00,0x02,0x02,0x02,0x02,0x02,0x02,2, 1, 4, 0x09, 8, 'J','U','M','A','_','B','L','E'};

//static uint8_t scan_rsp[9] = {0x44,0x06,0x00,0x01,0x02,0x03,0x04,0x05,0x06};
static uint8_t scan_rsp[9] = {0x44,0x06,0x00,0x01,0x02,0x03,0x04,0x05,0x06};
static uint8_t ble_rx_data[37];
	
static void adv_generic_onStart(void);
static void adv_generic_onWakeup(void);
static void adv_generic_onTxStarted(void);
static void adv_generic_onRxAA(void);
static void adv_generic_onRxPDU(void);
static void adv_generic_onSleep(void);

const bb_fsm_t adv_generic_fsm = {
    0,
    adv_generic_onStart,
    adv_generic_onWakeup,
    adv_generic_onRxAA,
    adv_generic_onRxPDU,
    adv_generic_onTxStarted,
    adv_generic_onSleep,
};

void bb_advGenericStart( void )
{
		//bb_setConnState( ADV_STATE );
    bb_fsm = &adv_generic_fsm;
    bb_fsm->onStart();
}

/*
 * Start adversiting
 */
static void adv_generic_onStart(void)
{
	 /*set channel*/
		bb_setConnRole(ADV_STATE);
		channel = 38;
		periph_radio_ch_set(channel);
		channel++; 
}

static void adv_generic_onWakeup(void)
{
	NRF_TIMER0->CC[3]=NRF_TIMER0->CC[1] +750;
	NRF_TIMER0->CC[0]=NRF_TIMER0->CC[1] +(adv_interval*1250);
	NRF_TIMER0->CC[2]=NRF_TIMER0->CC[1] +(adv_interval*1250) - 500; //- (adv_interval*500);
}


static void adv_generic_onTxStarted(void)
{
	if(count_scan == 0){
		//memcpy((uint8_t*)&ble_adv_data[0], _ble_adv_data, 22);
		periph_radio_packet_ptr_set((uint8_t*)&ble_adv_data[0]);
		count_scan = 0;
		
	}else{
		//memcpy((uint8_t*)&ble_adv_data[0], scan_rsp, 9);
		//periph_radio_packet_ptr_set((uint8_t*)&ble_adv_data[0]);
		periph_radio_packet_ptr_set((uint8_t*)&scan_rsp[0]);
		count_scan --;
	}
	if(adv_count < 100){
							adv_count++;
						}else{
							adv_count = 0;
							scan_sign = 0;
						}
	
	//periph_radio_packet_ptr_set(&ble_adv_data[0]);
	
}

static void adv_generic_onRxAA(void)
{
	uint8_t pdu_type = ble_rx_data[0] & 0x0F;
        switch (pdu_type) {
        case BB_PDU_TYPE_ADV_SCAN_REQ:
					if((ble_rx_data[9] == 0x01)&&(scan_sign < 5)){
							count_scan=1;
							NRF_TIMER0->CC[0]=NRF_TIMER0->CC[1] +280;
							NRF_TIMER0->CC[2]=NRF_TIMER0->CC[1] +200; //- (adv_interval*500);
							NRF_TIMER0->CC[3]=NRF_TIMER0->CC[1] +620;// + 500;
							scan_sign++;
					}
            break;
        case BB_PDU_TYPE_ADV_CONN_REQ:
						if(ble_rx_data[9] == 0x01){
		
							periph_radio_shorts_set(	RADIO_SHORTS_END_DISABLE_Msk  
																| RADIO_SHORTS_DISABLED_RXEN_Msk
																| RADIO_SHORTS_READY_START_Msk
																);//
							bb_sconnStart((uint8_t*)&ble_rx_data[0], ble_rx_data[1] + 3, NRF_TIMER0->CC[0]);
							
						}
            break;
					default:
						
						break;
        }        
}

static void adv_generic_onRxPDU()
{
		uint8_t pdu_type = ble_rx_data[0] & 0x0F;
		//uart_put_rx_data((uint8_t*)&ble_rx_data[0],ble_rx_data[1] + 3);
    switch (pdu_type) {
      case BB_PDU_TYPE_ADV_SCAN_REQ:
        
        break;
      case BB_PDU_TYPE_ADV_CONN_REQ:
				bb_sconnStart((uint8_t*)&ble_rx_data[0], ble_rx_data[1] + 3,NRF_TIMER0->CC[0]);
        break;
			default :
				break;
    }
		//adv_generic_onSleep();
}


static void adv_generic_onSleep(void)
{
	
	//radio_setMode(RADIO_MODE_ADV_TX);
	periph_radio_packet_ptr_set(&ble_rx_data[0]);
	
}

void adv_set_ch()
{
	/*if(channel < 40){
		periph_radio_ch_set(channel);
		channel++;
	}else{
		channel = 37;
		periph_radio_ch_set(channel);
		channel++;
	}*/
	//PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_TXEN);
}