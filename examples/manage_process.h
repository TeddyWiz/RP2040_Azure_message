
#ifndef _MANAGE_PROCESS_H_
#define _MANAGE_PROCESS_H_

#ifdef __cplusplus
extern "C"
{
#endif
//
#include "port_common.h"

#define FLASH_TARGET_OFFSET 0x180000//256*1024 //network data (MAC) 256
#define FLASH_DEVICEID_OFFSET (FLASH_TARGET_OFFSET + 256)    //device ID 256
#define FLASH_KEY_OFFSET (FLASH_DEVICEID_OFFSET + 256)    //SSL key 4096
#define FLASH_CERT_OFFSET (FLASH_KEY_OFFSET + 4096)    //cert data 4096
#define FLASH_RP_CONFIG_OFFSET (FLASH_CERT_OFFSET + 4096)    //cert data 4096


#define DEBUG_UDP_EN        1
#define DEBUG_PRINT         1
#define AZURE_EN            1



#define PROGVERSiON   "221118_001"

//#define Debug_UDP  1

extern uint8_t Debug_buff[4096];
extern uint16_t Debug_Len;
extern uint8_t NetStatus;

#if 0
extern uint8_t *Test_ApiKey;
extern uint16_t Test_ApiKeyLen;
extern uint8_t *Test_HostAddr;
extern uint16_t Test_HostAddrLen;
extern uint8_t *Test_Token;
extern uint16_t Test_TokenLen;
extern uint8_t *Test_UUID;
extern uint16_t Test_UUIDLen;
#endif
extern uint8_t *Test_DeviceID;
extern uint16_t Test_DeviceIDLen;
extern uint8_t *Test_SSLKey;
extern uint16_t Test_SSLKeyLen;
extern uint8_t *Test_Cert;
extern uint16_t Test_CertLen;
#if 0
extern uint8_t *Test_Config;
extern uint16_t Test_ConfigLen;
#endif




int Load_Flash_Internal_Data(void);
int Load_DeviceID(void);
int Load_Cert(void);
int Load_SSLKey(void);
int FREE_DeviceID(void);
int FREE_Cert(void);
int FREE_SSLKey(void);
int32_t init_Debug_UDP(void);
int32_t DebugSend(uint8_t *buf, uint16_t Len);


void init_GPIO_IRQ(void);
void ctrl_LGP_OUT(uint8_t val);
void ctrl_PW_RPI(uint8_t val);
void ctrl_PICO_LED(uint8_t val);
void init_TIMER_IRQ(void);

void init_ADC(void);
float ADC_DATA_Conv(uint16_t ADC_DATA);
void ADC_Process(void);
float GET_ADC_DATA(uint8_t ADC_CH);
float GET_Volt_DATA(uint8_t ADC_CH);

uint8_t GET_LGP_STATUS(void);
void SET_LGP_STATUS(uint8_t data);
void SET_RPI_STATUS(uint8_t data);
uint8_t GET_RPI_STATUS(void);



void flash_test(void);
void flash_test1(void);
void print_buf_c(const uint8_t *buf, size_t len);

uint8_t GET_EmergenSEQ(void);
void SET_EmergenSEQ(uint8_t SEQ);

uint16_t manage_process(void);
const uint8_t *proto_flash_read(uint32_t f_addr, uint16_t *d_len);
uint16_t proto_flash_write(uint32_t f_addr, uint8_t *Data, uint16_t d_len);
#ifdef __cplusplus
}
#endif

#endif




