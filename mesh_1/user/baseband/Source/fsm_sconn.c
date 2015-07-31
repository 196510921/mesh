#include "mesh_baseband.h"
#include "ts_peripheral.h"
#include "simple_uart.h"
#include "slave_dispatch.h"

extern uint32_t counter;
uint16_t ble_start_record;
static uint8_t crcInit[3];
static uint8_t AA[4];
static uint8_t winSize;
static uint16_t winOffset;
uint16_t interval, latency, timeout;
static uint8_t chM[5];
static uint8_t chM_new[5];
static uint8_t pre_ch;
static uint8_t hop, sca;
static uint16_t connEvents;
static uint16_t ch_instant = 0xFFFF;
uint8_t current_ch[40] = {0};

static uint8_t i;
static uint8_t ch_update_sign;
static uint8_t supervision_timeout;

static uint8_t ch, unmapped_ch;
static uint8_t remapping_table[37];
static uint8_t num_used_channels;
static uint8_t numUnUseCh;
static uint8_t s_tx_data[39] = {0};
static uint8_t s_rx_data[39] = {0};
static uint8_t pdu_type;
static uint8_t s_state;

static uint8_t start_ble_sign;

//static void apply_connPara(uint8_t * pdu, uint8_t len);
static void sconn_onNumUsedCh(void);
static void sconn_onWakeup(void);
static void sconn_onRxAA(void);
static void sconn_onRxPDU( void);
static void sconn_onTxStarted( void );
static void sconn_onSleep( void );

static const bb_fsm_t sconnFsm = {
    1,
    NULL,
    sconn_onWakeup,
    sconn_onRxAA,
    sconn_onRxPDU,
    sconn_onTxStarted,
    sconn_onSleep
};
void bb_sconnStart(uint8_t* pdu, uint8_t len, uint32_t hfclk)
{		
		pdu_type = pdu[0] &0x0F;
		bb_fsm = &sconnFsm;
		bb_setConnRole(CONN_REQ);
		apply_connPara( pdu, len );
		//NRF_TIMER0->CC[0]+= (198 - 10);
		//periph_ppi_set(26, &(NRF_TIMER0->TASKS_CAPTURE[1]), &(NRF_RADIO->EVENTS_ADDRESS));
		periph_radio_packet_ptr_set(&s_rx_data[0]);
    /*the number of used channel*/
    sconn_onNumUsedCh();
	
    radio_setCrcInit(crcInit);
    radio_setAccessCode(AA);
		
		 // setup next channel
    unmapped_ch += hop;
    unmapped_ch %= 37;
    if (chM[unmapped_ch/8] &( 1 << (unmapped_ch%8))) {
        ch = unmapped_ch;
    } else {
        ch = remapping_table[unmapped_ch % num_used_channels];
    }
		if(ch <= 10){
    	NRF_RADIO->FREQUENCY = 4 + (ch*2);
			NRF_RADIO->DATAWHITEIV = ch;
		}else{
			NRF_RADIO->FREQUENCY = 6 + (ch*2);
			NRF_RADIO->DATAWHITEIV = ch;
		}
		current_ch[i] = ch;
		i++;
		//PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_RXEN);
    //NRF_RADIO->FREQUENCY = ch;
}

static void sconn_onWakeup(void)
{
	
	if(bb_getConnRole() == CONN_REQ){
		
		NRF_TIMER0->CC[0]=(NRF_TIMER0->CC[1] + (1250 + (winOffset*1250)-50))%65536;
		NRF_TIMER0->CC[2]= (NRF_TIMER0->CC[1]+(1250 + (winOffset*1250)+(interval*1250) -1000))%65536;
		
	}
	if(bb_getConnRole() == SLAVE_STATE){
			NRF_TIMER0->CC[0]=(NRF_TIMER0->CC[1] +(interval*1250)-44 )%65536;
			NRF_TIMER0->CC[2]=(NRF_TIMER0->CC[1] +(interval*1250) - 2000)%65536 ;
			NRF_TIMER0->CC[3]=(NRF_TIMER0->CC[1] +(interval*1250)+2000)%65536;
			//NRF_TIMER1->INTENSET = (1 << (TIMER_INTENCLR_COMPARE3_Pos));
		
	}
}

static void sconn_onTxStarted( void )
{
	periph_radio_packet_ptr_set(&s_tx_data[0]);
	
}

