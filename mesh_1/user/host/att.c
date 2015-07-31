#include "att.h"
#include "mesh_app.h"

static const uint16_t chanId = 4;
static pdu_hdr_t hdr;
static ll_ver_in_t ll_ver_in;
static uint8_t tx_pdu[28];
static uint8_t notify_en;
static uint8_t ver_sign;

static void att_notFd(uint8_t pdu_type, uint8_t attOpcode, uint16_t attHd );

///empty packet
void att_empPk(uint8_t pdu_type)
{
    /// LLID: 1
    tx_pdu[0] = (pdu_type & 0x04) ^ 0x04 | (((pdu_type>>2)&0x1) << 3) | 0x01;//header
    tx_pdu[1] = 0;//len
    set_pdu(tx_pdu, tx_pdu[1]+3);
}

///ll version information
void s_llVersion(uint8_t* pdu, uint8_t pdu_type)
{
    //uint8_t _attData1[6] = {0x0c, 0x07, 0x59, 0x00, 0x4F, 0x20};
    //uint8_t* attData = NULL;
    
		//record version information
		ll_ver_in.verNr = pdu[3];
		ll_ver_in.CompId = pdu[4] + (pdu[5] << 8);
		ll_ver_in.SubVersNr = pdu[6] + (pdu[7] << 8);
	
		tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3) | 0x02;
		tx_pdu[1] = 6;
		tx_pdu[2] = 0x00;
		tx_pdu[3] = 0x0C;
		tx_pdu[4] = 0x07;
		tx_pdu[5] = 0x59;
		tx_pdu[6] = 0x00;
		tx_pdu[7] = 0x64;
		tx_pdu[8] = 0x00;

		//attData = _attData1;
		set_pdu(tx_pdu, tx_pdu[1]+3);
}

///ll feature req
void s_llFea(uint8_t* pdu, uint8_t pdu_type)
{
    tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
		tx_pdu[1] = 9;
		tx_pdu[2] = 0x00;
		tx_pdu[3] = 0x09;
		tx_pdu[4] = 0x01;
		tx_pdu[5] = 0x00;
		tx_pdu[6] = 0x00;
		tx_pdu[7] = 0x00;
		tx_pdu[8] = 0x00;
		tx_pdu[9] = 0x00;
		tx_pdu[10] = 0x00;
		tx_pdu[11] = 0x00;
   set_pdu(tx_pdu, tx_pdu[1]+3);	
    
}

void att_exMtuReq( uint8_t pdu_type, uint8_t attOpcode, uint16_t clientMtu )
{   
    uint16_t l2cap_len = 3;
    //uint8_t _attData1[7]; // = {0x03,0x00,0x04,0x00,0x03,0x17,0x00};
    uint16_t mymtu = L2CAP_MTU_SIZE;
    
    //data header
    /*
    * LLID : 2, NESSN : 1, SN : 1, MD : 0, PDU-Length : 7 
    */
    tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
    tx_pdu[1] = 7;

    tx_pdu[2] = l2cap_len >> 8;
		tx_pdu[3] = l2cap_len & 0xFF;
    tx_pdu[4] = chanId >> 8;
    tx_pdu[5] = chanId & 0xFF;
		tx_pdu[6] = 0x00;
    tx_pdu[7] = ATT_EXCHANGE_MTU_RSP;
    tx_pdu[8] = mymtu & 0xFF;
    //tx_pdu[9] = mymtu >> 8;
    
    set_pdu(tx_pdu, tx_pdu[1]+3);

}

