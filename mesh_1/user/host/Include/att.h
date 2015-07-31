#ifndef _ATT_H_
#define _ATT_H_
#include "mesh_baseband.h"

/// Invalid attribute handle
#define ATT_ERR_INVALID_HANDLE           0x01 
#define ATT_ERR_ATTR_NOT_FOUND           0x0a

#define L2CAP_MTU_SIZE                   0x0017

/// ll control opcode code
#define LL_CONN_UPDATE_REQ 								 0x00
#define LL_CH_MAP_REQ									 0x01
#define LL_TERMINATE_IND								 0x02
#define LL_Fea_REQ                                       0x08
#define LL_VERSION                                     0x0C

///att opcode code
#define ATT_ERROR_RSP                            0x01
#define ATT_EXCHANGE_MTU_REQ                     0x02
#define ATT_EXCHANGE_MTU_RSP                     0x03 
#define ATT_FIND_IN_REQ								 0x04
#define ATT_FIND_IN_RSP								 0x05
#define ATT_RD_BY_TYPE_REQ	                       0x08
#define ATT_RD_BY_TYPE_RSP 	                       0x09
#define ATT_RD_REQ					              0x0A
#define ATT_RD_RSP                              0x0b
#define ATT_RD_BY_GROUP_TYPE_REQ                       0x10
#define ATT_RD_BY_GROUP_TYPE_RSP                       0x11
///att server opcode code
#define ATT_WR_REQ										 0x12
#define ATT_WR_RSP										 0x13
#define ATT_NOTIFY_HANDLE								 0x1B
#define ATT_WR_CMD									 0x52

/// Characteristic Properties Bit
#define ATT_CHAR_PROP_RD                               0x02
#define ATT_CHAR_PROP_W_NORSP                          0x04
#define ATT_CHAR_PROP_W                                0x08
#define ATT_CHAR_PROP_NTF                  				0x10

#define ATT_CHAR_NUM																	 2


/*
 * DEFINES
 ****************************************************************************************
 */

#define PRIMARY_SERVICE_HD								           			 0x0009
#define FIRST_CHAR_DECL_HD													 			 0x000A
#define FIRST_CHAR_HD																			 0x000B

#define SEC_CHAR_DECL_HD											 	 	 0x000C
#define SEC_CHAR_HD													 		 0x000D

#define END_HD					            							 0x000E

#define ATT_1ST_REQ_START_HDL                          0x0001
#define ATT_1ST_REQ_END_HDL                            0xFFFF

#define PRIMARY_SERVICE_UUID                         0x0001
#define	FIRST_CHAR_UUID								               0x0002
#define SEC_CHAR_UUID								               0x0003

/// Common 16-bit Universal Unique Identifier
enum {
		
    ATT_INVALID_UUID =                                 0,
	
    /*----------------- SERVICES ---------------------*/
    /// Generic Access Profile
   
		
		/*------------------- UNITS ---------------------*/
    /// No defined unit
    ATT_UNIT_UNITLESS                           = 0x2700,
	
		 /*---------------- DECLARATIONS -----------------*/
    /// Primary service Declaration
    GATT_PRIMARY_SERVICE_UUID                       = 0x2800,
    /// Characteristic Declaration
    GATT_CHARACTER_UUID                             = 0x2803,


    /*----------------- DESCRIPTORS -----------------*/
    /// Characteristic extended properties
    GATT_CHAR_EXT_PROPS_UUID                         = 0x2900,
   

    /*--------------- CHARACTERISTICS ---------------*/
		/// Device name
    DEVICE_NAME_UUID                       			= 0x2A00,
    /// Last define
    ATT_LAST
};


/*
 * Type Definition
 ****************************************************************************************
 */
typedef struct att_incl_desc_t
{
    /// start handle value of included service
    uint16_t start_hdl;
    /// end handle value of included service
    uint16_t end_hdl;
    /// attribute value UUID
    uint16_t uuid;
} att_incl_desc;

typedef struct att_char_decl_t
{
    /// characteristic property
    uint8_t proper;
    /// chatacteristic handle
    uint16_t char_hd;
    /// characteristic UUID
    uint16_t char_uuid;
} char_decl_t;

///ll version infromation
typedef struct{
	uint8_t verNr;
	uint16_t CompId;
	uint16_t SubVersNr;
}ll_ver_in_t;


///empty packet
void att_empPk( uint8_t dataHdrP );
///ll version information
void s_llVersion( uint8_t* pdu, uint8_t pdu_type );
/// feature 
void s_llFea(uint8_t* pdu, uint8_t pdu_type);

///exchange MTU
void att_exMtuReq( uint8_t pdu_type, uint8_t attOpcode, uint16_t clientMtu );
/// read by group type request handle
void att_server_rdByGrType( uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd, uint16_t att_type );
/// read by type request handle
void att_server_rdByType(uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd, uint16_t att_type);
/// Find information
void att_server_findIn( uint8_t pdu_type, uint8_t attOpcode, uint16_t st_hd, uint16_t end_hd );
///write without responce
void ser_write_no_rsp(uint8_t pdu_type, uint8_t attOpcode, uint16_t att_hd, uint8_t* attValue, uint8_t valueLen_w);
///read responce
void att_server_rd( uint8_t pdu_type, uint8_t attOpcode, uint16_t att_hd, uint8_t* attValue, uint8_t datalen );
void server_rd_rsp(uint8_t attOpcode, uint16_t attHandle, uint8_t pdu_type);
///notify
void ser_notify_handle( uint8_t pdu_type, uint8_t attOpcode, uint16_t att_hd, uint8_t* attValue, uint8_t datalen );
#endif //_ATT_H_
