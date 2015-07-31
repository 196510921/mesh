#include "slave_dispatch.h"
#include "att.h"

static uint8_t i;
static uint8_t l2cap_pdu[27] = {0};
static uint8_t l2cap_len[1] = {0};
static uint8_t ser_pdu_n[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; 
static uint8_t dataHdrP;

static att_incl_desc att_incl;
static uint16_t attHandle;
static uint16_t clientMtu;
static uint8_t llid;
static uint8_t llCoOpcode;
static uint8_t attOpcode;

uint32_t counter;

void s_opcodeCheck( uint8_t * pdu, uint8_t len )
{
    uint8_t* attValue = NULL;
		uint8_t valueLen_w;
		uint16_t instantP[1] = {0};
    uint16_t instantChm[1] = {0};
		
    //memset(l2cap_pdu,0,sizeof(l2cap_pdu));
		memcpy(l2cap_pdu, pdu, len);
    dataHdrP	= l2cap_pdu[0]; ///data header 8bit
    llid = l2cap_pdu[0] & 0x03; ///get LLID
    
		
    //printf("**pdu_type[%d]**", pdu_type);
    if(llid == 0x01){  //data
      ////  server_notify(dataHdrP);
			if(counter == 3){
				s_llVersion( l2cap_pdu, dataHdrP);
				counter++;
			}else if(counter > 30){
					if(i < 20){
						ser_pdu_n[i] +=1;
						i++;
					}else{
						i = 0;
					}
          ser_notify_handle(dataHdrP+1, attOpcode, SEC_CHAR_HD, ser_pdu_n,20 );
          counter++;
       }else if(counter >200){
					counter = 7;
					counter++;
			 }else{
          att_empPk(dataHdrP);
          counter++;
       }
			 /*
			if(counter >200){
			counter = 4;
			}
			counter++;
			if(counter == 3){
				s_llVersion( l2cap_pdu, dataHdrP);
			}else{
				att_empPk(dataHdrP);
			}*/
    }else if(llid == 0x03){  
			//att_empPk(dataHdrP);
      //llCoOpcode = l2cap_pdu[2];	/// ll control opcode
			llCoOpcode = l2cap_pdu[3];	/// ll control opcode
      ///ll control pdu 
      switch(llCoOpcode){
				case LL_VERSION :
					//if(ver_count == 1){
					//	s_llVersion( l2cap_pdu, dataHdrP);
					//}else{
					//	att_empPk( dataHdrP );
					//	ver_count = 1;
					//}
					
          ///empty packet
            att_empPk( dataHdrP ); 
				break;
        case LL_TERMINATE_IND : 
					att_empPk( dataHdrP );
          ////  s_llTer(l2cap_pdu, dataHdrP);
        break;
        case LL_CONN_UPDATE_REQ :
					att_empPk( dataHdrP );
          //update connection parameter to master
          instantP[0] = l2cap_pdu[12] + (l2cap_pdu[13] << 8);
          ////  connPaInstant(instantP);
          ////  connUpdatePtr(l2cap_pdu, l2cap_len[0]);
				  //printf(".recv LL_CONN_UPDATE_REQ.");
        break;
        case LL_CH_MAP_REQ :
						//att_empPk( dataHdrP );
          //update connection parameter to master
             ch_update(l2cap_pdu, l2cap_pdu[9] + (l2cap_pdu[10] << 8));// ch + instant
						 att_empPk( dataHdrP );
          ////   instantChm[0] = l2cap_pdu[6] + (l2cap_pdu[7] << 8);
          ////   connPaInstant(instantChm);
          ////   connUpdateChm(l2cap_pdu, l2cap_len[0]);
				  //printf(".recv LL_CH_MAP_REQ.");
        break;
        case LL_Fea_REQ :
					//att_empPk( dataHdrP );
           s_llFea(l2cap_pdu, dataHdrP);
        break;
				default:
					att_empPk( dataHdrP );
				break;
            
      }
				
    }else  if(llid == 0x02){ //att
      //  if(l2cap_len[0] < 9){
            //printf("**Len Error[%d]**", l2cap_len[0]);
      //  }
				//att_empPk(dataHdrP);
        attOpcode = l2cap_pdu[7]; ///att opcode
        switch(attOpcode){
            case ATT_RD_BY_TYPE_REQ :
										//att_incl.start_hdl = l2cap_pdu[7] + (l2cap_pdu[8] << 8);
                    att_incl.start_hdl = l2cap_pdu[8] + (l2cap_pdu[9] << 8);
                    att_incl.end_hdl = l2cap_pdu[10] + (l2cap_pdu[11] << 8);
                    att_incl.uuid	= l2cap_pdu[12] + (l2cap_pdu[13] << 8);
                    att_server_rdByType(dataHdrP, attOpcode, att_incl.start_hdl, att_incl.end_hdl, att_incl.uuid );
                    break;
            case ATT_RD_BY_GROUP_TYPE_REQ :
                att_incl.start_hdl = l2cap_pdu[8] + (l2cap_pdu[9] << 8);
                att_incl.end_hdl = l2cap_pdu[10] + (l2cap_pdu[11] << 8);
                att_incl.uuid	= l2cap_pdu[12] + (l2cap_pdu[13] << 8);
                att_server_rdByGrType( dataHdrP, attOpcode, att_incl.start_hdl, att_incl.end_hdl, att_incl.uuid );
                break;
            case ATT_EXCHANGE_MTU_REQ :
                att_exMtuReq( dataHdrP, attOpcode, clientMtu);
                break;
            case ATT_FIND_IN_REQ :
                   att_server_findIn( dataHdrP, attOpcode, att_incl.start_hdl, att_incl.end_hdl );
                    break;
            case ATT_RD_REQ :
                    attHandle = l2cap_pdu[8] + (l2cap_pdu[9] << 8);
                    server_rd_rsp( attOpcode, attHandle, dataHdrP);
                    break;
            case ATT_WR_REQ :
                valueLen_w = l2cap_pdu[2] + (l2cap_pdu[3] << 8) - 3;//l2caplen - opcode(1) - attHandle(2)
                attHandle = l2cap_pdu[8] + (l2cap_pdu[9] << 8);
                attValue = &l2cap_pdu[10];
             ////   ser_write_rsp( dataHdrP, attOpcode, attHandle, attValue, valueLen_w);
                break;
						case ATT_WR_CMD :
                valueLen_w = l2cap_pdu[2] + (l2cap_pdu[3] << 8) - 3;//l2caplen - opcode(1) - attHandle(2)
                attHandle = l2cap_pdu[8] + (l2cap_pdu[9] << 8);
                attValue = &l2cap_pdu[10];
                ser_write_no_rsp( dataHdrP, attOpcode, attHandle, attValue, valueLen_w);
				break;		
            default:
              ////      att_empPk(dataHdrP);
                    break;
            
        }
    } else { //0
        ///att_empPk(0x01);
        //printf("*WID[%d,%d]*", l2cap_pdu[0], l2cap_len[0]);
    }
}