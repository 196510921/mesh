#ifndef _BASEBAND_H_
#define _BASEBAND_H_
#include "advertise.h"
#include <string.h>
#include <stdio.h>
#include "boards.h"

#define ADV_STATE							1
#define SCAN_STATE						3
#define CONN_REQ						  5
#define MASTER_STATE					7
#define SLAVE_STATE						9
#define SCAN_RSP							11

#define BB_PDU_TYPE_ADV_IND            0x0
#define BB_PDU_TYPE_ADV_DIRECT_IND     0x1
#define BB_PDU_TYPE_ADV_NONCONN_IND    0x2
#define BB_PDU_TYPE_ADV_SCAN_REQ       0x3
#define BB_PDU_TYPE_ADV_SCAN_RSP       0x4
#define BB_PDU_TYPE_ADV_CONN_REQ       0x5
#define BB_PDU_TYPE_ADV_SCAN_IND       0x6


#define BB_ADV_CH_PDU_TYPE_MASK     0x0F

#define BB_RX_AA_MATCH   0x01

typedef struct {
    uint8_t state;
    void (*onStart) (void);
    void (*onWakeup) (void);
    void (*onRxAA) (void);
    void (*onRxPDU) (void);
    void (*onTxStarted) (void);
    void (*onSleep) (void);
} bb_fsm_t;

typedef struct _pdu_hdr_t{
    uint8_t  type;
    uint8_t  len;
} pdu_hdr_t;

extern const bb_fsm_t* bb_fsm;

#define bb_onStart()                            \
    bb_fsm->onStart()

#define bb_onWakeup()               \
        bb_fsm->onWakeup();         

#define bb_onRxAA()                        \
    bb_fsm->onRxAA()

#define bb_onRxPDU()          \
    bb_fsm->onRxPDU()

#define bb_onTxStarted()                        \
    bb_fsm->onTxStarted()

#define bb_onSleep()              \
        bb_fsm->onSleep();        

#define bb_getState()                        \
    bb_fsm->state

extern uint8_t connState;
uint8_t bb_getConnState(void);
void bb_setConnState(uint8_t state);

extern  uint8_t connRole;
uint8_t bb_getConnRole(void);
void bb_setConnRole(uint8_t state);
//extern uint8_t bleEvents;
void bb_advGenericStart( void );
void apply_connPara(uint8_t * pdu, uint8_t len);
void bb_sconnStart(uint8_t* pdu, uint8_t len, uint32_t hfclk);
void slave_set_ch(void);
void adv_set_ch(void);
void set_interval(void);
void set_pdu(uint8_t * pdu, uint8_t len);
/// channel update
void ch_update(uint8_t* ch_new, uint16_t instant);
void new_ch_map_update(void);

#endif //_BASEBAND_H_