static void sconn_onRxAA(void)
{
	supervision_timeout = 0;
}

static void sconn_onRxPDU(void)
{
	 s_opcodeCheck((uint8_t*)&s_rx_data[0], s_rx_data[1] + 3);
}

static void sconn_onSleep( void )
{
	periph_radio_packet_ptr_set(&s_rx_data[0]);
	
	//set_ch();
}

/*apply connection request pdu*/
void apply_connPara(uint8_t * pdu, uint8_t len)
{
		 uint8_t* lldata;
    /*
      CONNECT_REQ PDU Format
      InitA (6 octets)
      AdvA (6 octets)
      LLData (22 octets)

      LLData

      AA (4 octets)
      CRCInit (3 octets)
      WinSize (1 octet)
      WinOffset (2 octets)
      Interval (2 octets)
      Latency (2 octets)
      Timeout (2 octets)
      ChM (5 octets)
      Hop (5 bits)
      SCA (3 bits)

    */
		//uart_put_rx_data((uint8_t*)&pdu[0],len);
		 lldata = pdu + 15;
		
    // byte 0
    memcpy(AA, lldata, 4);
		//uart_put_rx_data(AA,4);
    // byte 4
    memcpy(crcInit, lldata + 4, 3);
		//uart_put_rx_data(crcInit,3);
    // byte 7
    winSize = lldata[7];
    // byte 8
    winOffset = lldata[8] + (lldata[9] << 8);
    // byte 10
    interval = lldata[10] + (lldata[11] << 8);
    // byte 12
    latency = lldata[12] + (lldata[13] << 8);
    // byte 14
    timeout = lldata[14] + (lldata[15] << 8);

    memcpy(chM, lldata + 16, 5);
		
		 hop = lldata[21] & 0x1F;
		 sca = lldata[21] >> 5;
		
}

/*the number of used channel*/
static void sconn_onNumUsedCh(void)
{
		uint8_t i;
		unmapped_ch = 0;
    num_used_channels = 0;    
    for (i=0; i<37; i++) {
        if (chM[i/8] & (1 << (i%8))) {
            remapping_table[num_used_channels] = i;
            num_used_channels++;
        } else {
            numUnUseCh = i;
        }	
    }
}

void slave_set_ch(void)
{
	if(ch_update_sign != 1){
	if(connEvents < 0xFFFF){
		connEvents++;	
		}else{
			connEvents = 0;
		}
		
		unmapped_ch += hop;
    unmapped_ch %= 37;
    if (chM[unmapped_ch/8] &( 1 << (unmapped_ch%8))) {
        ch = unmapped_ch;
    } else {
        ch = remapping_table[unmapped_ch % num_used_channels];
    }
    	if(ch <= 10){
    	NRF_RADIO->FREQUENCY = 4 + (ch*2);
			NRF_RADIO->DATAWHITEIV = ch;
		}else{
			NRF_RADIO->FREQUENCY = 6 + (ch*2);
			NRF_RADIO->DATAWHITEIV = ch;
		}
	
		PERIPHERAL_TASK_TRIGGER(NRF_RADIO->TASKS_RXEN);
		NRF_TIMER0->CC[2]+= 100;
		ch_update_sign = 1;
	}else{
		new_ch_map_update();
		ch_update_sign = 0;
	}
}

void set_interval(void)
{
		supervision_timeout++;
		if(supervision_timeout == 8){
			//adv_start();
			//supervision_timeout = 0;
			//NVIC_SystemReset();
		}
		NRF_TIMER0->CC[1] = ((NRF_TIMER0->CC[0]+30) % 65536);
		
}

void set_pdu(uint8_t * pdu, uint8_t len)
{
		memcpy((uint8_t*)&s_tx_data[0], pdu, len);
}

void ch_update(uint8_t* ch_new, uint16_t instant)
{
	 if(((instant - connEvents)% 65536) < 32767){
		memcpy(chM_new, ch_new + 4, 5);
		ch_instant = instant;
	 }else{
		memcpy(chM, ch_new + 4, 5);
		pre_ch = unmapped_ch; 
		sconn_onNumUsedCh();
		unmapped_ch = pre_ch;
		
	 }
}

void new_ch_map_update(void)
{
	if(connEvents == ch_instant)
		{
			memcpy(chM, chM_new, 5);
			pre_ch = unmapped_ch; 
			sconn_onNumUsedCh();
			unmapped_ch = pre_ch;
		}	
}