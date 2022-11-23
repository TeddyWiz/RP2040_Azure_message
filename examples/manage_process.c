                                                                                                                    #include "manage_process.h"

#include "hardware/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/flash.h"
//#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "pico/critical_section.h"
#include "flashHandler.h"
#include "wizchip_conf.h"
#include "proto_pi_CMD.h"
#include "azure_samples.h"
//#include "hardware/watchdog.h"
//#include "hardware/pwm.h"


#include "socket.h"


#define UDP_DEBUG 1
#define UDP_DEBUG_PORT 50001
#define UDP_DEBUG_DEST_PORT 50002

#define RPI_LGP_IN          2
#define LGP_OUT             3
#define TEST_SWITCH         4
#define RPI_RUN             5
#define PW_RPI              6
#define TEST_PIN            7
#define PICO_LED            25
#define PIN_ADC_1           26
#define PIN_ADC_2           27
#define ADC_conversion_factor       18.85/4096.0;
#define ADC_MAX             200




const uint8_t *flash_Network_contents =  (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
const uint8_t *flash_deviceID_contents = (const uint8_t *) (XIP_BASE + FLASH_DEVICEID_OFFSET);
const uint8_t *flash_key_contents =      (const uint8_t *) (XIP_BASE + FLASH_KEY_OFFSET);
const uint8_t *flash_cert_contents =     (const uint8_t *) (XIP_BASE + FLASH_CERT_OFFSET);

uint8_t NetStatus = 0;

 uint8_t *Test_DeviceID = NULL;
 uint16_t Test_DeviceIDLen = 0;
 char *Test_DeviceID_str = NULL;
 uint8_t *Test_SSLKey = NULL;
 uint16_t Test_SSLKeyLen = 0;
 char *Test_SSLKey_str = NULL;
 uint8_t *Test_Cert = NULL;
 uint16_t Test_CertLen = 0;
 char *Test_Cert_str = NULL;


uint32_t RPI_RunTime;
uint8_t Timer_irq_flag = 0;
uint16_t regular_time = 0;

uint8_t Debug_buff[4096];
uint16_t Debug_Len = 0;

uint8_t EmergencyReportSEQ = 0;
uint16_t EmergencyCnt =0; 
uint32_t RPI_resp_cnt = 0;
uint8_t RPI_Status = 0;

//static critical_section_t g_flash_cri_sec;
int32_t init_Debug_UDP(void)
{
    int32_t ret;
    if((ret = socket(UDP_DEBUG, Sn_MR_UDP, UDP_DEBUG_PORT, 0x00)) != UDP_DEBUG)
        return ret;
}

int32_t DebugSend(uint8_t *buf, uint16_t Len)
{
    int32_t ret;
    uint8_t UDP_BroadIP[4] = {255,255,255,255};
    if(NetStatus > 1)
    {
        ret=sendto(UDP_DEBUG, buf, Len, UDP_BroadIP, UDP_DEBUG_DEST_PORT);
    }
    return ret;
}


int Load_Flash_Internal_Data(void)
{
    Test_DeviceID = (uint8_t *)proto_flash_read(FLASH_DEVICEID_OFFSET, &Test_DeviceIDLen);
    Test_SSLKey = (uint8_t *)proto_flash_read(FLASH_KEY_OFFSET, &Test_SSLKeyLen);
    Test_Cert = (uint8_t *)proto_flash_read(FLASH_CERT_OFFSET, &Test_CertLen);
    #if DEBUG_PRINT
    printf("DeviceIDLen:%d, SSLKeyLen:%d, CertLen:%d \r\n",Test_DeviceIDLen, Test_SSLKeyLen, Test_CertLen);
    printf("Test_DeviceID = %x\r\n", FLASH_DEVICEID_OFFSET);
    printf("Test_SSLKey = %x\r\n", FLASH_KEY_OFFSET);
    printf("Test_Cert = %x\r\n", FLASH_CERT_OFFSET);
    #endif
    
    #if AZURE_EN
    if(Test_DeviceIDLen != 0xFFFF)
    {
        if(Load_DeviceID() < 0)
        {
            printf("Load DeviceID Error\r\n");
            return -2;
        }
    }
    else
    {
        printf("Test_DeviceIDLen[%X] = NONE\r\n", Test_DeviceIDLen);
        return -5;
    }
    if(Test_CertLen != 0xFFFF)
    {
        if(Load_Cert() < 0)
            return -3;
    }
    else
    {
        printf("Test_CertLen[%X] = NONE\r\n", Test_CertLen);
        return -6;
    }
    if(Test_SSLKeyLen != 0xFFFF)
    {
        if(Load_SSLKey() < 0)
            return -4;
    }
    else{
        printf("Test_SSLKeyLen[%X] = NONE\r\n", Test_SSLKeyLen);
        return -7;
    }
    #endif
    return 0;
}
int Load_DeviceID(void)
{
    Test_DeviceID_str = calloc(Test_DeviceIDLen + 1, sizeof(char));
    if(Test_DeviceID_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Test_DeviceID_str not alocation [%ld]\r\n", time_us_32());
        #endif
        return -2;
    }
    memcpy(Test_DeviceID_str, Test_DeviceID, Test_DeviceIDLen);
    #if DEBUG_PRINT
    printf("Test_DeviceID_str[%d]\r\n[%s]\r\n",Test_DeviceIDLen, Test_DeviceID_str);
    #endif
    return 0;
}
int Load_Cert(void)
{
    Test_Cert_str = calloc(Test_CertLen + 1, sizeof(char));
    if(Test_Cert_str == NULL)
    {
    #if DEBUG_PRINT
        printf("Test_Cert_str not alocation [%ld]\r\n", time_us_32());
    #endif
        return -4;
    }
    memcpy(Test_Cert_str, Test_Cert, Test_CertLen);
    #if DEBUG_PRINT
        printf("Test_Cert__str[%d]\r\n[%s]\r\n", Test_CertLen ,Test_Cert_str);
    #endif

    return 0;
}

int Load_SSLKey(void)
{
    Test_SSLKey_str = calloc(Test_SSLKeyLen + 1, sizeof(char));
    if(Test_SSLKey_str == NULL)
    {
        #if DEBUG_PRINT
        printf("Test_SSLKey_str not alocation [%ld]\r\n", time_us_32());
        #endif
        return -3;
    }
    memcpy(Test_SSLKey_str, Test_SSLKey, Test_SSLKeyLen);
    #if DEBUG_PRINT
    printf("Test_SSLKey_str[%d]\r\n[%s]\r\n",Test_SSLKeyLen, Test_SSLKey_str);
    #endif
    return 0;
}
int FREE_DeviceID(void)
{
    free(Test_DeviceID_str);
    return 0;
}

int FREE_Cert(void)
{
    free(Test_Cert_str);
    return 0;
}

int FREE_SSLKey(void)
{
    free(Test_SSLKey_str);
    return 0;
}




void print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15)
            printf("\n");
        else
            printf(" ");
    }
}