///read by group type request handle
void att_server_rdByGrType( uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd, uint16_t att_type )
{	
    uint16_t att_hd; //service handle
    uint8_t len; // = 4 + sizeof(ser_uuid)*8; // att data list len
    uint16_t l2cap_len; // = 2 + len;
    uint8_t base_uuid[16] = {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                        0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E};
    uint8_t i;
    
    if(att_type == GATT_PRIMARY_SERVICE_UUID){
        for(att_hd=st_hd; att_hd < END_HD; att_hd++){
		    if(PRIMARY_SERVICE_HD == att_hd){
				///pdu
				len = 4 + 16; // att data list len
				l2cap_len = 2 + len;
					
				 tx_pdu[2] = l2cap_len >> 8;
				 tx_pdu[3] = l2cap_len & 0xFF;
				 tx_pdu[4] = chanId >> 8;
				 tx_pdu[5] = chanId & 0xFF;
				 tx_pdu[6] = 0x00;
				 tx_pdu[7] = ATT_RD_BY_GROUP_TYPE_RSP;
				 tx_pdu[8] = len;
				 tx_pdu[9] = att_hd & 0xFF;
				 tx_pdu[10] = att_hd >> 8;
				 tx_pdu[11] = SEC_CHAR_HD & 0xFF;
				 tx_pdu[12] = SEC_CHAR_HD >> 8;
					
                for(i = 0; i < 16; i++){
                    tx_pdu[13+i] = base_uuid[i];
                }
                tx_pdu[13+12] = PRIMARY_SERVICE_UUID & 0xFF;
                tx_pdu[13+13] = PRIMARY_SERVICE_UUID >> 8;
    
				//data header
				/*
				 * LLID : 2, NESSN : 1, SN : 0, MD : 0, PDU-Length : 18 
				 */
                tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
                tx_pdu[1] = 26;
								
                //to l2cap
                set_pdu(tx_pdu, tx_pdu[1]+3);
                return;
            }
        }
    }
		
     ///error handle
			
					att_notFd( pdu_type, attOpcode, st_hd );
}

///read by type resquest handle
void att_server_rdByType(uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd, uint16_t att_type)
{
    uint8_t len;
    uint16_t l2cap_len;
    uint8_t i;
    uint8_t base_uuid[16] = {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9,0xE0,
                        0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E};

    uint16_t hd;	
    uint8_t cha_num;
		
    for(hd = st_hd; hd <= end_hd; hd++){
        for(cha_num=0; cha_num<ATT_CHAR_NUM; cha_num++){
            if((GATT_CHARACTER_UUID == att_type) && (FIRST_CHAR_DECL_HD == hd)){
                len = 5 + 16;
                l2cap_len = 2 + len;
								
                tx_pdu[2] = l2cap_len >> 8;  
                tx_pdu[3] = l2cap_len & 0xFF;
                tx_pdu[4] = chanId >> 8;
                tx_pdu[5] = chanId & 0xFF;
								tx_pdu[6] = 0x00;
                tx_pdu[7] = ATT_RD_BY_TYPE_RSP;
                tx_pdu[8] = len;	
                tx_pdu[9] = hd & 0xFF;// char declaration handle
                tx_pdu[10] = hd >> 8;
                tx_pdu[11] =  ATT_CHAR_PROP_W_NORSP ; // | ATT_CHAR_PROP_W |ATT_CHAR_PROP_RD  |   ATT_CHAR_PROP_NTF;
                tx_pdu[12] = FIRST_CHAR_HD & 0xFF; //attribute value handle
                tx_pdu[13] = FIRST_CHAR_HD >> 8;

                //0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                //0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E
                for(i = 0; i < 16; i++){
                    tx_pdu[14+i] = base_uuid[i];
                }
                tx_pdu[14+12] = FIRST_CHAR_UUID & 0xFF;;// attribute value uuid
                tx_pdu[14+13] = FIRST_CHAR_UUID >> 8;
                    
                //data header
                /*
                 * LLID : 2, NESSN : 1, SN : 0, MD : 0, PDU-Length : 18 
                 */
                tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
                tx_pdu[1] = 27;
            
                set_pdu(tx_pdu, tx_pdu[1]+3);       
                return;
            }
						
						if((GATT_CHARACTER_UUID == att_type) && (SEC_CHAR_DECL_HD == hd)){
                len = 5 + 16;
                l2cap_len = 2 + len;
								
                tx_pdu[2] = l2cap_len >> 8;  
                tx_pdu[3] = l2cap_len & 0xFF;
                tx_pdu[4] = chanId >> 8;
                tx_pdu[5] = chanId & 0xFF;
								tx_pdu[6] = 0x00;
                tx_pdu[7] = ATT_RD_BY_TYPE_RSP;
                tx_pdu[8] = len;	
                tx_pdu[9] = hd & 0xFF;// char declaration handle
                tx_pdu[10] = hd >> 8;
                tx_pdu[11] = ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF;; //| ATT_CHAR_PROP_W_NORSP | ATT_CHAR_PROP_W 
                tx_pdu[12] = SEC_CHAR_HD & 0xFF; //attribute value handle
                tx_pdu[13] = SEC_CHAR_HD >> 8;

                //0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                //0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E
                for(i = 0; i < 16; i++){
                    tx_pdu[14+i] = base_uuid[i];
                }
                tx_pdu[14+12] = SEC_CHAR_UUID & 0xFF;;// attribute value uuid
                tx_pdu[14+13] = SEC_CHAR_UUID >> 8;
                    
                //data header
                /*
                 * LLID : 2, NESSN : 1, SN : 0, MD : 0, PDU-Length : 18 
                 */
                tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
                tx_pdu[1] = 27;
            
                set_pdu(tx_pdu, tx_pdu[1]+3);       
                return;
            }
        }
    }
    
    att_notFd( pdu_type, attOpcode, st_hd );/// error handle								
}


