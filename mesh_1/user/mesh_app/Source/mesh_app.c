#include "mesh_app.h"
#include "router.h"
#include "string.h"
uint8_t radio_app_rx_data[39] = {3,3};
static uint8_t mac_addr_1[6] = {0x09,0x09,0x09,0x09,0x09,0x09};
uint8_t radio_app_tx_data[20]= {1};

#include "att.h"

static uint8_t i;
uint8_t datalen_n = 20;
uint8_t ser_pdu_n[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};  //notify

///LED response to write value
void led_blink(uint8_t *attValue, uint8_t dataLen)
{
    if(dataLen >= 2){
		switch((*attValue) & 0x07){
				case 1:
          if((*attValue) & 0x10){
							nrf_gpio_pin_set(18);//LED0
          }else{
              nrf_gpio_pin_clear(18);
					}
        break;
				case 2:
					if((*attValue) & 0x10){
              nrf_gpio_pin_set(19);//LED1
          }else{
              nrf_gpio_pin_clear(19);
					}
				break;
        case 3:
					nrf_gpio_cfg_output(23);
					if((*attValue) & 0x10){
              nrf_gpio_pin_clear(23);//RED
          }else{
              nrf_gpio_pin_set(23);
					}
					break;
        case 4:
					nrf_gpio_cfg_output(21);
					if((*attValue) & 0x10){
               nrf_gpio_pin_clear(21);//GREEN
          }else{
              nrf_gpio_pin_set(21);
					}
				case 5:
					nrf_gpio_cfg_output(22);
					if((*attValue) & 0x10){
               nrf_gpio_pin_clear(22);//BLUE
          }else{
              nrf_gpio_pin_set(22);
					}
				break;
			
			}
    }
}

////app send data
void app_tx_data(uint8_t attOpcode, uint16_t attHandle, uint8_t pdu_type)
{
	if(i < 20){
				ser_pdu_n[i] +=1;
				i++;
			}else{
				i = 0;
			}
	att_server_rd( pdu_type, attOpcode, attHandle, ser_pdu_n, datalen_n );
}

void node_on_receive_cb(void)
{
	
	if(radio_app_rx_data[11] == 0x01){
		nrf_gpio_cfg_output(22);
		nrf_gpio_pin_toggle(22);
	}
}

void root_on_receive_cb(void)
{
	if(radio_app_rx_data[11] == 0x01){
		nrf_gpio_cfg_output(21);
		nrf_gpio_pin_toggle(21);
	}
}

void app_test(void)
{
	root_set(0);
	//mac_addr_set(mac_addr_1);
	//node_send_data(app_tx_data, 1);
}