void print_buf_c(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%c", buf[i]);
    }
    printf("\n");
}

const uint8_t *proto_flash_read(uint32_t f_addr, uint16_t *d_len)
{
    //uint8_t *temp_data = NULL;
    const uint8_t *flash_data_contents =  (const uint8_t *) (XIP_BASE + f_addr);
    uint16_t temp_len = ((flash_data_contents[0] << 8) & 0xff00)| (flash_data_contents[1] &0x00ff);
    *d_len = (temp_len & 0x00FFFF);
    
    //printf("temp_len = 0x%04x [%d] d_len=0x%04x [%d]\r\n", temp_len, temp_len, *d_len, *d_len);
    //return temp_data;
    return (flash_data_contents + 2);
}
uint16_t proto_flash_write(uint32_t f_addr, uint8_t *Data, uint16_t d_len)
{
    uint8_t *temp_data = NULL;
    uint8_t *temp_p = NULL;
    uint16_t temp_len = d_len +2;
    temp_data = calloc(temp_len + 2 , sizeof(uint8_t));
    if(temp_data == NULL)
        return 0;
    temp_p = temp_data;
    *temp_p++ = (d_len >> 8) & 0x00ff;
    *temp_p++ = d_len & 0x00ff;
    memcpy(temp_p , Data, d_len);
    #if DEBUG_PRINT
    printf("tempdat= %s len(%d) 0x%x%x(%d),",temp_data+2,  d_len, *temp_data, *(temp_data +1), (*temp_data<<8) |(*(temp_data +1)&0x00ff));
    #endif
    print_buf_c(temp_data+2, d_len);
    write_flash(f_addr, temp_data, temp_len + 2);
    return temp_len;
}
#if 0
void flash_test(void)
{
    uint8_t api_test[]="XIx8sEc7voMlwJPw7e1wzipF7yKHCar3fY6Jnezd9T0cie";
    uint16_t api_len = 0, rst = 0;
    uint8_t *api_data = NULL;

    api_len = strlen(api_test);
    flash_critical_section_init();
    printf("load flash data \r\n");
    print_buf_c(flash_api_contents, api_len);
    printf("\r\nsave flash data \r\n");
    //flash_range_program(FLASH_TARGET_OFFSET, api_test, api_len);
    rst = proto_flash_write(FLASH_API_OFFSET, api_test, api_len);
    printf("flash write = %d \r\n ", rst);
    api_data = (uint8_t *)proto_flash_read(FLASH_API_OFFSET, &api_len);
    if(api_data == NULL)
        printf("flash read fail \r\n");
    printf("data Len %d \r\n", api_len);
    print_buf_c(api_data, api_len);
}
#endif

#if 1
int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    static uint8_t pico_LED_data = 0;
    if(pico_LED_data == 1)
    {
        gpio_put(PICO_LED, (bool)pico_LED_data);
        gpio_put(TEST_PIN, 1);
        pico_LED_data = 0;
    }
    else
    {
        gpio_put(PICO_LED, (bool)pico_LED_data);
        gpio_put(TEST_PIN, 0);
        pico_LED_data = 1;
    }
    Timer_irq_flag = 1;
    regular_time++;
    return 500000;
}
#endif
#if 1
void init_TIMER_IRQ(void)
{
    add_alarm_in_ms(500, alarm_callback, NULL, false);
}
#endif


uint16_t manage_process(void)
{
    uint32_t nowTime = time_us_32();
    uint8_t buff_data[512];
    uint16_t buff_len = 0;
    char telemetry_temp_buffer[512];
    
    if(Timer_irq_flag ==1)
    {
        Timer_irq_flag = 0;
        
        //if(regular_time > 7119) //regular report 1hour
        if(regular_time > 120)
        {
            regular_time = 0;
            //regular report 1hour

            sprintf(telemetry_temp_buffer, "{\"DeviceID\": \"%s\",\"TIMES\":%ld,\"STATUS\":\"SUCCESS\"}", Test_DeviceID_str, nowTime);
            #if AZURE_EN
            telemetry_send(telemetry_temp_buffer);
            #endif
            printf("send: %s\r\n", telemetry_temp_buffer);
        }
    }
    return 0;
}