static void att_notFd(uint8_t pdu_type, uint8_t attOpcode, uint16_t attHd )
{
    uint16_t l2cap_len = 5; 
    uint8_t _errorhd[9]; // = {0x05, 0x00, 0x04, 0x00, 0x01, 0x08, 0x08, 0x00, 0x0A}; ///error code ATT_NOT_FOUND
		
		//data header
		/*
		 * LLID : 2, NESSN : 1, SN : 0, MD : 0, PDU-Length : 9 
		 */
    tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
    tx_pdu[1] = 9;
    
		//pdu
		/*
		 *l2cap-length:0x0005, chanId:0x0004, opcode:0x01, ReqOpCode: 0x08, 
		 AttHandle:0x0008, ErrorCode: ATT_NOT_FOUND(0x0A) 
		 */
		tx_pdu[2] = l2cap_len >> 8;
		tx_pdu[3] = l2cap_len & 0xFF;
    tx_pdu[4] = chanId >> 8;
    tx_pdu[5] = chanId & 0xFF;
		tx_pdu[6] = 0x00;
    tx_pdu[7] = ATT_ERROR_RSP;	//opcode 0x01
    tx_pdu[8] = attOpcode;
    /// responce content : 'start handle'
    tx_pdu[9] = attHd & 0xFF;
    tx_pdu[10] = attHd >> 8;
    tx_pdu[11] = ATT_ERR_ATTR_NOT_FOUND; 
    
    //to l2cap
    set_pdu(tx_pdu, tx_pdu[1]+3);
}

void att_server_findIn( uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd )
{
  //uint16_t counter;
	uint16_t l2cap_len = 6;
	uint8_t _attData1[10];
		
    //for(counter = st_hd; counter <= end_hd; counter++){
	    if(st_hd < END_HD ){ //// value exist
        tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
        tx_pdu[1] = 10;
        tx_pdu[2] = l2cap_len >> 8;
        tx_pdu[3] = l2cap_len & 0xFF;
        tx_pdu[4] = chanId >> 8;
        tx_pdu[5] = chanId & 0xFF;
			  tx_pdu[6] = 0x00;
        tx_pdu[7] = ATT_FIND_IN_RSP;
        tx_pdu[8] = 0x01;
			  tx_pdu[9] =SEC_CHAR_DECL_HD & 0xFF;	          //// handle
			  tx_pdu[10] = SEC_CHAR_DECL_HD >> 8;		
				tx_pdu[11] = GATT_CHARACTER_UUID & 0xFF; ////value
				tx_pdu[12] = GATT_CHARACTER_UUID >> 8;			
        
            //to l2cap						
        set_pdu(tx_pdu, tx_pdu[1]+3);				
				return;
		}
	//}

    ///error handle
    att_notFd(pdu_type, attOpcode, st_hd);
}

void ser_write_no_rsp(uint8_t pdu_type, uint8_t attOpcode, uint16_t att_hd, uint8_t* attValue, uint8_t valueLen_w)
{		
		if(att_hd == FIRST_CHAR_HD){
			led_blink( attValue, valueLen_w);
			//server_dataBuf(attValue, valueLen_w, pdu_type);
			//add_rx_data(att_hd, attValue, valueLen_w);
			att_empPk( pdu_type);
		}else{
			///error handle
			att_notFd( pdu_type, attOpcode, att_hd );			
		}		
}


///read request handle
void att_server_rd( uint8_t pdu_type, uint8_t attOpcode, uint16_t att_hd, uint8_t* attValue, uint8_t datalen )
{
    uint16_t l2cap_len;
    uint8_t counter;
        
		//data header
		/*
		 * LLID : 2, NESN : 1, SN : 0, MD : 0, PDU-Length :  
		 */
    tx_pdu[0]  = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
		tx_pdu[1]  = 5 + datalen; //l2cap_length + chanid + opcode + datalen
        
		//pdu
		///l2cap_length + chanid + opcode + data
    l2cap_len = 1 + datalen; 
		tx_pdu[2]  = l2cap_len >> 8;
		tx_pdu[3]  = l2cap_len & 0xFF;
		tx_pdu[4]  = chanId >> 8;
		tx_pdu[5]  = chanId & 0xFF;
		tx_pdu[6] = 0x00;
    tx_pdu[7]  = ATT_RD_RSP;
    for(counter = 0; counter < datalen; counter++){
         tx_pdu[8+counter] = attValue[counter];
    }
        
		//to l2cap
		set_pdu(tx_pdu, tx_pdu[1]+3);		

}

//// read response
void server_rd_rsp(uint8_t attOpcode, uint16_t attHandle, uint8_t pdu_type)
{
    ///write request
    uint8_t i;
    uint8_t pos;
	
    if(attHandle == SEC_CHAR_HD){
			app_tx_data( attOpcode, attHandle, pdu_type);
      return;
    }
		if(attHandle == 0x000C){
			 tx_pdu[0]  = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
		tx_pdu[1]  = 5 + 2; //l2cap_length + chanid + opcode + datalen
        
		//pdu
		///l2cap_length + chanid + opcode + data
    uint16_t l2cap_len = 1 + 2; 
		tx_pdu[2]  = l2cap_len >> 8;
		tx_pdu[3]  = l2cap_len & 0xFF;
		tx_pdu[4]  = chanId >> 8;
		tx_pdu[5]  = chanId & 0xFF;
		tx_pdu[6] = 0x00;
    tx_pdu[7]  = ATT_RD_RSP;
     tx_pdu[8]  = 0x00;
			 tx_pdu[9]  = 0x00;
        
		//to l2cap
		set_pdu(tx_pdu, tx_pdu[1]+3);		

		} 
    ///error handle
    att_notFd( pdu_type, attOpcode, attHandle );
          
}

///notify handle
void ser_notify_handle( uint8_t pdu_type, uint8_t attOpcode, uint16_t att_hd, uint8_t* attValue, uint8_t datalen )
{
    uint16_t l2cap_len = (3+datalen);
    uint8_t counter;
		//if(notify_en <= 0){
		//	att_empPk(pdu_type);
   // }else{	
        /*
         * LLID : 2, NESN : 1, SN : 0, MD : 0, PDU-Length :  
         */
        tx_pdu[0] = (pdu_type & 0x07) ^ 0x04 | (((pdu_type>>2)&0x1) << 3);
        tx_pdu[1] = 7 + datalen; //l2cap_length + chanid + opcode + handle + datalen
        
        //pdu
        ///l2cap_length + chanid + opcode + handle + data
        tx_pdu[2] = l2cap_len >> 8;					
        tx_pdu[3] = l2cap_len & 0xFF;
        tx_pdu[4] = chanId >> 8;
        tx_pdu[5] = chanId & 0xFF;
        tx_pdu[6] = 0x00;
				tx_pdu[7] = ATT_NOTIFY_HANDLE;
        tx_pdu[8] = att_hd & 0xFF;  //hd_end;
        tx_pdu[9] = att_hd >> 8;  //hd_pre;
        for(counter = 0; counter < datalen; counter++){
            tx_pdu[10+counter] = attValue[counter];
        }
        
        //to l2cap
        set_pdu(tx_pdu, tx_pdu[1]+3);	
	//	}
}



